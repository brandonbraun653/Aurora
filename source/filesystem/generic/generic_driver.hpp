/********************************************************************************
 *  File Name:
 *    generic_driver.hpp
 *
 *  Description:
 *    Generic OS interface definition
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_FILESYSTEM_GENERIC_OS_HPP
#define AURORA_FILESYSTEM_GENERIC_OS_HPP

/* STL Includes */
#include <cstdint>

/* Aurora Includes */
#include <Aurora/source/filesystem/file_types.hpp>

namespace Aurora::FileSystem::Generic
{
  /*-------------------------------------------------------------------------------
  Public Data
  -------------------------------------------------------------------------------*/
  extern const Interface implementation;

}  // namespace Aurora::FileSystem::Generic

#endif  /* !AURORA_FILESYSTEM_GENERIC_OS_HPP */
