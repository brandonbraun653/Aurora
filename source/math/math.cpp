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


  size_t maxBitSet( uint32_t value )
  {
    unsigned int v         = value;  // 32-bit value to find the log2 of
    const unsigned int b[] = { 0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000 };
    const unsigned int S[] = { 1, 2, 4, 8, 16 };
    int i;

    unsigned int r = 0;  // result of log2(v) will go here
    for ( i = 4; i >= 0; i-- )    // unroll for speed...
    {
      if ( v & b[ i ] )
      {
        v >>= S[ i ];
        r |= S[ i ];
      }
    }

    return r;
  }


  size_t maxBitSetPow2( uint32_t value )
  {
    unsigned int v = value;  // 32-bit value to find the log2 of
    static const unsigned int b[] = { 0xAAAAAAAA, 0xCCCCCCCC, 0xF0F0F0F0, 0xFF00FF00, 0xFFFF0000 };
    unsigned int r       = ( v & b[ 0 ] ) != 0;
    int i;

    for ( i = 4; i > 0; i-- )  // unroll for speed...
    {
      r |= ( ( v & b[ i ] ) != 0 ) << i;
    }

    return r;
  }

}  // namespace Aurora::Math
