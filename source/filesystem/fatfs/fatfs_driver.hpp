/******************************************************************************
 *  File Name:
 *    fatfs_driver.hpp
 *
 *  Description:
 *    Hooks for integrating FatFS with Aurora
 *
 *  2023 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef FATFS_AURORA_FILESYSTEM_DRIVER_HPP
#define FATFS_AURORA_FILESYSTEM_DRIVER_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <ff.h>

#include <Chimera/thread>
#include <Aurora/source/filesystem/file_types.hpp>
#include <Aurora/source/memory/generic/generic_intf.hpp>


namespace Aurora::FileSystem::FatFs
{
  /*---------------------------------------------------------------------------
  Structures
  ---------------------------------------------------------------------------*/

  /**
   * @brief Details a unique volume that can be mounted
   */
  struct Volume
  {
    FATFS                           fs;       /**< Core memory to manage a full filesystem */
    Aurora::Memory::IGenericDevice *device;   /**< Memory device to use for storage */
    VolumeId                        volumeID; /**< Mapped volume ID */
    Chimera::Thread::RecursiveMutex lock;     /**< Multi-threaded access protection */
  };


  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/

  /**
   * @brief Initializes FatFS specific driver data
   */
  void initialize();

  /**
   * @brief Get the implementation of the FatFS filesystem
   * @return Interface&
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
}  // namespace Aurora::FileSystem::FatFs

#endif /* !FATFS_AURORA_FILESYSTEM_DRIVER_HPP */
