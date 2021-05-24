/********************************************************************************
 *  File Name:
 *    math.hpp
 *
 *  Description:
 *    Math functions
 *
 *  2020-2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_MATH_HPP
#define AURORA_MATH_HPP

/* STL Includes */
#include <cstddef>
#include <cstdint>

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

  /**
   *  Checks to see if X and Y are nearly equal to each other within
   *  a tolerance of epsilon.
   *
   *  @param[in]  x           Value 1 to be compared
   *  @param[in]  y           Value 2 to be compared
   *  @param[in]  epsilon     A small tolerance value (1e-9 perhaps)
   *  @return bool            True if they are equal to within epsilon
   */
  bool isNearlyEqual( const float x, const float y, const float epsilon );

  /**
   *  Calculates the percent error between the given values
   *
   *  @param[in]  actual      The measured value
   *  @param[in]  expected    The expected value
   *  @return float           Percent error
   */
  float percentError( const float actual, const float expected );

  /**
   * @brief Gets position of the highest bit set in a number
   *
   * @param value     Value to check
   * @return size_t
   */
  size_t maxBitSet( uint32_t value );

  /**
   * @brief Gets position of the highest bit set in a number
   * Assumes number is a power of 2.
   *
   * @param value     Value to check
   * @return size_t
   */
  size_t maxBitSetPow2( uint32_t value );

}  // namespace Aurora::Math

#endif  /* !AURORA_MATH_HPP */
