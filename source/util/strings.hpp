/********************************************************************************
 *  File Name:
 *    strings.hpp
 *
 *  Description:
 *    String utilities
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_UTIL_STRING_HPP
#define AURORA_UTIL_STRING_HPP

/* STL Includes */
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstring>

/*-------------------------------------------------------------------------------
Public Functions
-------------------------------------------------------------------------------*/
int scnprintf( char *buf, size_t size, const char *fmt, ... );
int vscnprintf( char *buf, size_t size, const char *fmt, va_list args );

namespace Aurora::Utility
{
}  // namespace Aurora::Utility

#endif /* !AURORA_UTIL_STRING_HPP */
