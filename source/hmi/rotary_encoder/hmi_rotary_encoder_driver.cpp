/********************************************************************************
 *  File Name:
 *    hmi_rotary_encoder_driver.cpp
 *
 *  Description:
 *    Human machine interface driver for a GPIO based rotary encoder
 *
 *  2020-2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* Aurora Includes */
#include <Aurora/hmi>

/* Chimera Includes */
#include <Chimera/exti>
#include <Chimera/function>
#include <Chimera/gpio>
#include <Chimera/scheduler>


namespace Aurora::HMI::Encoder
{
  /*-------------------------------------------------------------------------------
  Encoder Implementation
  -------------------------------------------------------------------------------*/
  Encoder::Encoder()
  {
  }


  Encoder::~Encoder()
  {
  }

  /*-------------------------------------------------
  Public Methods
  -------------------------------------------------*/
  bool Encoder::initialize( const Config &cfg )
  {
    using namespace Chimera::EXTI;

    /*-------------------------------------------------
    Input protection
    -------------------------------------------------*/
    if ( !cfg.encACfg.validity || !cfg.encBCfg.validity )
    {
      return false;
    }
    else if ( cfg.btnCfg.validity
              && ( !cfg.btnNumSamples || !cfg.btnSampleRate || !( cfg.btnSampleRate < cfg.btnDebounceTime ) ) )
    {
      return false;
    }
    else
    {
      this->lock();
      mConfig = cfg;
    }

    /*-------------------------------------------------
    Initialize the scheduler timer
    -------------------------------------------------*/
    Chimera::Scheduler::LoRes::open();

    /*-------------------------------------------------
    Configure the GPIO for encoder pin A. Only this pin
    needs the EXTI configured due to processing algo.
    -------------------------------------------------*/
    auto result = Chimera::Status::OK;
    auto cb     = Chimera::Function::vGeneric::create<Encoder, &Encoder::processRotateEventCallback>( *this );
    auto driver = Chimera::GPIO::getDriver( mConfig.encACfg.port, mConfig.encACfg.pin );


    EdgeTrigger edgeTrigger;
    switch ( mConfig.encActiveEdge )
    {
      case HMI::Button::ActiveEdge::BOTH_EDGES:
        edgeTrigger = EdgeTrigger::BOTH_EDGE;
        break;

      case HMI::Button::ActiveEdge::RISING_EDGE:
        edgeTrigger = EdgeTrigger::RISING_EDGE;
        break;

      case HMI::Button::ActiveEdge::FALLING_EDGE:
        edgeTrigger = EdgeTrigger::FALLING_EDGE;
        break;

      default:
        edgeTrigger = EdgeTrigger::UNKNOWN;
        break;
    };

    result |= driver->init( mConfig.encACfg );
    result |= driver->attachInterrupt( cb, edgeTrigger );

    /*-------------------------------------------------
    Configure GPIO for encoder pin B
    -------------------------------------------------*/
    driver = Chimera::GPIO::getDriver( mConfig.encBCfg.port, mConfig.encBCfg.pin );
    result |= driver->init( mConfig.encBCfg );

    /*-------------------------------------------------
    Initialize last known encoder states
    -------------------------------------------------*/
    if ( mConfig.encIdleHigh )
    {
      a0 = Chimera::GPIO::State::HIGH;
      b0 = Chimera::GPIO::State::HIGH;
    }
    else
    {
      a0 = Chimera::GPIO::State::LOW;
      b0 = Chimera::GPIO::State::LOW;
    }

    /*-------------------------------------------------
    Configure GPIO for center press button (if exists)
    -------------------------------------------------*/
    if ( mConfig.btnCfg.validity )
    {
      Aurora::HMI::Button::EdgeConfig bCfg;

      bCfg.activeEdge    = mConfig.btnActiveEdge;
      bCfg.debounceTime  = mConfig.btnDebounceTime;
      bCfg.gpioConfig    = mConfig.btnCfg;
      bCfg.sampleRate    = mConfig.btnSampleRate;
      bCfg.stableSamples = mConfig.btnNumSamples;

      mCenterButton.initialize( bCfg );
      mCenterButton.enable();
    }

    this->unlock();
    return ( result == Chimera::Status::OK );
  }


