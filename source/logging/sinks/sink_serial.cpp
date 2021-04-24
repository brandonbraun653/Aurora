/********************************************************************************
 *  File Name:
 *    serial_sink->cpp
 *
 *  Description:
 *    Implements a serial based sink for the uLogger interface.
 *
 *  2019-2021 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

/* Aurora Includes */
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
  static Chimera::Serial::Driver_rPtr sink;


  /*-------------------------------------------------------------------------------
  Driver Implementation
  -------------------------------------------------------------------------------*/
  SerialSink::SerialSink( Chimera::Serial::Channel channel ) : SinkInterface(), mSerialChannel( channel )
  {
  }


  SerialSink::SerialSink() : mSerialChannel( Chimera::Serial::Channel::NOT_SUPPORTED )
  {
  }


  SerialSink::~SerialSink()
  {
  }


  void SerialSink::assignChannel( Chimera::Serial::Channel channel )
  {
    mSerialChannel = channel;
  }


  Result SerialSink::open()
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
      return Result::RESULT_FAIL;
    }

    return Result::RESULT_SUCCESS;
  }


  Result SerialSink::close()
  {
    Result sinkResult = Result::RESULT_SUCCESS;

    if ( sink->end() != Chimera::Status::OK )
    {
      sinkResult = Result::RESULT_FAIL;
    }

    return sinkResult;
  }


  Result SerialSink::flush()
  {
    Result sinkResult = Result::RESULT_SUCCESS;

    if ( sink->flush( Chimera::Hardware::SubPeripheral::TXRX ) != Chimera::Status::OK )
    {
      sinkResult = Result::RESULT_FAIL;
    }

    return sinkResult;
  }


  IOType SerialSink::getIOType()
  {
    return IOType::SERIAL_SINK;
  }


  Result SerialSink::log( const Level level, const void *const message, const size_t length )
  {
    using namespace Chimera::Thread;

    /*------------------------------------------------
    Make sure we can actually log the data
    ------------------------------------------------*/
    if ( level < getLogLevel() )
    {
      return Result::RESULT_INVALID_LEVEL;
    }

    /*------------------------------------------------
    Write the data and block the current thread execution
    until the transfer is complete.
    ------------------------------------------------*/
    auto hwResult = Chimera::Status::OK;
    auto ulResult = Result::RESULT_SUCCESS;

    sink->lock();
    hwResult |= sink->write( message, length );
    hwResult |= sink->await( Chimera::Event::Trigger::TRIGGER_WRITE_COMPLETE, TIMEOUT_BLOCK );
    sink->unlock();

    if ( hwResult != Chimera::Status::OK )
    {
      ulResult = Result::RESULT_FAIL;
    }

    return ulResult;
  }

}  // namespace Aurora::Logging
