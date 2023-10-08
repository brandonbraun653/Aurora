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
#include <diskio.h>

#include <Chimera/thread>
#include <Aurora/source/filesystem/file_types.hpp>
#include <Aurora/source/memory/generic/generic_intf.hpp>
#include <etl/string.h>


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
    FATFS                           fs;                /**< Core memory to manage a full filesystem */
    DSTATUS                         status;            /**< Status of the volume */
    size_t                          mount_retry_delay; /**< Retry delay when mounting fails */
    etl::string<32>                 path;              /**< Volume mounting path, one of FF_VOLUME_STRS */
    Aurora::Memory::IGenericDevice *device;            /**< Memory device to use for storage */
    VolumeId                        volumeID;          /**< Mapped volume ID */
    Chimera::Thread::RecursiveMutex lock;              /**< Multi-threaded access protection */

    Volume()
    {
      reset();
    }

    /**
     * @brief Reset the volume data to a known state
     */
    void reset()
    {
      path.clear();
      memset( &fs, 0, sizeof( fs ) );
      status            = STA_NOINIT;
      mount_retry_delay = 75;
      device            = nullptr;
      volumeID          = -1;
    }
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
