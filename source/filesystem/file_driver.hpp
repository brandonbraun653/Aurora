/********************************************************************************
 *  File Name:
 *    file_driver.hpp
 *
 *  Description:
 *    Driver level interfaces for the filesystem
 *
 *  2021-2022 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_FILE_SYSTEM_DRIVER_INTERFACES_HPP
#define AURORA_FILE_SYSTEM_DRIVER_INTERFACES_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/source/memory/flash/eeprom/eeprom_generic_driver.hpp>
#include <Aurora/source/memory/flash/eeprom/eeprom_generic_types.hpp>
#include <Aurora/source/memory/flash/nor/nor_generic_driver.hpp>
#include <Aurora/source/memory/flash/nor/nor_generic_types.hpp>
#include <Chimera/i2c>
#include <Chimera/spi>
#include <cstdint>
#include <string_view>


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
     *  @param drive    Which drive to attach the filesystem to
     *  @param fs        The control block to attach   (lfs_t)
     *  @param cfg       Memory storage config options (lfs_config)
     *  @return bool
     */
    bool attachFS( const std::string_view &drive, void *const fs, const void *const cfg );

  }  // namespace LFS


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

    /**
     * @brief Gets the NOR driver that backs the SPIFF implementation
     *
     * @return Aurora::Flash::NOR::Driver*
     */
    Aurora::Flash::NOR::Driver *getNORDriver();

  }  // namespace SPIFFS

}  // namespace Aurora::FileSystem

#endif /* !AURORA_FILE_SYSTEM_DRIVER_INTERFACES_HPP */

