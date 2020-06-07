/********************************************************************************
 *  File Name:
 *    math.hpp
 *
 *  Description:
 *    Math functions
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_MATH_HPP
#define AURORA_MATH_HPP

/* STL Includes */
#include <cstddef>

namespace Aurora::Math
{

  /**
   *  Calculates the power of two positive integers in O(log2(p))
   *
   *  @note Optimized for integers rather than c-style pow() function
   *
   *  @param[in]  x   The number to be exponentiated
   *  @param[in]  p   The power to exponentiate to
   *  @return size_t
   */
  size_t intPow( const size_t x, const size_t p );

}  // namespace Aurora::Math

#endif  /* !AURORA_MATH_HPP */
