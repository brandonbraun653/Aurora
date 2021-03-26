/********************************************************************************
 *  File Name:
 *    sink_jlink.hpp
 *
 *  Description:
 *    Logger sink for dumping to the Segger SystemView software
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_LOGGING_SEGGER_SYSTEM_VIEW_HPP
#define AURORA_LOGGING_SEGGER_SYSTEM_VIEW_HPP

/* C++ Includes */
#include <cstdlib>

/* Aurora Includes */
#include <Aurora/source/logging/logging_types.hpp>
#include <Aurora/source/logging/sinks/sink_intf.hpp>

namespace Aurora::Logging
{
  /*-------------------------------------------------------------------------------
  Classes
  -------------------------------------------------------------------------------*/
  class JLinkSink : public SinkInterface
  {
  public:
    JLinkSink();
    ~JLinkSink();

    static JLinkSink &getInstance();

    Result open() final override;
    Result close() final override;
    Result flush() final override;
    IOType getIOType() final override;
    Result log( const Level level, const void *const message, const size_t length ) final override;

  private:
  };

}  // namespace Aurora::Logging

#endif /* !AURORA_LOGGING_SEGGER_SYSTEM_VIEW_HPP */
