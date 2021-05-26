/********************************************************************************
 *  File Name:
 *    hmi_sr_input.cpp
 *
 *  Description:
 *    Shift register input driver
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* Aurora Includes */
#include <Aurora/hmi>
#include <Aurora/logging>
#include <Aurora/math>

/* Chimera Includes */
#include <Chimera/thread>

/*-------------------------------------------------------------------------------
Macros
-------------------------------------------------------------------------------*/
#define BIT_TO_IO_STATE( bit, data ) ( ( ( 1u << bit ) & data ) ? Chimera::GPIO::State::HIGH : Chimera::GPIO::State::LOW )

namespace Aurora::HMI::SR
{
  /*-------------------------------------------------------------------------------
  ShiftInput Implementation
  -------------------------------------------------------------------------------*/
  ShiftInput::ShiftInput()
  {
  }


  ShiftInput::~ShiftInput()
  {
  }


  /**
   * @brief Initializes the ShiftInput driver
   *
   * @param cfg     Driver configuration structure
   * @return Chimera::Status_t
   */
  Chimera::Status_t ShiftInput::init( const ShifterConfig &cfg )
  {
    /*-------------------------------------------------
    Input Protection
    -------------------------------------------------*/
    if ( ( cfg.byteWidth > sizeof( uint32_t ) ) || ( cfg.inputMask == 0 )
         || ( nullptr == Chimera::SPI::getDriver( cfg.spiChannel ) ) )
    {
      return Chimera::Status::INVAL_FUNC_PARAM;
    }

    /*-------------------------------------------------
    Initialize the driver
    -------------------------------------------------*/
    Chimera::Thread::LockGuard lck( *this );

    mDriverCfg = cfg;
    mEventQueue.clear();

    /*-------------------------------------------------
    Read the current state of the SR
    -------------------------------------------------*/
    uint32_t sr_data = 0;
    if ( !readSR( &sr_data, mDriverCfg.byteWidth ) )
    {
      return Chimera::Status::FAIL;
    }

    /*-------------------------------------------------
    Update each bit to their default values
    -------------------------------------------------*/
    for ( size_t x = 0; x < ARRAY_COUNT( mBitConfig ); x++ )
    {
      mBitConfig[ x ].bit          = 0;
      mBitConfig[ x ].debounceTime = Chimera::Thread::TIMEOUT_BLOCK;
      mBitConfig[ x ].polarity     = Polarity::INVALID;
      mBitConfig[ x ].configured   = false;

      mBitState[ x ].debounceStart = 0;
      mBitState[ x ].numSamples    = 0;
      mBitState[ x ].active        = false;
      mBitState[ x ].lastState     = BIT_TO_IO_STATE( x, sr_data );
    }

    return Chimera::Status::OK;
  }


  /**
   * @brief Configures a single bit
   *
   * Performs checks to ensure that only a single bit is being monitored and that
   * it complies with expected parameters.
   *
   * @param cfg     Bit configuration structure
   * @return Chimera::Status_t
   */
  Chimera::Status_t ShiftInput::configureBit( const InputConfig &cfg )
  {
    /*-------------------------------------------------
    Is the bit supported in the global input mask?
    Is a single bit set (aka power of 2)?
    -------------------------------------------------*/
    if ( !( cfg.bit & mDriverCfg.inputMask ) || !Aurora::Math::isPower2( cfg.bit ) )
    {
      return Chimera::Status::INVAL_FUNC_PARAM;
    }

    /*-------------------------------------------------
    Figure out the position of the set bit
    -------------------------------------------------*/
    size_t idx = Aurora::Math::maxBitSetPow2( cfg.bit );
    if ( idx >= 32 )
    {
      return Chimera::Status::INVAL_FUNC_PARAM;
    }

    /*-------------------------------------------------
    Read the current state of the SR
    -------------------------------------------------*/
    uint32_t sr_data = 0;
    if ( !readSR( &sr_data, mDriverCfg.byteWidth ) )
    {
      return Chimera::Status::FAIL;
    }

    /*-------------------------------------------------
    Update the config and current state
    -------------------------------------------------*/
    mBitConfig[ idx ]            = cfg;
    mBitConfig[ idx ].configured = true;
    mBitState[ idx ].active      = false;
    mBitState[ idx ].lastState   = BIT_TO_IO_STATE( idx, sr_data );

    return Chimera::Status::OK;
  }


  /**
   * @brief Pulls the next edge event off the queue
   *
   * @param event     The event structure to fill out
   * @return true     An event was retrieved
   * @return false    No pending events
   */
  bool ShiftInput::nextEvent( InputEvent &event )
  {
    Chimera::Thread::LockGuard lck( *this );

    if ( !mEventQueue.empty() )
    {
      mEventQueue.pop_into( event );
      return true;
    }
    else
    {
      return false;
    }
  }


