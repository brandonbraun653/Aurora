/********************************************************************************
 *  File Name:
 *    serial_sink->cpp
 *
 *  Description:
 *    Implements a serial based sink for the uLogger interface.
 *
 *  2019-2020 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

/* C++ Includes */
#include <array>

/* Serial Sink Includes */
#include <Aurora/logging/serial_sink.hpp>
#include <Aurora/logging/serial_sink_config.hpp>

/* Chimera Includes */
#include <Chimera/serial>

/* Boost Includes */
#include <boost/circular_buffer.hpp>

#if defined( CHIMERA_MODULES_ULOG_SUPPORT ) && ( CHIMERA_MODULES_ULOG_SUPPORT == 1 )

static Chimera::Serial::Serial_sPtr sink;
static boost::circular_buffer<uint8_t> buffer( 256 );
static std::array<uint8_t, 256> hwBuffer;

namespace Chimera::Modules::uLog
{
  SerialSink::SerialSink( Chimera::Serial::Channel channel, bool init ) : mInitHW( init ), mSerialChannel( channel )
  {
  }

  SerialSink::SerialSink() : mInitHW( true ), mSerialChannel( SerialChannel )
  {
  }

  SerialSink::~SerialSink()
  {
    sink = nullptr;
    hwBuffer.fill( 0 );
    buffer.clear();
  }

  ::uLog::Result SerialSink::open()
  {
    Chimera::Status_t hwResult = Chimera::Status::OK;
    ::uLog::Result sinkResult  = ::uLog::Result::RESULT_SUCCESS;

    /*-------------------------------------------------
    Initialize the buffer memory
    -------------------------------------------------*/
    buffer.clear();
    buffer.linearize();
    hwBuffer.fill( 0 );

    /*-------------------------------------------------
    Grab a reference to the underlying serial driver
    -------------------------------------------------*/
    if ( !sink )
    {
      sink = Chimera::Serial::create_shared_ptr( mSerialChannel );
    }

    /*------------------------------------------------
    Initialize the hardware
    ------------------------------------------------*/
    if( mInitHW )
    {
      Chimera::Serial::Config cfg;
      cfg.baud     = static_cast<size_t>( SerialBaud );
      cfg.flow     = SerialFlowCtrl;
      cfg.parity   = SerialParity;
      cfg.stopBits = SerialStopBits;
      cfg.width    = SerialCharWid;

      auto hwMode = Chimera::Hardware::PeripheralMode::INTERRUPT;

      hwResult |= sink->assignHW( mSerialChannel, SerialPins );
      hwResult |= sink->configure( cfg );
      hwResult |= sink->begin( hwMode, hwMode );
      hwResult |= sink->enableBuffering( Chimera::Hardware::SubPeripheral::TX, &buffer, hwBuffer.data(), hwBuffer.size() );
    }

    /*------------------------------------------------
    Mask the error code into a simple pass/fail. I don't think the sinks
    in general should support complicated return codes.
    ------------------------------------------------*/
    if ( hwResult != Chimera::Status::OK )
    {
      sinkResult = ::uLog::Result::RESULT_FAIL;
    }

    return sinkResult;
  }

  ::uLog::Result SerialSink::close()
  {
    ::uLog::Result sinkResult = ::uLog::Result::RESULT_SUCCESS;

    if ( sink->end() != Chimera::Status::OK )
    {
      sinkResult = ::uLog::Result::RESULT_FAIL;
    }

    return sinkResult;
  }

  ::uLog::Result SerialSink::flush()
  {
    ::uLog::Result sinkResult = ::uLog::Result::RESULT_SUCCESS;

    if ( sink->flush( Chimera::Hardware::SubPeripheral::TXRX ) != Chimera::Status::OK )
    {
      sinkResult = ::uLog::Result::RESULT_FAIL;
    }

    return sinkResult;
  }

  ::uLog::IOType SerialSink::getIOType()
  {
    return ::uLog::IOType::SERIAL_SINK;
  }

  ::uLog::Result SerialSink::log( const ::uLog::Level level, const void *const message, const size_t length )
  {
    using namespace Chimera::Threading;

    /*------------------------------------------------
    Make sure we can actually log the data
    ------------------------------------------------*/
    if ( level < getLogLevel() )
    {
      return ::uLog::Result::RESULT_INVALID_LEVEL;
    }

    /*------------------------------------------------
    Write the data and block the current thread execution
    until the transfer is complete.
    ------------------------------------------------*/
    auto hwResult = Chimera::Status::OK;
    auto ulResult = ::uLog::Result::RESULT_SUCCESS;

    sink->lock();

    hwResult |= sink->write( reinterpret_cast<const uint8_t *const>( message ), length, TIMEOUT_DONT_WAIT );
    hwResult |= sink->await( Chimera::Event::TRIGGER_WRITE_COMPLETE, TIMEOUT_BLOCK );

    if ( hwResult != Chimera::Status::OK )
    {
      ulResult = ::uLog::Result::RESULT_FAIL;
    }

    sink->unlock();

    return ulResult;
  }
}  // namespace Chimera::Modules::uLog

#endif /* CHIMERA_MODULES_ULOG_SUPPORT */