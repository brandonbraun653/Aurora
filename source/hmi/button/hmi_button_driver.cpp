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
#include <Chimera/exti>
#include <Chimera/function>
#include <Chimera/gpio>
#include <Chimera/scheduler>

namespace Aurora::HMI::Button
{
  /*-------------------------------------------------------------------------------
  EdgeTrigger Implementation
  -------------------------------------------------------------------------------*/
  EdgeTrigger::EdgeTrigger() : mCallback( EdgeCallback() ), mConfig( {} ), mNumEvents( 0 )
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
      mLock.lock();
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

    mLock.unlock();
    return ( result == Chimera::Status::OK );
  }


  void EdgeTrigger::reset()
  {
    mLock.lock();

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

    mLock.unlock();
  }


  void EdgeTrigger::enable()
  {
    auto driver = Chimera::GPIO::getDriver( mConfig.gpioConfig.port, mConfig.gpioConfig.pin );
    Chimera::EXTI::enable( driver->getInterruptLine() );
  }


  void EdgeTrigger::disable()
  {
    auto driver = Chimera::GPIO::getDriver( mConfig.gpioConfig.port, mConfig.gpioConfig.pin );
    Chimera::EXTI::disable( driver->getInterruptLine() );
  }


  void EdgeTrigger::onActiveEdge( EdgeCallback callback )
  {
    mCallback = callback;
  }


  size_t EdgeTrigger::numEdgeEvents()
  {
    mLock.lock();
    auto tmp = mNumEvents;
    mLock.unlock();

    return tmp;
  }


  ActiveEdge EdgeTrigger::getActiveEdge()
  {
    mLock.lock();
    auto tmp = mConfig.activeEdge;
    mLock.unlock();

    return tmp;
  }


  /*-------------------------------------------------
  Private Methods
  -------------------------------------------------*/
  void EdgeTrigger::gpioEdgeTriggerCallback( void *arg )
  {
    /*-------------------------------------------------
    Disable the interrupt as quickly as possible
    -------------------------------------------------*/
    this->disable();

    /*-------------------------------------------------
    Figure out the number of samples the polling
    function should be active for. Div zero protected
    by the initialization sequence.
    -------------------------------------------------*/
    size_t numSamples = mConfig.debounceTime / mConfig.sampleRate;

    /*-------------------------------------------------
    Register a periodic function with the scheduler to
    poll the GPIO state for a bit.
    -------------------------------------------------*/
    auto cb = Chimera::Function::Opaque::create<EdgeTrigger, &EdgeTrigger::gpioEdgeSamplerCallback>( *this );

    Chimera::Scheduler::LoRes::periodic( cb, mConfig.sampleRate, numSamples );
  }


  void EdgeTrigger::gpioEdgeSamplerCallback()
  {
    /*-------------------------------------------------
    Read the current state of the GPIO pin
    -------------------------------------------------*/
    Chimera::GPIO::State currentState;
    auto driver = Chimera::GPIO::getDriver( mConfig.gpioConfig.port, mConfig.gpioConfig.pin );
    driver->getState( currentState );

    /*-------------------------------------------------
    Shift the filter because a single sample has passed
    -------------------------------------------------*/
    mDebounced = mDebounced << 1u;

    /*-------------------------------------------------
    Assuming the state is valid, fill in the spot just
    created by the shift. Forcefully set the zero as
    shift hardware technically could be circular and we
    might run out of bits.
    -------------------------------------------------*/
    if ( mConfig.activeEdge == ActiveEdge::BOTH_EDGES )
    {
      mDebounced |= 1u;
    }
    else
    {
      switch ( mConfig.activeEdge )
      {
        case ActiveEdge::RISING_EDGE:
          if ( currentState == Chimera::GPIO::State::HIGH )
          {
            mDebounced |= 1u;
          }
          else
          {
            mDebounced &= ~1u;
          }
          break;

        case ActiveEdge::FALLING_EDGE:
          if ( currentState == Chimera::GPIO::State::LOW )
          {
            mDebounced |= 1u;
          }
          else
          {
            mDebounced &= ~1u;
          }
          break;

        default:
          mDebounced &= ~1u;
          break;
      };
    }

    /*-------------------------------------------------
    Build the debounced mask
    -------------------------------------------------*/
    size_t debounceMask = 0;
    for ( size_t x = 0; x < mConfig.stableSamples; x++ )
    {
      debounceMask |= ( 1u << x );
    }

    /*-------------------------------------------------
    Have enough samples been stable to consider this a
    sufficiently debounced button?
    -------------------------------------------------*/
    if ( ( mDebounced & debounceMask ) == debounceMask )
    {
      /*-------------------------------------------------
      Update trackers
      -------------------------------------------------*/
      mLock.lock();
      mNumEvents++;
      mLock.unlock();

      mDebounced = 0;
      Chimera::Scheduler::LoRes::cancel_this();

      /*-------------------------------------------------
      Re-enable the trigger to listen for more presses
      -------------------------------------------------*/
      this->enable();

      /*-------------------------------------------------
      Invoke user callback if exists
      -------------------------------------------------*/
      if ( mCallback )
      {
        mCallback( mConfig.activeEdge );
      }
    }
    // else button not sufficiently debounced yet
  }

}  // namespace Aurora::HMI::Button
