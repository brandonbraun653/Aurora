/********************************************************************************
 *  File Name:
 *    sink_jlink.cpp
 *
 *  Description:
 *    Implements the JLink based sink
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#if defined( EMBEDDED ) && defined( SEGGER_SYS_VIEW )

/* Aurora Includes */
#include <Aurora/logging>
#include <Aurora/tracing>

/* Chimera Includes */
#include <Chimera/event>
#include <Chimera/thread>


namespace Aurora::Logging
{
  /*-------------------------------------------------------------------------------
  Static Data
  -------------------------------------------------------------------------------*/
  static JLinkSink s_jlink_sink;


  /*-------------------------------------------------------------------------------
  Sink Implementation
  -------------------------------------------------------------------------------*/
  JLinkSink::JLinkSink()
  {
  }


  JLinkSink::~JLinkSink()
  {
  }


  JLinkSink &JLinkSink::getInstance()
  {
    return s_jlink_sink;
  }


  Result JLinkSink::open()
  {
    return Result::RESULT_SUCCESS;
  }


  Result JLinkSink::close()
  {
    return Result::RESULT_SUCCESS;
  }


  Result JLinkSink::flush()
  {
    return Result::RESULT_SUCCESS;
  }


  IOType JLinkSink::getIOType()
  {
    return IOType::JLINK_SINK;
  }


  Result JLinkSink::log( const Level level, const void *const message, const size_t length )
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
    switch( level )
    {
      case Level::LVL_ERROR:
        SEGGER_SYSVIEW_Error( reinterpret_cast<const char*>( message ) );
        break;

      case Level::LVL_WARN:
        SEGGER_SYSVIEW_Warn( reinterpret_cast<const char*>( message ) );
        break;

      default:
        SEGGER_SYSVIEW_Print( reinterpret_cast<const char*>( message ) );
        break;
    }

    return Result::RESULT_SUCCESS;
  }

}  // namespace Aurora::Logging

#endif  /* EMBEDDED && SEGGER_SYS_VIEW */
