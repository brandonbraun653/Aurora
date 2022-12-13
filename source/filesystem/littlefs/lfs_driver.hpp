/******************************************************************************
 *  File Name:
 *    lfs_hooks.hpp
 *
 *  Description:
 *    Hooks for integration of a generic memory device with LittleFS
 *
 *  2020-2022 | Brandon Braun | brandonbraun653@gmail.com
 *****************************************************************************/

#pragma once
#ifndef LFS_HOOKS_HPP
#define LFS_HOOKS_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include "lfs.h"
#include <Aurora/source/filesystem/file_types.hpp>
#include <Aurora/source/memory/flash/nor/nor_generic_driver.hpp>
#include <Chimera/spi>
#include <Chimera/thread>
#include <etl/string.h>

namespace Aurora::FileSystem::LFS
{
  /*---------------------------------------------------------------------------
  Structures
  ---------------------------------------------------------------------------*/
  struct Volume
  {
    lfs_t                           fs;        /**< Core memory to manage a full filesystem */
    lfs_config                      cfg;       /**< Configuration of the filesystem interface */
    Aurora::Memory::Flash::NOR::Driver      flash;     /**< Flash memory driver */
    VolumeId                        _volumeID; /**< Mapped volume ID */
    Chimera::Thread::RecursiveMutex _lock;     /**< Multi-threaded access protection */

#if defined( SIMULATOR )
    std::filesystem::path _dataFile; /**< Backing file for a fake NOR chip */
#endif

    void clear()
    {
      memset( &fs, 0, sizeof( fs ) );
      memset( &cfg, 0, sizeof( cfg ) );
      _volumeID = -1;
      _lock.unlock();
    }
  };

  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/
  /**
   * @brief Initializes LFS specific driver data
   */
  void initialize();

  /**
   * @brief Get the implementation of the LittleFS filesystem
   *
   * @param vol     The volume associated with the interface
   * @return Interface
   */
  Interface getInterface( Volume *const vol );

  /**
   * @brief Registers a volume for use with the filesystem
   * @warning The memory associated with this volume must always exist!
   *
   * @param vol     Which volume to register
   * @return bool
   */
  bool attachVolume( Volume *const vol );

  /**
   * @brief Reformats the given volume
   *
   * @param vol     The volume to format
   * @return bool
   */
  bool formatVolume( Volume *const vol );
}

#endif  /* !LFS_HOOKS_HPP */
