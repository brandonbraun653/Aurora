/******************************************************************************
 *  File Name:
 *    serial_sink->cpp
 *
 *  Description:
 *    Implements a serial based sink for the uLogger interface.
 *
 *  2019-2022 | Brandon Braun | brandonbraun653@gmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/logging>
#include <Chimera/event>
#include <Chimera/serial>
#include <Chimera/thread>


namespace Aurora::Logging
{
  /*---------------------------------------------------------------------------
  Driver Implementation
  ---------------------------------------------------------------------------*/
  SerialSink::SerialSink( Chimera::Serial::Channel channel ) : SinkInterface(), mSerial( nullptr )
  {
  }


  SerialSink::SerialSink() : mSerial( nullptr )
  {
  }


  SerialSink::~SerialSink()
  {
  }


  void SerialSink::assignChannel( Chimera::Serial::Channel channel )
  {
    mSerial = Chimera::Serial::getDriver( channel );
    RT_DBG_ASSERT( mSerial );
  }


  Result SerialSink::open()
  {
    if ( !mSerial )
    {
      return Result::RESULT_FAIL;
    }

    return Result::RESULT_SUCCESS;
  }


  Result SerialSink::close()
  {
    mSerial = nullptr;
    return Result::RESULT_SUCCESS;
  }


  Result SerialSink::flush()
  {
    return Result::RESULT_SUCCESS;
  }


  IOType SerialSink::getIOType()
  {
    return IOType::SERIAL_SINK;
  }


  Result SerialSink::log( const Level level, const void *const message, const size_t length )
  {
    using namespace Chimera::Thread;

    /*-------------------------------------------------------------------------
    Make sure we can actually log the data
    -------------------------------------------------------------------------*/
    if( mSerial == nullptr )
    {
      return Result::RESULT_FAIL_BAD_SINK;
    }

    if ( level < logLevel )
    {
      return Result::RESULT_INVALID_LEVEL;
    }

    /*-------------------------------------------------------------------------
    Write the data and block the current thread execution until complete.
    -------------------------------------------------------------------------*/
    if ( mSerial->write( message, length, TIMEOUT_BLOCK ) == Chimera::Status::OK )
    {
      return Result::RESULT_SUCCESS;
    }
    else
    {
      return Result::RESULT_FAIL;
    }
  }

}  // namespace Aurora::Logging
