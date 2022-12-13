/******************************************************************************
 *  File Name:
 *    sink_file.cpp
 *
 *  Description:
 *    File sink implementation
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *****************************************************************************/

#if !defined( EMBEDDED )

/* C++ Includes */
#include <iostream>
#include <string>

/* Aurora Includes */
#include <Aurora/logging>

namespace Aurora::Logging
{
  /*---------------------------------------------------------------------------
  Class Implementation
  ---------------------------------------------------------------------------*/
  FileSink::FileSink()
  {
  }


  FileSink::~FileSink()
  {
  }


  void FileSink::setFile( const std::string_view &file )
  {

  }


  Result FileSink::open()
  {
    return Result::RESULT_SUCCESS;
  }


  Result FileSink::close()
  {
    return Result::RESULT_SUCCESS;
  }


  Result FileSink::flush()
  {
    return Result::RESULT_SUCCESS;
  }


  IOType FileSink::getIOType()
  {
    return IOType::FILE_SINK;
  }


  Result FileSink::log( const Level level, const void *const message, const size_t length )
  {
    /*-------------------------------------------------------------------------
    Check to see if we should even write
    -------------------------------------------------------------------------*/
    if ( !enabled || ( level < logLevel ) || !message || !length )
    {
      return Result::RESULT_FAIL;
    }

    return Result::RESULT_SUCCESS;
  }
}  // namespace Aurora::Logging

#endif  /* !EMBEDDED */
