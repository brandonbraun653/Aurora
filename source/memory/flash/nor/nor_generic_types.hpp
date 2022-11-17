/********************************************************************************
 *  File Name:
 *    nor_generic_types.hpp
 *
 *  Description:
 *    Types and definitions for the NOR flash driver
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef NOR_FLASH_GENERIC_DRIVER_TYPES_HPP
#define NOR_FLASH_GENERIC_DRIVER_TYPES_HPP

/* STL Includes */
#include <cstddef>

namespace Aurora::Memory::Flash::NOR
{
  /*-------------------------------------------------------------------------------
  Aliases
  -------------------------------------------------------------------------------*/
  using Jedec_t   = uint8_t;
  using Command_t = uint8_t;


  /*-------------------------------------------------------------------------------
  Constants
  -------------------------------------------------------------------------------*/
  /*-------------------------------------------------
  Addressing Constants
  -------------------------------------------------*/
  static constexpr size_t ADDRESS_BYTE_1_POS = 0;
  static constexpr size_t ADDRESS_BYTE_1_MSK = 0x000000FF;

  static constexpr size_t ADDRESS_BYTE_2_POS = 8;
  static constexpr size_t ADDRESS_BYTE_2_MSK = 0x0000FF00;

  static constexpr size_t ADDRESS_BYTE_3_POS = 16;
  static constexpr size_t ADDRESS_BYTE_3_MSK = 0x00FF0000;

  /*-------------------------------------------------
  Manufacturer & Device ID Bit Masks
  -------------------------------------------------*/
  static constexpr uint8_t MFR_MSK = 0xFF;

  static constexpr uint8_t FAMILY_CODE_POS = 5;
  static constexpr uint8_t FAMILY_CODE_MSK = 0x07;

  static constexpr uint8_t DENSITY_CODE_POS = 0;
  static constexpr uint8_t DENSITY_CODE_MSK = 0x1F;

  static constexpr uint8_t SUB_CODE_POS = 5;
  static constexpr uint8_t SUB_CODE_MSK = 0x07;

  static constexpr uint8_t PROD_VERSION_POS = 0;
  static constexpr uint8_t PROD_VERSION_MSK = 0x1F;

  /*-------------------------------------------------
  Common Block Sizes
  -------------------------------------------------*/
  static constexpr size_t CHUNK_SIZE_256 = 256;
  static constexpr size_t CHUNK_SIZE_4K  = 4 * 1024;
  static constexpr size_t CHUNK_SIZE_32K = 32 * 1024;
  static constexpr size_t CHUNK_SIZE_64K = 64 * 1024;

  /*-------------------------------------------------------------------------------
  Enumerations
  -------------------------------------------------------------------------------*/
  /**
   *  Lists the supported NOR flash memory chips
   */
  enum Chip : uint8_t
  {
    ADESTO_START,
    AT25SF081 = ADESTO_START,
    ADESTO_END,

    NUM_OPTIONS,
    UNKNOWN
  };
  using Chip_t = uint8_t;

  /*-------------------------------------------------------------------------------
  Structures
  -------------------------------------------------------------------------------*/

}  // namespace Aurora::Memory::Flash::NOR

#endif /* !NOR_FLASH_GENERIC_DRIVER_TYPES_HPP */
