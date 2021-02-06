/********************************************************************************
 *  File Name:
 *    hmi_button_driver.cpp
 *
 *  Description:
 *    Human machine interface driver for a GPIO based button
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* Aurora Includes */
#include <Aurora/hmi>

/* Chimera Includes */
#include <Chimera/assert>
#include <Chimera/exti>
#include <Chimera/function>
#include <Chimera/gpio>
#include <Chimera/scheduler>

namespace Aurora::HMI::Button
{
  /*-------------------------------------------------------------------------------
  EdgeTrigger Implementation
  -------------------------------------------------------------------------------*/
  EdgeTrigger::EdgeTrigger() :
      mCallback( EdgeCallback() ), mConfig( {} ), mNumEvents( 0 ), mMaxNumSamples( 0 ), mEnabled( false )
  {
  }


  EdgeTrigger::~EdgeTrigger()
  {
  }


  /*-------------------------------------------------
  Public Methods
  -------------------------------------------------*/
  bool EdgeTrigger::initialize( const EdgeConfig &cfg )
  {
    /*-------------------------------------------------
    Input protection
    -------------------------------------------------*/
    /* clang-format off */
    if ( !cfg.gpioConfig.validity ||
         !cfg.stableSamples ||
         !cfg.sampleRate ||
         !( cfg.sampleRate < cfg.debounceTime ) ||
         !( cfg.activeEdge < ActiveEdge::NUM_OPTIONS ) )
    { /* clang-format on */
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
    Convert the HMI edge type to the EXTI type
    -------------------------------------------------*/
    Chimera::EXTI::EdgeTrigger edgeTrigger;

    switch ( mConfig.activeEdge )
    {
      case ActiveEdge::BOTH_EDGES:
        edgeTrigger = Chimera::EXTI::EdgeTrigger::BOTH_EDGE;
        break;

      case ActiveEdge::RISING_EDGE:
        edgeTrigger = Chimera::EXTI::EdgeTrigger::RISING_EDGE;
        break;

      case ActiveEdge::FALLING_EDGE:
        edgeTrigger = Chimera::EXTI::EdgeTrigger::FALLING_EDGE;
        break;

      default:
        edgeTrigger = Chimera::EXTI::EdgeTrigger::UNKNOWN;
        break;
    };

    /*-------------------------------------------------
    Configure the GPIO
    -------------------------------------------------*/
    auto result = Chimera::Status::OK;
    auto cb     = Chimera::Function::vGeneric::create<EdgeTrigger, &EdgeTrigger::gpioEdgeTriggerCallback>( *this );
    auto driver = Chimera::GPIO::getDriver( mConfig.gpioConfig.port, mConfig.gpioConfig.pin );

    result |= driver->init( mConfig.gpioConfig );
    result |= driver->attachInterrupt( cb, edgeTrigger );
    result |= driver->getState( mLastStableState );

    this->unlock();
    return ( result == Chimera::Status::OK );
  }


  void EdgeTrigger::reset()
  {
    this->lock();

    /*-------------------------------------------------
    Reset the trackers
    -------------------------------------------------*/
    mNumEvents = 0;
    mDebounced = 0;

    /*-------------------------------------------------
    Configure GPIO to have minimal system impact
    -------------------------------------------------*/
    auto driver = Chimera::GPIO::getDriver( mConfig.gpioConfig.port, mConfig.gpioConfig.pin );
    driver->setMode( Chimera::GPIO::Drive::HIZ, Chimera::GPIO::Pull::NO_PULL );
    driver->detachInterrupt();

    /*-------------------------------------------------
    Disable the GPIO's EXTI hooks
    -------------------------------------------------*/
    Chimera::EXTI::disable( driver->getInterruptLine() );
    mEnabled = false;

    this->unlock();
  }


  void EdgeTrigger::enable()
  {
    mEnabled = true;
    enableISR();
  }


  void EdgeTrigger::disable()
  {
    disableISR();
    mEnabled = false;
  }


  void EdgeTrigger::onActiveEdge( EdgeCallback callback )
  {
    mCallback = callback;
  }


  size_t EdgeTrigger::numEdgeEvents()
  {
    this->lock();
    auto tmp = mNumEvents;
    this->unlock();

    return tmp;
  }


  ActiveEdge EdgeTrigger::getActiveEdge()
  {
    this->lock();
    auto tmp = mConfig.activeEdge;
    this->unlock();

    return tmp;
  }


  /*-------------------------------------------------
  Private Methods
  -------------------------------------------------*/
  void EdgeTrigger::enableISR()
  {
    auto driver = Chimera::GPIO::getDriver( mConfig.gpioConfig.port, mConfig.gpioConfig.pin );
    Chimera::EXTI::enable( driver->getInterruptLine() );
  }


  void EdgeTrigger::disableISR()
  {
    auto driver = Chimera::GPIO::getDriver( mConfig.gpioConfig.port, mConfig.gpioConfig.pin );
    Chimera::EXTI::disable( driver->getInterruptLine() );
  }


  void EdgeTrigger::gpioEdgeTriggerCallback( void *arg )
  {
    /*-------------------------------------------------
    Don't do anything if not explicitly enabled
    -------------------------------------------------*/
    if ( !mEnabled )
    {
      return;
    }

    /*-------------------------------------------------
    Disable the interrupt as quickly as possible
    -------------------------------------------------*/
    this->disableISR();

    /*-------------------------------------------------
    Figure out the number of samples the polling
    function should be active for. Div zero protected
    by the initialization sequence.
    -------------------------------------------------*/
    mCurrentNumSamples = 0;
    mMaxNumSamples     = mConfig.debounceTime / mConfig.sampleRate;

    /*-------------------------------------------------
    Build the debounced mask
    -------------------------------------------------*/
    mDebounceMsk = 0;
    for ( size_t x = 0; x < mConfig.stableSamples; x++ )
    {
      mDebounceMsk |= ( 1u << x );
    }

    /*-------------------------------------------------
    Register a periodic function with the scheduler to
    poll the GPIO state for a bit.
    -------------------------------------------------*/
    auto cb = Chimera::Function::Opaque::create<EdgeTrigger, &EdgeTrigger::gpioEdgeSamplerCallback>( *this );
    Chimera::Scheduler::LoRes::periodic( cb, mConfig.sampleRate, mMaxNumSamples );
  }


  void EdgeTrigger::gpioEdgeSamplerCallback()
  {
    using namespace Chimera::GPIO;

    /*-------------------------------------------------
    Don't do anything if not explicitly enabled
    -------------------------------------------------*/
    if ( !mEnabled )
    {
      return;
    }

    /*-------------------------------------------------
    Read the current state of the GPIO pin
    -------------------------------------------------*/
    State currentState;
    auto driver = Chimera::GPIO::getDriver( mConfig.gpioConfig.port, mConfig.gpioConfig.pin );
    driver->getState( currentState );

    /*-------------------------------------------------
    Handle too many failed samples. Force another ISR
    event to re-register this sampler function.
    -------------------------------------------------*/
    if ( mCurrentNumSamples >= mMaxNumSamples )
    {
      Chimera::Scheduler::LoRes::cancel_this();
      this->enableISR();
      return;
    }
    else
    {
      mCurrentNumSamples++;
    }

    /*-------------------------------------------------
    Update the filter to reflect how many samples the
    GPIO has held a state that differs from the last
    known stable logic level.
    -------------------------------------------------*/
    if ( currentState == mLastStableState )
    {
      mDebounced = 0;
      return;
    }
    else
    {
      /*-------------------------------------------------
      Insert a single stable sample
      -------------------------------------------------*/
      mDebounced = ( mDebounced << 1u ) | 1u;
      mCurrentNumSamples++;

      /*-------------------------------------------------
      Require sequential samples to be the same before
      continuing on to accept a new state change.
      -------------------------------------------------*/
      if ( ( mDebounced & mDebounceMsk ) != mDebounceMsk )
      {
        return;
      }
    }

    /*-------------------------------------------------
    A stable edge transition has been observed
    -------------------------------------------------*/
    ActiveEdge edgeType = ActiveEdge::UNKNOWN;
    if( ( currentState == State::HIGH ) && ( mLastStableState == State::LOW ) )
    {
      edgeType         = ActiveEdge::RISING_EDGE;
      mLastStableState = currentState;
    }
    else if ( ( currentState == State::LOW ) && ( mLastStableState == State::HIGH ) )
    {
      edgeType         = ActiveEdge::FALLING_EDGE;
      mLastStableState = currentState;
    }
    else  // Should never hit this
    {
      RT_HARD_ASSERT( false );
    }

    /*-------------------------------------------------
    Invoke the callback if registered
    -------------------------------------------------*/
    if( mCallback )
    {
      mCallback( edgeType );
    }

    /*-------------------------------------------------
    Reset the sampler such that another ISR event has
    to re-trigger things.
    -------------------------------------------------*/
    Chimera::Scheduler::LoRes::cancel_this();
    this->disableISR();
    this->enableISR();
  }

}  // namespace Aurora::HMI::Button
