/********************************************************************************
 *  File Name:
 *    file_intf.hpp
 *
 *  Description:
 *    High level file system wrapper for consistent project interfacing
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_FILESYSTEM_INTERFACE_HPP
#define AURORA_FILESYSTEM_INTERFACE_HPP

/* STL Includes */
#include <cstdint>
#include <cstddef>

/* Aurora Includes */
#include <Aurora/source/filesystem/file_types.hpp>

namespace Aurora::FileSystem
{
  /*-------------------------------------------------------------------------------
  Public Functions:
  All of these interfaces must be defined in some back end interface driver.
  -------------------------------------------------------------------------------*/
  /**
   *  Configures the file system to use the appropriate driver. This
   *  assumes that any driver specific data has already been passed
   *  to it and the system is in a state that is ready for initialization.
   *
   *  @param[in]  type        Which filesystem driver to use
   *  @return bool
   */
  bool configureDriver( const BackendType type );

  /**
   *  Mounts the file system
   *  @return int
   */
  int mount();

  /**
   *  Un-mounts the file system
   *  @return int
   */
  int unmount();
}  // namespace Aurora::FileSystem

#endif  /* !AURORA_FILESYSTEM_INTERFACE_HPP */
