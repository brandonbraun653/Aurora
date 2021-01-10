/********************************************************************************
 *  File Name:
 *    serial_sink.hpp
 *
 *  Description:
 *    Defines the interface to a serial based sink for the uLogger system.
 *
 *  2019-2020 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#pragma once
#ifndef AURORA_LOGGING_SERIAL_SINK_HPP
#define AURORA_LOGGING_SERIAL_SINK_HPP

/* C++ Includes */
#include <cstdlib>

/* uLog Includes */
#include <uLog/ulog.hpp>
#include <uLog/types.hpp>
#include <uLog/sinks/sink_intf.hpp>

/* Chimera Includes */
#include <Chimera/serial>

namespace Aurora::Logging
{
  class SerialSink : public ::uLog::SinkInterface
  {
  public:
    /**
     *  Constructs a new serial sink object using a specific Serial channel.
     *  Typically used when the underlying serial driver is shared across
     *  multiple threads and has already been initialized.
     *
     *  @param[in]  channel   Which channel to hook into
     */
    explicit SerialSink( Chimera::Serial::Channel channel );
    SerialSink();
    ~SerialSink();

    /**
     *  Assigns the serial channel to use when the default
     *  constructor was used.
     *
     *  @param[in]  channel   Which channel to hook into
     *  @return void
     */
    void assignChannel( Chimera::Serial::Channel channel );

    ::uLog::Result open() final override;
    ::uLog::Result close() final override;
    ::uLog::Result flush() final override;
    ::uLog::IOType getIOType() final override;
    ::uLog::Result log( const ::uLog::Level level, const void *const message, const size_t length ) final override;

  private:
    Chimera::Serial::Channel mSerialChannel;
  };
}  // namespace Aurora::Logging

#endif /* !AURORA_LOGGING_SERIAL_SINK_HPP */
