/******************************************************************************
 *  File Name:
 *    logging_driver.cpp
 *
 *  Description:
 *    Logging implementation
 *
 *  2019-2023 | Brandon Braun | brandonbraun653@gmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <array>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <string>
#include <Chimera/system>
#include <Chimera/thread>
#include <Aurora/logging>

namespace Aurora::Logging
{
  /*---------------------------------------------------------------------------
  Constants
  ---------------------------------------------------------------------------*/
  static constexpr size_t LOG_BUF_SIZE = 512;

  /*---------------------------------------------------------------------------
  Static Data
  ---------------------------------------------------------------------------*/
  static bool                                                     uLogInitialized    = false;
  static Level                                                    globalLogLevel     = Level::LVL_MIN;
  static SinkHandle_rPtr                                          globalRootSink     = nullptr;
  static size_t                                                   defaultLockTimeout = 100;
  static char                                                     s_log_buffer[ LOG_BUF_SIZE ];
  static Chimera::Thread::RecursiveTimedMutex                     threadLock;
  static Chimera::Thread::Mutex                                   s_format_lock;
  static std::array<SinkHandle_rPtr, ULOG_MAX_REGISTERABLE_SINKS> sinkRegistry;


  /*---------------------------------------------------------------------------
  Static Functions
  ---------------------------------------------------------------------------*/
  /**
   *  Looks up the registry index associated with a particular sink handle
   *
   *  @param[in]  sinkHandle  The handle to search for
   *  @return size_t
   */
  static size_t getSinkOffsetIndex( const SinkHandle_rPtr &sinkHandle );


  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/
  void initialize()
  {
    Chimera::Thread::LockGuard x( threadLock );

    if ( !uLogInitialized )
    {
      sinkRegistry.fill( nullptr );
      uLogInitialized = true;
    }
  }


  Result setGlobalLogLevel( const Level level )
  {
    Chimera::Thread::LockGuard x( threadLock );

    globalLogLevel = level;
    return Result::RESULT_SUCCESS;
  }


  Result registerSink( SinkHandle_rPtr &sink, const Config options )
  {
    constexpr size_t invalidIndex = std::numeric_limits<size_t>::max();

    Chimera::Thread::TimedLockGuard x( threadLock );
    size_t                          nullIndex        = invalidIndex; /* First index that doesn't have a sink registered */
    bool                            sinkIsRegistered = false; /* Indicates if the sink we are registering already exists */
    bool                            registryIsFull   = true;  /* Is the registry full of sinks? */
    auto                            result           = Result::RESULT_SUCCESS; /* Function return code */

    if ( x.try_lock_for( defaultLockTimeout ) )
    {
      /*-----------------------------------------------------------------------
      Check if the sink already is registered as well as
      an empty slot to insert the new sink.
      -----------------------------------------------------------------------*/
      for ( size_t i = 0; i < sinkRegistry.size(); i++ )
      {
        /* Did we find the first location that is free? */
        if ( ( nullIndex == invalidIndex ) && ( sinkRegistry[ i ] == nullptr ) )
        {
          nullIndex      = i;
          registryIsFull = false;
        }

        /* Does the sink already exist in the registry? */
        if ( sinkRegistry[ i ] == sink )
        {
          sinkIsRegistered = true;
          registryIsFull   = false;
          result           = Result::RESULT_SUCCESS;
          break;
        }
      }

      /*-----------------------------------------------------------------------
      Perform the registration
      -----------------------------------------------------------------------*/
      if ( !sinkIsRegistered )
      {
        if ( registryIsFull )
        {
          result = Result::RESULT_FULL;
        }
        else if ( sink->open() != Result::RESULT_SUCCESS )
        {
          result = Result::RESULT_FAIL;
        }
        else
        {
          sinkRegistry[ nullIndex ] = sink;
        }
      }
    }

    return result;
  }


  Result removeSink( SinkHandle_rPtr &sink )
  {
    Result                          result = Result::RESULT_LOCKED;
    Chimera::Thread::TimedLockGuard x( threadLock );

    if ( x.try_lock_for( defaultLockTimeout ) )
    {
      auto index = getSinkOffsetIndex( sink );
      if ( index < sinkRegistry.size() )
      {
        sinkRegistry[ index ]->close();
        sinkRegistry[ index ] = nullptr;
        result                = Result::RESULT_SUCCESS;
      }
      else if ( sink == nullptr )
      {
        for ( size_t i = 0; i < sinkRegistry.size(); i++ )
        {
          if ( sinkRegistry[ i ] )
          {
            sinkRegistry[ i ]->close();
            sinkRegistry[ i ] = nullptr;
          }
        }

        result = Result::RESULT_SUCCESS;
      }
    }

    return result;
  }


  Result setRootSink( SinkHandle_rPtr &sink )
  {
    Result                          result = Result::RESULT_LOCKED;
    Chimera::Thread::TimedLockGuard x( threadLock );

    if ( x.try_lock_for( defaultLockTimeout ) )
    {
      globalRootSink = sink;
      result         = Result::RESULT_SUCCESS;
    }

    return result;
  }


  SinkHandle_rPtr getRootSink()
  {
    return globalRootSink;
  }


  size_t getSinkOffsetIndex( const SinkHandle_rPtr &sinkHandle )
  {
    /*-------------------------------------------------------------------------
    Figure out the real addresses and boundary limit them

    Note: I'm going to run into trouble if packing is weird...
    -------------------------------------------------------------------------*/
    std::uintptr_t offsetAddress = reinterpret_cast<std::uintptr_t>( &sinkHandle );
    std::uintptr_t beginAddress  = reinterpret_cast<std::uintptr_t>( &sinkRegistry[ 0 ] );
    std::uintptr_t secondAddress = reinterpret_cast<std::uintptr_t>( &sinkRegistry[ 1 ] );
    size_t         elementSize   = secondAddress - beginAddress;

    if ( ( sinkHandle == nullptr ) || ( beginAddress > offsetAddress ) || !elementSize )
    {
      return std::numeric_limits<size_t>::max();
    }

    /*-------------------------------------------------------------------------
    Calculate the index
    -------------------------------------------------------------------------*/
    size_t index = ( offsetAddress - beginAddress ) / elementSize;

    if ( index > sinkRegistry.size() )
    {
      index = std::numeric_limits<size_t>::max();
    }

    return index;
  }


  Result log( const Level level, const void *const message, const size_t length )
  {
    /*-------------------------------------------------------------------------
    Input boundary checking
    -------------------------------------------------------------------------*/
    Chimera::Thread::TimedLockGuard x( threadLock );
    if ( !x.try_lock_for( defaultLockTimeout ) )
    {
      return Result::RESULT_LOCKED;
    }
    else if ( ( level < globalLogLevel ) || !message || !length )
    {
      return Result::RESULT_FAIL;
    }

    /*-------------------------------------------------------------------------
    Process the message through each sink. At the moment
    we won't concern ourselves if a sink failed to log.
    -------------------------------------------------------------------------*/
    for ( size_t i = 0; i < sinkRegistry.size(); i++ )
    {
      if ( sinkRegistry[ i ] && ( sinkRegistry[ i ]->logLevel >= globalLogLevel ) )
      {
        sinkRegistry[ i ]->log( level, message, length );
      }
    }

    return Result::RESULT_SUCCESS;
  }


  Result flog( const Level lvl, const char *const file, const size_t line, const char *fmt, ... )
  {
    /*-------------------------------------------------------------------------
    Input boundary checking
    -------------------------------------------------------------------------*/
    if ( ( lvl < globalLogLevel ) || !file || !fmt )
    {
      return Result::RESULT_FAIL;
    }

    /*-------------------------------------------------------------------------
    Create the logging level
    -------------------------------------------------------------------------*/
    std::string_view str_level = "";
    switch ( lvl )
    {
      case Level::LVL_TRACE:
        str_level = "TRACE";
        break;

      case Level::LVL_DEBUG:
        str_level = "DEBUG";
        break;

      case Level::LVL_INFO:
        str_level = "INFO";
        break;

      case Level::LVL_WARN:
        str_level = "WARN";
        break;

      case Level::LVL_ERROR:
        str_level = "ERROR";
        break;

      case Level::LVL_FATAL:
        str_level = "FATAL";
        break;

      default:
        return Result::RESULT_INVALID_LEVEL;
    };

    /*-------------------------------------------------------------------------
    Format the message header
    -------------------------------------------------------------------------*/
    RT_DBG_ASSERT( Chimera::System::inISR() == false );
    Chimera::Thread::LockGuard _lock( s_format_lock );

    memset( s_log_buffer, 0, LOG_BUF_SIZE );
    npf_snprintf( s_log_buffer, LOG_BUF_SIZE, "%u | %s:%u | %s | ",
                  Chimera::millis(), file, line, str_level.data() );

    /*-------------------------------------------------------------------------
    Format the user message
    -------------------------------------------------------------------------*/
    const size_t offset = strlen( s_log_buffer );

    va_list argptr;
    va_start( argptr, fmt );
    npf_vsnprintf( s_log_buffer + offset, LOG_BUF_SIZE - offset, fmt, argptr );
    va_end( argptr );

    /*-------------------------------------------------------------------------
    Ensure the message terminates with a carriage return and newline
    -------------------------------------------------------------------------*/
    const size_t msg_len = strlen( s_log_buffer );

    bool ends_with_crlf = false;
    if( ( msg_len >= 2 ) && ( msg_len < LOG_BUF_SIZE ) )
    {
      if( s_log_buffer[ msg_len - 2 ] == '\r' && s_log_buffer[ msg_len - 1 ] == '\n' )
      {
        ends_with_crlf = true;
      }
      else if( s_log_buffer[ msg_len - 2 ] == '\n' && s_log_buffer[ msg_len - 1 ] == '\r' )
      {
        ends_with_crlf = true;
      }
    }

    if( !ends_with_crlf )
    {
      if ( msg_len < LOG_BUF_SIZE - 2 )
      {
        s_log_buffer[ msg_len ]     = '\r';
        s_log_buffer[ msg_len + 1 ] = '\n';
      }
      else
      {
        s_log_buffer[ LOG_BUF_SIZE - 2 ] = '\r';
        s_log_buffer[ LOG_BUF_SIZE - 1 ] = '\n';
      }
    }

    /*-------------------------------------------------------------------------
    Log through the standard method
    -------------------------------------------------------------------------*/
    return log( lvl, s_log_buffer, strlen( s_log_buffer ) );
  }

}  // namespace Aurora::Logging