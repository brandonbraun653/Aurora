/********************************************************************************
 *   File Name:
 *    sink_vgdb_semihosting.hpp
 *
 *   Description:
 *    Visual GDB ARM Semihosting Sink
 *
 *   2019-2020 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#pragma once
#ifndef MICRO_LOGGER_VISUAL_GDB_SEMIHOSTING_SINK_HPP
#define MICRO_LOGGER_VISUAL_GDB_SEMIHOSTING_SINK_HPP

/*------------------------------------------------
Visual GDB framework file that supports Semihosting
------------------------------------------------*/
#if defined( USING_VISUAL_GDB )
#define MICRO_LOGGER_HAS_VGDB_SEMIHOSTING   ( 1 )

/* C++ Includes */
#include <cstdlib>

/* uLog Includes */
#include <uLog/sinks/sink_intf.hpp>
#include <uLog/types.hpp>

namespace uLog
{
  class VGDBSemihostingSink : public SinkInterface
  {
  public:
    VGDBSemihostingSink();
    ~VGDBSemihostingSink();

    Result open() final override;

    Result close() final override;

    Result flush() final override;

    IOType getIOType() final override;

    Result log( const Level level, const void *const message, const size_t length ) final override;

  private:
    bool enabled;
    Level logLevel;
  };

}    // namespace uLog

#endif /* FastSemihosting.cpp */

#endif /* !MICRO_LOGGER_VISUAL_GDB_SEMIHOSTING_SINK_HPP */
