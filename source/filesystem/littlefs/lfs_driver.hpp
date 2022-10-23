/********************************************************************************
 *  File Name:
 *    lfs_hooks.hpp
 *
 *  Description:
 *    Hooks for integration of a generic memory device with LittleFS
 *
 *  2020-2022 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef LFS_HOOKS_HPP
#define LFS_HOOKS_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/source/filesystem/file_types.hpp>
#include "lfs.h"

namespace Aurora::FileSystem::LFS
{
  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/
  /**
   * @brief Get the implementation of the LittleFS filesystem
   * @return Interface
   */
  Interface getInterface();
}

#endif  /* !LFS_HOOKS_HPP */
