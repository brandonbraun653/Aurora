/********************************************************************************
 *  File Name:
 *    math.cpp
 *
 *  Description:
 *    Implementation of math functions
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* Aurora Includes */
#include <Aurora/math>

namespace Aurora::Math
{

  size_t intPow( const size_t x, const size_t p )
  {
    if( p == 0 ) return 1;
    if( p == 1 ) return x;

    size_t tmp = intPow( x, p / 2 );

    if( p % 2 == 0 )
    {
      return tmp * tmp;
    }
    else
    {
      return x * tmp * tmp;
    }
  }
}  // namespace Aurora::Math
