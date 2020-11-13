/********************************************************************************
 *  File Name:
 *    math.cpp
 *
 *  Description:
 *    Implementation of math functions
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* STL Includes */
#include <cmath>

/* Aurora Includes */
#include <Aurora/math>

namespace Aurora::Math
{
  size_t intPow( const size_t x, const size_t p )
  {
    if ( p == 0 )
      return 1;
    if ( p == 1 )
      return x;

    size_t tmp = intPow( x, p / 2 );

    if ( p % 2 == 0 )
    {
      return tmp * tmp;
    }
    else
    {
      return x * tmp * tmp;
    }
  }


  bool isNearlyEqual( const float x, const float y, const float epsilon )
  {
    // see Knuth section 4.2.2 pages 217-218
    return std::abs( x - y ) <= ( epsilon * std::abs( x ) );
  }


  float percentError( const float actual, const float expected )
  {
    /*-------------------------------------------------
    Prevent blowing up the divisor below
    -------------------------------------------------*/
    if ( isNearlyEqual( expected, 0.0f, 1.0e-9f ) )
    {
      return 100.0f;
    }

    /*-------------------------------------------------
    Percent error calculation
    -------------------------------------------------*/
    float result = ( std::abs( actual - expected ) / expected ) * 100.0f;
    return result;
  }

}  // namespace Aurora::Math
