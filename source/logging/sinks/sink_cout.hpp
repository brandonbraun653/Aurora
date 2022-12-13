/******************************************************************************
 *  File Name:
 *    sink_cout.hpp
 *
 *  Description:
 *    Implements a sink for std::cout
 *
 *  2019-2021 | Brandon Braun | brandonbraun653@gmail.com
 *****************************************************************************/

#pragma once
#ifndef MICRO_LOGGER_SINK_COUT_HPP
#define MICRO_LOGGER_SINK_COUT_HPP

/* C++ Includes */
#include <cstdlib>

/* Logging Includes */
#include <Aurora/source/logging/sinks/sink_intf.hpp>
#include <Aurora/source/logging/logging_types.hpp>

namespace Aurora::Logging
{
  class CoutSink : public SinkInterface
  {
  public:
    CoutSink();
    ~CoutSink();

    Result open() final override;
    Result close() final override;
    Result flush() final override;
    IOType getIOType() final override;
    Result log( const Level level, const void *const message, const size_t length ) final override;
  };
}  // namespace Aurora::Logging

#endif /* MICRO_LOGGER_SINK_COUT_HPP */
