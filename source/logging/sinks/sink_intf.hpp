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
    bool enabled;
    Level logLevel;
    std::string_view name;

    SinkInterface() : enabled( false ), logLevel( Level::LVL_MAX ), name( "" )
    {
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

  private:
    friend Chimera::Thread::Lockable<SinkInterface>;
  };

}  // namespace Aurora::Logging

#endif /* MICRO_LOGGER_SINK_INTERFACE_HPP */
