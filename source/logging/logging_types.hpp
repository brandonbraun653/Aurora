/********************************************************************************
 *  File Name:
 *    logging_types.hpp
 *
 *  Description:
 *    Logger types
 *
 *  2019-2022 | Brandon Braun | brandonbraun653@gmail.com
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
  using SinkHandle_rPtr = SinkInterface*;


  /*---------------------------------------------------------------------------
  Constants
  ---------------------------------------------------------------------------*/
  namespace Terminal
  {
    static constexpr std::array<uint8_t, 4> CmdClearScreen = { 0x1B, 0x5B, 0x32, 0x4A };
  }

  /*-------------------------------------------------------------------------------
  Enumerations
  -------------------------------------------------------------------------------*/
  enum class Result : size_t
  {
    RESULT_SUCCESS,
    RESULT_IGNORE,
    RESULT_FAIL,
    RESULT_FAIL_MSG_TOO_LONG,
    RESULT_FAIL_BAD_SINK,
    RESULT_NO_MEM,
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