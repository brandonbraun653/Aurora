/********************************************************************************
 *  File Name:
 *    enum.hpp
 *
 *  Description:
 *    Utility for enum class operations
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_UTIL_ENUM_HPP
#define AURORA_UTIL_ENUM_HPP

/* STL Includes */
#include <type_traits>


/**
 * @brief Convert enum class member into its underlying type
 *
 * @tparam E Enum class
 * @param e Enum value
 * @return constexpr std::underlying_type<E>::type
 */
template<typename E>
constexpr typename std::underlying_type<E>::type EnumValue( E e ) noexcept
{
  return static_cast<typename std::underlying_type<E>::type>( e );
}

/**
 * @brief Declares a bitwise operator overload for an enum class
 *
 * @param cls     Enum class to define operator on
 * @param op      Bitwise operator
 */
#define ENUM_CLS_BITWISE_OPERATOR( cls, op )                                        \
  inline constexpr cls operator op( cls x, cls y )                                  \
  {                                                                                 \
    return static_cast<cls>( static_cast<size_t>( x ) | static_cast<size_t>( y ) ); \
  }

#endif /* !AURORA_UTIL_ENUM_HPP */
