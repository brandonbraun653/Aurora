/********************************************************************************
 *  File Name:
 *    serial_sink->cpp
 *
 *  Description:
 *    Implements a serial based sink for the uLogger interface.
 *
 *  2019-2021 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

/* Serial Sink Includes */
#include <Aurora/logging>

/* Chimera Includes */
#include <Chimera/event>
#include <Chimera/serial>
#include <Chimera/thread>


namespace Aurora::Logging
{
  /*-------------------------------------------------------------------------------
  Static Data
  -------------------------------------------------------------------------------*/
  static Chimera::Serial::Driver_sPtr sink;

  /*-------------------------------------------------------------------------------
  Driver Implementation
  -------------------------------------------------------------------------------*/
  SerialSink::SerialSink( Chimera::Serial::Channel channel ) : mSerialChannel( channel ), SinkInterface()
  {
  }


  SerialSink::SerialSink() : mSerialChannel( Chimera::Serial::Channel::NOT_SUPPORTED )
  {
  }


  SerialSink::~SerialSink()
  {
    sink.reset();
  }


  void SerialSink::assignChannel( Chimera::Serial::Channel channel )
  {
    mSerialChannel = channel;
  }


  ::uLog::Result SerialSink::open()
  {
    /*-------------------------------------------------
    Grab a reference to the underlying serial driver
    -------------------------------------------------*/
    if ( !sink )
    {
      sink = Chimera::Serial::getDriver( mSerialChannel );
    }

    /*------------------------------------------------
    Mask the error code into a simple pass/fail. I don't think the sinks
    in general should support complicated return codes.
    ------------------------------------------------*/
    if ( !sink )
    {
      return ::uLog::Result::RESULT_FAIL;
    }

    return ::uLog::Result::RESULT_SUCCESS;
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
    hwResult |= sink->write( message, length );
    hwResult |= sink->await( Chimera::Event::Trigger::TRIGGER_WRITE_COMPLETE, TIMEOUT_BLOCK );
    sink->unlock();

    if ( hwResult != Chimera::Status::OK )
    {
      ulResult = ::uLog::Result::RESULT_FAIL;
    }

    return ulResult;
  }

}  // namespace Aurora::Logging
