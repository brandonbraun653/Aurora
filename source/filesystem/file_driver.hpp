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

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/source/memory/flash/eeprom/eeprom_generic_driver.hpp>
#include <Aurora/source/memory/flash/eeprom/eeprom_generic_types.hpp>
#include <Aurora/source/memory/flash/nor/nor_generic_driver.hpp>
#include <Aurora/source/memory/flash/nor/nor_generic_types.hpp>
#include <Chimera/spi>
#include <Chimera/i2c>
#include <cstdint>
#include <string_view>


namespace Aurora::FileSystem
{
  /*-------------------------------------------------------------------------------
  Project Side Interface
  -------------------------------------------------------------------------------*/
  namespace EEPROM
  {
    /**
     *  @brief Attaches a specific device to use for the filesystem backend
     *
     *  @param address    The address of the device to attach
     *  @param dev        The device to attach
     *  @param channel    Which I2C channel to communicate on
     *  @return bool
     */
    bool attachDevice( const uint16_t address, const Aurora::Flash::EEPROM::Chip_t dev, const Chimera::I2C::Channel channel );

    /**
     * @brief Gets the driver loaded for the current filesystem
     *
     * @return Aurora::Flash::EEPROM::Driver*
     */
    Aurora::Flash::EEPROM::Driver* getEEPROMDriver();

  }  // namespace EEPROM


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