  /**
   * @brief Periodic processing to detect edge events
   *
   * Must be called fairly quickly to not miss any transitions. This
   * is effectively a polling driver.
   */
  void ShiftInput::processHardware()
  {
    /*-------------------------------------------------
    Read the current state of the shift register
    -------------------------------------------------*/
    uint32_t sr_data = 0;
    if ( !readSR( &sr_data, mDriverCfg.byteWidth ) )
    {
      LOG_ERROR( "Failed SR read on SPI channel %d\r\n", mDriverCfg.spiChannel );
      return;
    }

    /*-------------------------------------------------
    Make sure we have the queue
    -------------------------------------------------*/
    Chimera::Thread::LockGuard lck( *this );

    /*-------------------------------------------------
    Iterate over each bit and check for new events
    -------------------------------------------------*/
    for ( size_t bit = 0; bit < 32; bit++ )
    {
      /*-------------------------------------------------
      Is this bit supported in the global mask and has it
      been configured?
      -------------------------------------------------*/
      if ( !( ( 1u << bit ) & mDriverCfg.inputMask ) || !mBitConfig[ bit ].configured )
      {
        continue;
      }

      /*-------------------------------------------------
      Detect start of new debounce
      -------------------------------------------------*/
      auto currState = BIT_TO_IO_STATE( bit, sr_data );

      if ( !mBitState[ bit ].active )
      {
        if( currState != mBitState[ bit ].lastState )
        {
          mBitState[ bit ].debounceStart = Chimera::millis();
          mBitState[ bit ].numSamples    = 0;
          mBitState[ bit ].active        = true;
        }
        else
        {
          continue;
        }
      }

      /*-------------------------------------------------
      Has enough time elapse for a debounce?
      -------------------------------------------------*/
      if ( ( Chimera::millis() - mBitState[ bit ].debounceStart ) >= mBitConfig[ bit ].debounceTime )
      {
        /*-------------------------------------------------
        Prepare new event to post to queue
        -------------------------------------------------*/
        InputEvent newEvent;
        newEvent.timestamp = Chimera::millis();

        /*-------------------------------------------------
        Which bit is set?
        -------------------------------------------------*/
        newEvent.bit = bit;

        /*-------------------------------------------------
        Determine the edge transition
        -------------------------------------------------*/
        if ( ( currState == Chimera::GPIO::State::HIGH ) && ( mBitState[ bit ].lastState == Chimera::GPIO::State::LOW ) )
        {
          newEvent.edge = Edge::RISING;
        }
        else if( ( currState == Chimera::GPIO::State::LOW ) && ( mBitState[ bit ].lastState == Chimera::GPIO::State::HIGH ) )
        {
          newEvent.edge = Edge::FALLING;
        }
        else
        {
          RT_HARD_ASSERT( false );
        }

        /*-------------------------------------------------
        Determine if the input is active
        -------------------------------------------------*/
        newEvent.state = State::INACTIVE;
        if ( ( newEvent.edge == Edge::FALLING ) && ( mBitConfig[ bit ].polarity == Polarity::ACTIVE_LOW ) )
        {
          newEvent.state = State::ACTIVE;
        }

        /*-------------------------------------------------
        Disable debouncing on this bit
        -------------------------------------------------*/
        mBitState[ bit ].active    = false;
        mBitState[ bit ].lastState = currState;

        /*-------------------------------------------------
        Post the event
        -------------------------------------------------*/
        if ( mEventQueue.full() )
        {
          LOG_WARN( "Missed SR event on bit %d due to queue full\r\n", bit );
        }

        mEventQueue.push( newEvent );
      }
    }
  }


  bool ShiftInput::readSR( uint32_t *const data, const size_t bytes )
  {
    using namespace Chimera::Thread;

    /*-------------------------------------------------
    Guarantee access to the device
    -------------------------------------------------*/
    auto spi = Chimera::SPI::getDriver( mDriverCfg.spiChannel );
    auto smp = Chimera::GPIO::getDriver( mDriverCfg.sampleKeyPort, mDriverCfg.sampleKeyPin );
    auto cs  = Chimera::GPIO::getDriver( mDriverCfg.chipSelectPort, mDriverCfg.chipSelectPin );

    if ( !spi || !smp || !cs )
    {
      return false;
    }

    /*-------------------------------------------------
    Strobe the sample pin to lock in a new measurement
    -------------------------------------------------*/
    smp->setState( Chimera::GPIO::State::LOW );
    Chimera::blockDelayMicroseconds( 1 );
    smp->setState( Chimera::GPIO::State::HIGH );
    Chimera::blockDelayMicroseconds( 1 );

    /*-------------------------------------------------
    Read out the requested number of bytes
    -------------------------------------------------*/
    cs->setState( Chimera::GPIO::State::LOW );
    spi->readBytes( data, bytes );
    spi->await( Chimera::Event::Trigger::TRIGGER_TRANSFER_COMPLETE, TIMEOUT_BLOCK );
    cs->setState( Chimera::GPIO::State::HIGH );

    return true;
  }

}  // namespace Aurora::HMI::SR
