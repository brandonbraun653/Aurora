/********************************************************************************
 *  File Name:
 *    math_constexpr.hpp
 *
 *  Description:
 *    Constexpr math
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_CONSTEXPR_MATH_HPP
#define AURORA_CONSTEXPR_MATH_HPP

/* STL Includes */
#include <cstdint>
#include <cstddef>

namespace Aurora::Math
{
  /**
   * @brief Checks if an input is a power of 2
   *
   * @tparam T      Type to use
   * @param v       Input value to check
   * @return true   Value is power of 2
   * @return false  Value is not power of 2
   */
  template<typename T>
  constexpr bool isPower2( T v )
  {
    return v && !( v & ( v - static_cast<T>( 1 ) ) );
  }
}  // namespace Aurora::Math

#endif /* !AURORA_CONSTEXPR_MATH_HPP */
