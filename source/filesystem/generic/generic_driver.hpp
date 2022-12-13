/******************************************************************************
 *  File Name:
 *    generic_driver.hpp
 *
 *  Description:
 *    Generic OS interface definition
 *
 *  2021-2022 | Brandon Braun | brandonbraun653@gmail.com
 *****************************************************************************/

#pragma once
#ifndef AURORA_FILESYSTEM_GENERIC_OS_HPP
#define AURORA_FILESYSTEM_GENERIC_OS_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <cstdint>
#include <Aurora/source/filesystem/file_types.hpp>

namespace Aurora::FileSystem::Generic
{
  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/
  /**
   * @brief Get the implementation of the generic filesystem
   * @return Interface
   */
  Interface getInterface();

}  // namespace Aurora::FileSystem::Generic

#endif  /* !AURORA_FILESYSTEM_GENERIC_OS_HPP */
