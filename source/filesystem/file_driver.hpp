/********************************************************************************
 *  File Name:
 *    file_driver.hpp
 *
 *  Description:
 *    Driver level interfaces for the filesystem
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_FILE_SYSTEM_DRIVER_INTERFACES_HPP
#define AURORA_FILE_SYSTEM_DRIVER_INTERFACES_HPP

/* STL Includes */
#include <cstdint>
#include <string_view>

/* Aurora Includes */
#include <Aurora/source/memory/flash/nor/nor_generic_driver.hpp>
#include <Aurora/source/memory/flash/nor/nor_generic_types.hpp>

/* Chimera Includes */
#include <Chimera/spi>


namespace Aurora::FileSystem
{
  /*-------------------------------------------------------------------------------
  Project Side Interface
  -------------------------------------------------------------------------------*/
  namespace LFS
  {
    /**
     *  Attaches file system control blocks and config data to
     *  be used by the Aurora FileSystem wrapper.
     *
     *  @note This is assuming a single FS is in use system wide
     *
     *  @param[in]  fs        The control block to attach   (lfs_t)
     *  @param[in]  cfg       Memory storage config options (lfs_config)
     *  @return bool
     */
    bool attachFS( void *const fs, const void *const cfg );

    /**
     *  Attaches a generic memory device to the opaque pointer contained in the
     *  LittleFS configuration structure. This allows the read/write/erase hooks
     *  to act on the proper device at runtime.
     *
     *  @param[in]  dev       The device to attach
     *  @param[in]  cfg       LittleFS configuration structure (lfs_config)
     *  @return bool
     */
    bool attachDevice( const Aurora::Flash::NOR::Chip_t dev, const Chimera::SPI::Channel channel, const void *const cfg );

    /**
     *  Erases the device completely
     *
     *  @param[in] timeout    How long to wait for the chip to erase
     *  @return bool
     */
     bool fullChipErase( const size_t timeout );

    /**
     *  Utility function to convert the given error code into a
     *  string variant specific to LittleFS.
     *
     *  @param[in]  error     Error code to convert
     *  @return std::string_view
     */
    std::string_view err2str( const int error );

  }  // namespace LFS


  namespace YAFFS
  {
    /**
     *  Attaches a generic memory device to the opaque pointer contained in the
     *  LittleFS configuration structure. This allows the read/write/erase hooks
     *  to act on the proper device at runtime.
     *
     *  @param[in]  dev       The device to attach
     *  @param[in]  cfg       LittleFS configuration structure
     *  @return bool
     */
    bool attachDevice( const Aurora::Flash::NOR::Chip_t dev, const Chimera::SPI::Channel channel );
  }  // namespace YAFFS


  namespace SPIFFS
  {
    /**
     *  Attaches a generic memory device to the opaque pointer contained in the
     *  LittleFS configuration structure. This allows the read/write/erase hooks
     *  to act on the proper device at runtime.
     *
     *  @param[in]  dev       The device to attach
     *  @param[in]  cfg       LittleFS configuration structure
     *  @return bool
     */
    bool attachDevice( const Aurora::Flash::NOR::Chip_t dev, const Chimera::SPI::Channel channel );

    Aurora::Flash::NOR::Driver *getDriver();
  }  // namespace SPIFFS

}  // namespace Aurora::FileSystem

#endif /* !AURORA_FILE_SYSTEM_DRIVER_INTERFACES_HPP */
