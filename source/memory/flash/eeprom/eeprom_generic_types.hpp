/******************************************************************************
 *  File Name:
 *    eeprom_generic_types.hpp
 *
 *  Description:
 *    Types and definitions for the EEPROM flash driver
 *
 *  2022 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef EEPROM_FLASH_GENERIC_DRIVER_TYPES_HPP
#define EEPROM_FLASH_GENERIC_DRIVER_TYPES_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <cstddef>
#include <Chimera/i2c>

namespace Aurora::Flash::EEPROM
{
  /*---------------------------------------------------------------------------
  Aliases
  ---------------------------------------------------------------------------*/
  using Chip_t = uint8_t;

  /*---------------------------------------------------------------------------
  Enumerations
  ---------------------------------------------------------------------------*/
  /**
   * @brief Enumerates a supported chip
   */
  enum Chip : uint8_t
  {
    EEPROM_CHIP_START,

    AT24C02 = EEPROM_CHIP_START,

    EEPROM_CHIP_END,
    EEPROM_CHIP_OPTIONS = EEPROM_CHIP_END,
    EEPROM_CHIP_UNKNOWN
  };

  /*---------------------------------------------------------------------------
  Structures
  ---------------------------------------------------------------------------*/
  struct DeviceConfig
  {
    Chip_t                whichChip;     /**< Which chip this is */
    uint16_t              deviceAddress; /**< Address of the chip */
    Chimera::I2C::Channel i2cChannel;    /**< I2C channel the chip is on */

    void clear()
    {
      whichChip     = EEPROM_CHIP_UNKNOWN;
      deviceAddress = 0;
      i2cChannel    = Chimera::I2C::Channel::NOT_SUPPORTED;
    }
  };
}  // namespace Aurora::Flash::EEPROM

#endif /* !EEPROM_FLASH_GENERIC_DRIVER_TYPES_HPP */
