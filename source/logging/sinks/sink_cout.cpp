/********************************************************************************
 *  File Name:
 *    sink_cout.cpp
 *
 *  Description:
 *    Implementation of the std::cout based uLog sink
 *
 *  2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#if !defined( EMBEDDED )

/* C++ Includes */
#include <iostream>
#include <string>

/* Aurora Includes */
#include <Aurora/logging>

namespace Aurora::Logging
{
  /*-------------------------------------------------------------------------------
  Class Implementation
  -------------------------------------------------------------------------------*/
  CoutSink::CoutSink()
  {
  }


  CoutSink::~CoutSink()
  {
  }


  Result CoutSink::open()
  {
    return Result::RESULT_SUCCESS;
  }


  Result CoutSink::close()
  {
    return Result::RESULT_SUCCESS;
  }


  Result CoutSink::flush()
  {
    std::cout << " " << std::flush;
    return Result::RESULT_SUCCESS;
  }


  IOType CoutSink::getIOType()
  {
    return IOType::CONSOLE_SINK;
  }


  Result CoutSink::log( const Level level, const void *const message, const size_t length )
  {
    /*------------------------------------------------
    Check to see if we should even write
    ------------------------------------------------*/
    if ( !enabled || ( level < logLevel ) || !message || !length )
    {
      return Result::RESULT_FAIL;
    }

    /*------------------------------------------------
    Simply assign the data as a string, then write it.
    ------------------------------------------------*/
    std::string coutBuffer;
    coutBuffer.assign( reinterpret_cast<const char *const>( message ), length );
    std::cout << coutBuffer << std::flush;

    return Result::RESULT_SUCCESS;
  }
}  // namespace Aurora::Logging

#endif /* EMBEDDED */
