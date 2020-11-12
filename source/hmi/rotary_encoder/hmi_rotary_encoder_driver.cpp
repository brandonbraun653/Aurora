/********************************************************************************
 *  File Name:
 *    hmi_rotary_encoder_driver.cpp
 *
 *  Description:
 *    Human machine interface driver for a GPIO based rotary encoder
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* Aurora Includes */
#include <Aurora/hmi>

/* Chimera Includes */
#include <Chimera/exti>
#include <Chimera/function>
#include <Chimera/gpio>
#include <Chimera/scheduler>


namespace Aurora::HMI::RotaryEncoder
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
  bool Encoder::initialize( const REConfig &cfg )
  {
    /*-------------------------------------------------
    Input protection
    -------------------------------------------------*/
    /* clang-format off */
    if ( !cfg.pinACfg.validity ||
         !cfg.pinBCfg.validity ||
         !cfg.stableSamples ||
         !cfg.sampleRate ||
         !( cfg.sampleRate < cfg.debounceTime ) )
    { /* clang-format on */
      return false;
    }
    else
    {
      this->lock();
      mConfig = cfg;
    }

    /*-------------------------------------------------
    Reset to a known state
    -------------------------------------------------*/
    this->reset();

    /*-------------------------------------------------
    Initialize the scheduler timer
    -------------------------------------------------*/
    Chimera::Scheduler::LoRes::open();

    /*-------------------------------------------------
    Configure the GPIO for encoder pin A. Only this pin
    needs the EXTI configured. See encoder processing
    to know why.
    -------------------------------------------------*/
    auto result = Chimera::Status::OK;
    auto cb     = Chimera::Function::vGeneric::create<Encoder, &Encoder::processRotateEventCallback>( *this );
    auto driver = Chimera::GPIO::getDriver( mConfig.pinACfg.port, mConfig.pinACfg.pin );


    Chimera::EXTI::EdgeTrigger edgeTrigger;

    switch ( mConfig.pinAActiveEdge )
    {
      case HMI::Button::ActiveEdge::BOTH_EDGES:
        edgeTrigger = Chimera::EXTI::EdgeTrigger::BOTH_EDGE;
        break;

      case HMI::Button::ActiveEdge::RISING_EDGE:
        edgeTrigger = Chimera::EXTI::EdgeTrigger::RISING_EDGE;
        break;

      case HMI::Button::ActiveEdge::FALLING_EDGE:
        edgeTrigger = Chimera::EXTI::EdgeTrigger::FALLING_EDGE;
        break;

      default:
        edgeTrigger = Chimera::EXTI::EdgeTrigger::UNKNOWN;
        break;
    };

    result |= driver->init( mConfig.pinACfg );
    result |= driver->attachInterrupt( cb, edgeTrigger );

    /*-------------------------------------------------
    Configure GPIO for encoder pin B
    -------------------------------------------------*/
    driver = Chimera::GPIO::getDriver( mConfig.pinBCfg.port, mConfig.pinBCfg.pin );
    result |= driver->init( mConfig.pinBCfg );

    /*-------------------------------------------------
    Initialize last known encoder states
    -------------------------------------------------*/
    if ( mConfig.idleHigh )
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
    if ( mConfig.pinPushCfg.validity )
    {
      Aurora::HMI::Button::EdgeConfig bCfg;

      bCfg.activeEdge    = mConfig.pinPushActiveEdge;
      bCfg.debounceTime  = mConfig.debounceTime;
      bCfg.gpioConfig    = mConfig.pinPushCfg;
      bCfg.sampleRate    = mConfig.sampleRate;
      bCfg.stableSamples = mConfig.stableSamples;

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
    mNumRotateEvents = 0;

    /*-------------------------------------------------
    Configure GPIO pin A to have minimal system impact
    -------------------------------------------------*/
    auto driver = Chimera::GPIO::getDriver( mConfig.pinACfg.port, mConfig.pinACfg.pin );
    driver->setMode( Chimera::GPIO::Drive::HIZ, Chimera::GPIO::Pull::NO_PULL );
    driver->detachInterrupt();
    Chimera::EXTI::disable( driver->getInterruptLine() );

    /*-------------------------------------------------
    Configure GPIO pin B to have minimal system impact
    -------------------------------------------------*/
    driver = Chimera::GPIO::getDriver( mConfig.pinBCfg.port, mConfig.pinBCfg.pin );
    driver->setMode( Chimera::GPIO::Drive::HIZ, Chimera::GPIO::Pull::NO_PULL );

    /*-------------------------------------------------
    Configure GPIO center push button for minimal impact.
    -------------------------------------------------*/
    if ( mConfig.pinPushCfg.validity )
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
    auto driver = Chimera::GPIO::getDriver( mConfig.pinACfg.port, mConfig.pinACfg.pin );
    Chimera::EXTI::enable( driver->getInterruptLine() );

    /*-------------------------------------------------
    Enable the center push button
    -------------------------------------------------*/
    if ( mConfig.pinPushCfg.validity )
    {
      mCenterButton.enable();
    }
  }


  void Encoder::disable()
  {
    /*-------------------------------------------------
    Disable the encoder listener
    -------------------------------------------------*/
    auto driver = Chimera::GPIO::getDriver( mConfig.pinACfg.port, mConfig.pinACfg.pin );
    Chimera::EXTI::disable( driver->getInterruptLine() );

    /*-------------------------------------------------
    Disable the center push button
    -------------------------------------------------*/
    if ( mConfig.pinPushCfg.validity )
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
    if ( mConfig.pinPushCfg.validity )
    {
      mCenterButton.onActiveEdge( callback );
    }
  }


  int Encoder::numRotateEvents()
  {
    this->lock();
    int tmp = mNumRotateEvents;
    this->unlock();

    return tmp;
  }


  size_t Encoder::numPushEvents()
  {
    if ( mConfig.pinPushCfg.validity )
    {
      return mCenterButton.numEdgeEvents();
    }
    else
    {
      return 0;
    }
  }


  HMI::Button::ActiveEdge Encoder::getPushActiveEdge()
  {
    if ( mConfig.pinPushCfg.validity )
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
    Chimera::GPIO::Driver_sPtr driver;

    /*-------------------------------------------------
    Read the current state of input A
    -------------------------------------------------*/
    driver = Chimera::GPIO::getDriver( mConfig.pinACfg.port, mConfig.pinACfg.pin );
    driver->getState( pinA );

    /*-------------------------------------------------
    Read the current state of input B
    -------------------------------------------------*/
    driver = Chimera::GPIO::getDriver( mConfig.pinBCfg.port, mConfig.pinBCfg.pin );
    driver->getState( pinB );

    /*-------------------------------------------------
    Convert states into pulses
    -------------------------------------------------*/
    int rotation = 0;

    if ( pinA != a0 )
    {
      a0 = pinA;

      if ( pinB != b0 )
      {
        b0 = pinB;

        if ( pinA == pinB )
        {
          rotation = -1;
        }
        else
        {
          rotation = 1;
        }

        mNumRotateEvents += rotation;

        /*-------------------------------------------------
        Invoke the user callback if it exists
        -------------------------------------------------*/
        if ( mRotateCallback )
        {
          mRotateCallback( rotation );
        }
      }
    }
  }

}  // namespace Aurora::HMI::RotaryEncoder
