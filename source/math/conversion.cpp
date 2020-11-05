/********************************************************************************
 *  File Name:
 *    conversion.cpp
 *
 *  Description:
 *    Implementation of mathematical conversion function
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* STL Includes */
#include <cmath>
#include <cstddef>

/* Aurora Includes */
#include <Aurora/math>

namespace Aurora::Math
{

  size_t asBase( const size_t num, const BaseType from, const BaseType to )
  {
    size_t iter = 0;
    size_t shift = 0;
    size_t mask = 0;
    size_t newBase = 1;



    switch( to )
    {
      case BaseType::BINARY:
        newBase = 2;
        break;

      case BaseType::DECIMAL:
        newBase = 10;
        break;

      case BaseType::OCTAL:
        newBase = 8;
        break;

      case BaseType::HEX:
        newBase = 16;
        break;

      default:
        return 0;
        break;
    };

    /*-------------------------------------------------
    Convert the number into a base 10 system first
    -------------------------------------------------*/
    size_t num_as_base10 = num;
    if( from != BaseType::DECIMAL )
    {
      size_t tmp    = num;
      num_as_base10 = 0;

      switch ( from )
      {
        case BaseType::BINARY:
          shift = 1;
          mask  = 1;
          break;

        case BaseType::OCTAL:
          shift = 3;
          mask  = 0x03;
          break;

        case BaseType::HEX:
          shift = 4;
          mask  = 0x0F;
          break;

        case BaseType::DECIMAL:
        default:
          shift = 0;
          break;
      };


      while( tmp )
      {
        num_as_base10 += ( tmp & mask ) * intPow( 10, iter );
        tmp >>= shift;
        iter++;
      }
    }

    /*-------------------------------------------------
    Convert from base 10 into the new base
    -------------------------------------------------*/
    iter = 0;
    size_t num_as_baseX = 0;

    while( num_as_base10 )
    {
      size_t remainder = num % newBase;
      size_t multiplier = intPow( newBase, iter );


      num_as_baseX  += remainder * multiplier;
      num_as_base10 /= newBase;
      iter++;
    }

    return num_as_baseX;
  }
}  // namespace Aurora::Math
