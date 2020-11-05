/********************************************************************************
 *  File Name:
 *    utility.hpp
 *
 *  Description:
 *    Common utilities useful for various applications
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_UTILITY_HPP
#define AURORA_UTILITY_HPP

/* STL Includes */
#include <cstddef>

namespace Aurora::Utility
{
  /**
   *  Inserts a breakpoint into the software. Will need to go up the
   *  call stack in order to see where this came from.
   *
   *  @return void
   */
  void insertBreakpoint();

}  // namespace Aurora::Utility

#endif  /* !AURORA_UTILITY_HPP */
