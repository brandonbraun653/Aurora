/********************************************************************************
 *  File Name:
 *    logging_types.hpp
 *
 *  Description:
 *    Logger types
 *
 *  2019-2021 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#pragma once
#ifndef MICRO_LOGGER_TYPES_HPP
#define MICRO_LOGGER_TYPES_HPP

/* C++ Includes */
#include <cstdint>
#include <memory>

namespace Aurora::Logging
{
  /*-------------------------------------------------------------------------------
  Forward Declarations
  -------------------------------------------------------------------------------*/
  class SinkInterface;


  /*-------------------------------------------------------------------------------
  Aliases
  -------------------------------------------------------------------------------*/
  using SinkHandle = std::shared_ptr<SinkInterface>;


  /*-------------------------------------------------------------------------------
  Enumerations
  -------------------------------------------------------------------------------*/
  enum class Result : size_t
  {
    RESULT_SUCCESS,
    RESULT_FAIL,
    RESULT_FAIL_MSG_TOO_LONG,
    RESULT_FAIL_BAD_SINK,
    RESULT_LOCKED,
    RESULT_FULL,
    RESULT_INVALID_LEVEL
  };

  /**
   *  The supported logging level types for all log sinks. An increasing numerical
   *  value corresponds with an increasing priority.
   */
  enum class Level : size_t
  {
    LVL_TRACE,
    LVL_DEBUG,
    LVL_INFO,
    LVL_WARN,
    LVL_ERROR,
    LVL_FATAL,

    LVL_MIN = LVL_TRACE,
    LVL_MAX = LVL_FATAL
  };

  enum Config : size_t
  {
    CFG_NONE                            = 0,
    CFG_INITIALIZE_ALWAYS               = ( 1u << 0 ), /**< Like the name says, always initialize */
    CFG_INITIALIZE_IFF_SINK_UNIQUE_TYPE = ( 1u << 1 )  /**< Only initialize the sink if it's the only one of its kind */
  };

  enum class IOType : size_t
  {
    CONSOLE_SINK,
    FILE_SINK,
    JLINK_SINK,
    SERIAL_SINK,
    VGDB_SINK
  };

}  // namespace Aurora::Logging

#endif /* MICRO_LOGGER_TYPES_HPP */