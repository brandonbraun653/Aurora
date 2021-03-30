/********************************************************************************
 *  File Name:
 *    logging_driver.hpp
 *
 *  Description:
 *    uLog attempts to be a fairly simple logger for embedded systems
 *
 *  2019-2021 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#pragma once
#ifndef AURORA_LOGGING_DRIVER_HPP
#define AURORA_LOGGING_DRIVER_HPP

/* C++ Includes */
#include <array>
#include <cstdlib>
#include <cstring>
#include <string>

/* Aurora Includes */
#include <Aurora/source/logging/logging_config.hpp>
#include <Aurora/source/logging/logging_types.hpp>

namespace Aurora::Logging
{
  /*-------------------------------------------------------------------------------
  Public Functions
  -------------------------------------------------------------------------------*/
  /**
   *  Initializes the backend driver
   *
   *  @return void
   */
  void initialize();

  /**
   *  Sets a minimum log level that is needed to emit messages
   *  to registered sinks
   *
   *  @param[in]  level      The global log level to be set
   *  @return SinkHandleType
   */
  Result setGlobalLogLevel( const Level level );

  /**
   *  Registers a sink with the back end driver
   *
   *  @param[in]  sink      The sink to be registered
   *  @return SinkHandleType
   */
  Result registerSink( SinkHandle &sink, const Config options = CFG_NONE );

  /**
   *  Removes the associated sink.
   *
   *  @note If nullptr is passed in, all sinks are removed.
   *
   *  @param[in]  sink      The sink that should be removed
   *  @return ResultType
   */
  Result removeSink( SinkHandle &sink );

  /**
   *  Sets the default global logger instance
   *
   *  @param[in]  sink      The sink to become the root
   *  @return Result
   */
  Result setRootSink( SinkHandle &sink );

  /**
   *  Gets the default global logger instance
   *
   *  @return SinkHandle
   */
  SinkHandle getRootSink();

  /**
   *  Attempts to log to every registered sink. Each sink determines if the message
   *  should be logged with them depending on the sink specific logging level.
   *
   *  @param[in]  lvl       The severity level of the message to be logged
   *  @param[in]  msg       Raw byte message to be logged
   *  @param[in]  length    Length of the log message
   *  @return Result
   */
  Result log( const Level lvl, const void *const msg, const size_t length );

  /**
   *  Logs a formatted string to every registered sink that is listening to the
   *  requested logging level.
   *
   *  @param[in]  lvl       The severity level of the message to be logged
   *  @param[in]  file      Which file this is being logged from
   *  @param[in]  line      Line this is being logged from
   *  @param[in]  fmt       Format string
   *  @return Result
   */
  Result flog( const Level lvl, const char *const file, const size_t line, const char *fmt, ... );

}  // namespace Aurora::Logging

#endif /* AURORA_LOGGING_DRIVER_HPP */