  void Encoder::reset()
  {
    this->lock();

    /*-------------------------------------------------
    Reset the trackers
    -------------------------------------------------*/
    mState.clear();

    /*-------------------------------------------------
    Configure GPIO pin A to have minimal system impact
    -------------------------------------------------*/
    auto driver = Chimera::GPIO::getDriver( mConfig.encACfg.port, mConfig.encACfg.pin );
    driver->setMode( Chimera::GPIO::Drive::HIZ, Chimera::GPIO::Pull::NO_PULL );
    driver->detachInterrupt();
    Chimera::EXTI::disable( driver->getInterruptLine() );

    /*-------------------------------------------------
    Configure GPIO pin B to have minimal system impact
    -------------------------------------------------*/
    driver = Chimera::GPIO::getDriver( mConfig.encBCfg.port, mConfig.encBCfg.pin );
    driver->setMode( Chimera::GPIO::Drive::HIZ, Chimera::GPIO::Pull::NO_PULL );

    /*-------------------------------------------------
    Configure GPIO center push button for minimal impact.
    -------------------------------------------------*/
    if ( mConfig.btnCfg.validity )
    {
      mCenterButton.reset();
    }

    this->unlock();
  }


  void Encoder::enable()
  {
    /*-------------------------------------------------
    Enable the encoder listener
    -------------------------------------------------*/
    auto driver = Chimera::GPIO::getDriver( mConfig.encACfg.port, mConfig.encACfg.pin );
    Chimera::EXTI::enable( driver->getInterruptLine() );

    /*-------------------------------------------------
    Enable the center push button
    -------------------------------------------------*/
    if ( mConfig.btnCfg.validity )
    {
      mCenterButton.enable();
    }
  }


  void Encoder::disable()
  {
    /*-------------------------------------------------
    Disable the encoder listener
    -------------------------------------------------*/
    auto driver = Chimera::GPIO::getDriver( mConfig.encACfg.port, mConfig.encACfg.pin );
    Chimera::EXTI::disable( driver->getInterruptLine() );

    /*-------------------------------------------------
    Disable the center push button
    -------------------------------------------------*/
    if ( mConfig.btnCfg.validity )
    {
      mCenterButton.disable();
    }
  }


  void Encoder::onRotation( RotationCallback callback )
  {
    mRotateCallback = callback;
  }


  void Encoder::onCenterPush( HMI::Button::EdgeCallback callback )
  {
    if ( mConfig.btnCfg.validity )
    {
      mCenterButton.onActiveEdge( callback );
    }
  }


  State Encoder::getState()
  {
    /*-------------------------------------------------
    Copy out the data
    -------------------------------------------------*/
    this->lock();
    State tmp = mState;
    this->unlock();

    /*-------------------------------------------------
    Clear the accumulated info
    -------------------------------------------------*/
    mState.clearAccumulated();

    return tmp;
  }


  HMI::Button::ActiveEdge Encoder::getPushActiveEdge()
  {
    if ( mConfig.btnCfg.validity )
    {
      return mCenterButton.getActiveEdge();
    }
    else
    {
      return HMI::Button::ActiveEdge::UNKNOWN;
    }
  }


  /*-------------------------------------------------
  Private Methods
  -------------------------------------------------*/
  void Encoder::processRotateEventCallback( void *arg )
  {
    /*-------------------------------------------------
    Local Variables
    -------------------------------------------------*/
    Chimera::GPIO::State pinA;
    Chimera::GPIO::State pinB;
    Chimera::GPIO::Driver_rPtr driver;

    /*-------------------------------------------------
    Read the current state of input A
    -------------------------------------------------*/
    driver = Chimera::GPIO::getDriver( mConfig.encACfg.port, mConfig.encACfg.pin );
    driver->getState( pinA );

    /*-------------------------------------------------
    Read the current state of input B
    -------------------------------------------------*/
    driver = Chimera::GPIO::getDriver( mConfig.encBCfg.port, mConfig.encBCfg.pin );
    driver->getState( pinB );

    /*-------------------------------------------------
    Convert states into pulses. Uses the knowledge that
    pinB is always stable while pinA is edge triggering
    to sample the system and provide a cleaned up
    version of the encoder signal.
    -------------------------------------------------*/
    if ( pinA != a0 )
    {
      /*-------------------------------------------------
      pinA has edge triggered, effectively sampling pinB
      -------------------------------------------------*/
      a0 = pinA;

      if ( pinB != b0 )
      {
        /*-------------------------------------------------
        pinB has changed from the last time, this means a
        rotation of some kind has occurred.
        -------------------------------------------------*/
        b0 = pinB;

        /*-------------------------------------------------
        The rotation direction is defined by the relative
        states of pinA and pinB.
        -------------------------------------------------*/
        int rotation = ( pinA == pinB ) ? -1 : 1;

        mState.absolutePosition += rotation;
        mState.diffPosition += rotation;

        /*-------------------------------------------------
        Invoke the user callback if it exists
        -------------------------------------------------*/
        if ( mRotateCallback )
        {
          mRotateCallback( mState );
        }
      }
    }
  }

}  // namespace Aurora::HMI::Encoder
