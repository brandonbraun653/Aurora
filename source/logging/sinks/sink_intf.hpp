/********************************************************************************
 *  File Name:
 *    sink_intf.hpp
 *
 *  Description:
 *    Defines the interface that sinks must implement at a bare minimum
 *
 *  2019-2021 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#pragma once
#ifndef MICRO_LOGGER_SINK_INTERFACE_HPP
#define MICRO_LOGGER_SINK_INTERFACE_HPP

/* C++ Includes */
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>

/* Chimera Includes */
#include <Chimera/thread>

/* Aurora Includes */
#include <Aurora/source/logging/logging_config.hpp>
#include <Aurora/source/logging/logging_types.hpp>

namespace Aurora::Logging
{
  /*-------------------------------------------------------------------------------
  Classes
  -------------------------------------------------------------------------------*/
  class SinkInterface : public Chimera::Thread::Lockable<SinkInterface>
  {
  public:
    SinkInterface()
    {
      mSinkEnabled  = false;
      mLoggingLevel = Level::LVL_MAX;
      mName         = "";
      mLogBuffer.fill( 0 );
    }

    virtual ~SinkInterface() = default;

    virtual Result open() = 0;

    virtual Result close() = 0;

    virtual Result flush() = 0;

    virtual IOType getIOType() = 0;

    /**
     *  Provides the core functionality of the sink by logging messages.
     *
     *  @note   Assume the memory can be modified/destroyed after return
     *
     *  @param[in]  level     The log level the message was sent at
     *  @param[in]  message   The message to be logged. Can be any kind of data
     *  @param[in]  length    How large the message is in bytes
     *  @return ResultType    Whether or not the logging action succeeded
     */
    virtual Result log( const Level level, const void *const message, const size_t length ) = 0;

    /**
     *  Enables the sink so logs can be processed
     */
    void enable()
    {
      mSinkEnabled = true;
    }

    /**
     *  Disables the sink so logs cannot be processed
     */
    void disable()
    {
      mSinkEnabled = false;
    }

    /**
     *  Checks if the sink is enabled
     *  @return bool
     */
    bool isEnabled()
    {
      return mSinkEnabled;
    }

    /**
     *  Sets the minimum log level threshold. This level, plus any higher priority
     *  levels, will be logged with the sink.
     *
     *  @param[in]  level   The minimum log level for this sink
     *  @return ResultType
     */
    void setLogLevel( const Level level )
    {
      mLoggingLevel = level;
    }

    /**
     *  Gets the log level currently assigned to the sink
     *
     *  @return Level
     */
    Level getLogLevel()
    {
      return mLoggingLevel;
    }

    /**
     *  Assigns a name to the sink
     *
     *  @warning  This naming should be unique
     *
     *  @param[in]  name    The name to be assigned
     *  @return Result
     */
    Result setName( const std::string_view &name )
    {
      mName = name;
      return Result::RESULT_SUCCESS;
    }

    /**
     *  Gets the name of the sink
     *
     *  @return std::string_view
     */
    std::string_view getName()
    {
      return mName;
    }

    /**
     *
     */
    template<typename... Args>
    Result flog( const Level lvl, const char *str, Args const &... args )
    {
      /*-------------------------------------------------
      Note to future me: If 'this' is null, you haven't
      initialized the sink yet.
      -------------------------------------------------*/
      auto result = Result::RESULT_SUCCESS;
      this->lock();

      /*------------------------------------------------
      Until custom formatters are available, simply dump the thread name in there
      ------------------------------------------------*/
      int bytesWritten = snprintf( mLogBuffer.data(), mLogBuffer.size(), "[%s] -- ", mName.data() );

      if ( bytesWritten < 0 )
      {
        this->unlock();
        return Result::RESULT_FAIL;
      }

      /*------------------------------------------------
      Attach the user's message, or what will fit anyways
      ------------------------------------------------*/
      snprintf( mLogBuffer.data() + bytesWritten, mLogBuffer.size() - bytesWritten, str, args... );
      result = log( lvl, mLogBuffer.data(), strlen( mLogBuffer.data() ) );

      this->unlock();

      return result;
    }

  private:
    friend Chimera::Thread::Lockable<SinkInterface>;


    Level mLoggingLevel;
    bool mSinkEnabled;
    std::string_view mName;
    std::array<char, ULOG_MAX_SNPRINTF_BUFFER_LENGTH> mLogBuffer;
  };

}  // namespace Aurora::Logging

#endif /* MICRO_LOGGER_SINK_INTERFACE_HPP */
