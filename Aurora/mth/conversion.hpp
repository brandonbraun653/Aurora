/********************************************************************************
 *  File Name:
 *    conversion.hpp
 *
 *  Description:
 *    Functions used to convert numerical entities
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_MATH_CONVERSION_HPP
#define AURORA_MATH_CONVERSION_HPP

/* STL Includes */
#include <cstddef>

namespace Aurora::Math
{
  enum class BaseType
  {
    BINARY,
    OCTAL,
    DECIMAL,
    HEX,

    NUM_OPTIONS,
    INVALID
  };

  /**
   *  Converts the given number in one base to another base
   *
   *  @param[in]  num     The original number
   *  @param[in]  from    The base converting from
   *  @param[in]  to      The base to convert into
   *  @return size_t      The converted number
   */
  size_t asBase( const size_t num, const BaseType from, const BaseType to );

}  // namespace Aurora::Math

#endif  /* !AURORA_MATH_CONVERSION_HPP */
