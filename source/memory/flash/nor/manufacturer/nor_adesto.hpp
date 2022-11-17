/********************************************************************************
 *  File Name:
 *    nor_adesto.hpp
 *
 *  Description:
 *    Memory descriptions for the Adesto manufacturer
 *
 *  2021-2022 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef NOR_FLASH_ADESTO_HPP
#define NOR_FLASH_ADESTO_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/source/memory/flash/nor/nor_generic_types.hpp>
#include <Aurora/source/memory/generic/generic_types.hpp>
#include <cstdint>

namespace Aurora::Memory::Flash::NOR::Adesto
{
  /*-------------------------------------------------------------------------------
  Public Data
  -------------------------------------------------------------------------------*/
  extern const Aurora::Memory::Properties ChipProperties[ static_cast<size_t>( Chip::ADESTO_END - Chip::ADESTO_START ) ];

  /*-------------------------------------------------------------------------------
  Constants
  -------------------------------------------------------------------------------*/
  static constexpr Jedec_t JEDEC_CODE = 0x1F;

  /*-------------------------------------------------------------------------------
  Enumerations
  -------------------------------------------------------------------------------*/
  enum FamilyCode : uint8_t
  {
    AT45Dxxx  = 0x01,
    AT25SFxxx = 0x04,
  };

  enum DensityCode : uint8_t
  {
    DENSITY_2MBIT  = 0x03,
    DENSITY_4MBIT  = 0x04,
    DENSITY_8MBIT  = 0x05,
    DENSITY_16MBIT = 0x06,
    DENSITY_32MBIT = 0x07,
    DENSITY_64MBIT = 0x08
  };

  enum SubCode : uint8_t
  {
    STANDARD_SERIES = 0x00,
    // Add more as needed
  };

  enum ProductVariant : uint8_t
  {
    DEFAULT  = 0x00,
    VERSION1 = 0x01,
    // Add more as needed
  };

  /*-------------------------------------------------------------------------------
  Public Functions
  -------------------------------------------------------------------------------*/
  /**
   *  Adesto specific polling for an Read/Write/Erase event flag
   *  @see EventPollFunc
   */
  Aurora::Memory::Status pollEvent( void *driver, const uint8_t device, const Aurora::Memory::Event event, const size_t timeout );

}  // namespace Aurora::Memory::Flash::NOR::Adesto

#endif /* !NOR_FLASH_ADESTO_HPP */
