/********************************************************************************
 *  File Name:
 *    sink_file.hpp
 *
 *  Description:
 *    File sink interface
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_LOGGING_FILE_SINK_HPP
#define AURORA_LOGGING_FILE_SINK_HPP

/* C++ Includes */
#include <cstdlib>
#include <cstring>

/* Aurora Includes */
#include <Aurora/source/logging/logging_types.hpp>
#include <Aurora/source/logging/sinks/sink_intf.hpp>


namespace Aurora::Logging
{
  class FileSink : public SinkInterface
  {
  public:
    FileSink();
    ~FileSink();

    /**
     *  Assigns the serial channel to use when the default
     *  constructor was used.
     *
     *  @param[in]  channel   Which channel to hook into
     *  @return void
     */
    void setFile( const std::string_view &file );

    Result open() final override;
    Result close() final override;
    Result flush() final override;
    IOType getIOType() final override;
    Result log( const Level level, const void *const message, const size_t length ) final override;

  private:
    std::string_view mFile;
  };
}  // namespace Aurora::Logging

#endif  /* !AURORA_LOGGING_FILE_SINK_HPP */
