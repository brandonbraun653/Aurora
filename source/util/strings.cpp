/********************************************************************************
 *  File Name:
 *    strings.cpp
 *
 *  Description:
 *    Implementation of string utilities
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* STL Includes */
#include <cstdio>

/* Aurora Includes */
#include <Aurora/utility>

/*-------------------------------------------------------------------------------
Macros
-------------------------------------------------------------------------------*/
#ifndef likely
#define likely( x ) ( __builtin_expect( !!( x ), 1 ) )
#endif
#ifndef unlikely
#define unlikely( x ) ( __builtin_expect( !!( x ), 0 ) )
#endif

/*-------------------------------------------------------------------------------
Public Functions
-------------------------------------------------------------------------------*/
/**
 * @brief Format a string and place it in a buffer
 *
 * Taken from the Linux kernel. The return value is the number of characters which
 * have been written, not including the trailing '\0'. If size is == 0 the function returns 0.
 *
 * @param buf     The buffer to place the result into
 * @param size    The size of the buffer, including the trailing null space
 * @param fmt     The format string to use
 * @param ...     Arguments for the format string
 * @return int    Bytes actually written, not including trailing '\0'
 */
int scnprintf( char *buf, size_t size, const char *fmt, ... )
{
  va_list args;
  int i;

  va_start( args, fmt );
  i = vscnprintf( buf, size, fmt, args );
  va_end( args );

  return i;
}


/**
 * @brief Format a string and place it in a buffer
 *
 * Taken from the Linux kernel. The return value is the number of characters which
 * have been written, not including the trailing '\0'. If size is == 0 the function returns 0.
 *
 * @param buf     The buffer to place the result into
 * @param size    The size of the buffer, including the trailing null space
 * @param fmt     The format string to use
 * @param args    Arguments for the format string
 * @return int    Bytes actually written, not including trailing '\0'
 */
int vscnprintf( char *buf, size_t size, const char *fmt, va_list args )
{
  int i;

  i = vsnprintf( buf, size, fmt, args );

  if ( likely( i < size ) )
    return i;
  if ( size != 0 )
    return size - 1;
  return 0;
}


char *safe_strcpy( char *dest, size_t size, const char *src )
{
  if ( size > 0 )
  {
    size_t i;
    for ( i = 0; i < size - 1 && src[ i ]; i++ )
    {
      dest[ i ] = src[ i ];
    }
    dest[ i ] = '\0';
  }
  return dest;
}


namespace Aurora::Utility
{
}  // namespace Aurora::Utility
