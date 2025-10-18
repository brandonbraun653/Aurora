/******************************************************************************
 *  File Name:
 *    logging_macro.hpp
 *
 *  Description:
 *    Helper macros for logging
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *****************************************************************************/

#pragma once
#ifndef LOGGING_MACROS_HPP
#define LOGGING_MACROS_HPP

/* Aurora Includes */
#include <Aurora/source/logging/logging_driver.hpp>

/*-------------------------------------------------------------------------------
Create the __SHORT_FILE__ macro, which returns just the file name instead of the
whole path from __FILE__ https://blog.galowicz.de/2016/02/20/short_file_macro/
-------------------------------------------------------------------------------*/
namespace Aurora::Logging::detail
{
  using cstr = const char *;

  constexpr cstr short_file_name( cstr path ) noexcept
  {
    cstr filename = path;
    for ( cstr current = path; *current != '\0'; ++current )
    {
      if ( ( *current == '/' ) || ( *current == '\\' ) )
      {
        filename = current + 1;
      }
    }

    return filename;
  }
}  // namespace Aurora::Logging::detail

#define __SHORT_FILE__ ::Aurora::Logging::detail::short_file_name( __FILE__ )

/*-------------------------------------------------------------------------------
Logging helper macros
-------------------------------------------------------------------------------*/
#define LOG_TRACE( str, ... ) \
  Aurora::Logging::flog( Aurora::Logging::Level::LVL_TRACE, __SHORT_FILE__, __LINE__, str, ##__VA_ARGS__ )
#define LOG_TRACE_IF( predicate, str, ... ) \
  if ( ( predicate ) )                      \
  {                                         \
    LOG_TRACE( ( str ), ##__VA_ARGS__ );    \
  }


#define LOG_DEBUG( str, ... ) \
  Aurora::Logging::flog( Aurora::Logging::Level::LVL_DEBUG, __SHORT_FILE__, __LINE__, str, ##__VA_ARGS__ )
#define LOG_DEBUG_IF( predicate, str, ... ) \
  if ( ( predicate ) )                      \
  {                                         \
    LOG_DEBUG( ( str ), ##__VA_ARGS__ );    \
  }


#define LOG_INFO( str, ... ) \
  Aurora::Logging::flog( Aurora::Logging::Level::LVL_INFO, __SHORT_FILE__, __LINE__, str, ##__VA_ARGS__ )
#define LOG_INFO_IF( predicate, str, ... ) \
  if ( ( predicate ) )                     \
  {                                        \
    LOG_INFO( ( str ), ##__VA_ARGS__ );    \
  }


#define LOG_WARN( str, ... ) \
  Aurora::Logging::flog( Aurora::Logging::Level::LVL_WARN, __SHORT_FILE__, __LINE__, str, ##__VA_ARGS__ )
#define LOG_WARN_IF( predicate, str, ... ) \
  if ( ( predicate ) )                     \
  {                                        \
    LOG_WARN( ( str ), ##__VA_ARGS__ );    \
  }


#define LOG_ERROR( str, ... ) \
  Aurora::Logging::flog( Aurora::Logging::Level::LVL_ERROR, __SHORT_FILE__, __LINE__, str, ##__VA_ARGS__ )
#define LOG_ERROR_IF( predicate, str, ... ) \
  if ( ( predicate ) )                      \
  {                                         \
    LOG_ERROR( ( str ), ##__VA_ARGS__ );    \
  }


#define LOG_FATAL( str, ... ) \
  Aurora::Logging::flog( Aurora::Logging::Level::LVL_FATAL, __SHORT_FILE__, __LINE__, str, ##__VA_ARGS__ )
#define LOG_FATAL_IF( predicate, str, ... ) \
  if ( ( predicate ) )                      \
  {                                         \
    LOG_FATAL( ( str ), ##__VA_ARGS__ );    \
  }

#endif /* !LOGGING_MACROS_HPP */
