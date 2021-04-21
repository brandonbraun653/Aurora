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


#endif /* !AURORA_UTIL_ENUM_HPP */
