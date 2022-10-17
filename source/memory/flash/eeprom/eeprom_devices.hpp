/******************************************************************************
 *  File Name:
 *    eeprom_devices.hpp
 *
 *  Description:
 *    Device descriptors for EEPROM chips
 *
 *  2022 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef AURORA_EEPROM_DEVICE_DESCRIPTORS_HPP
#define AURORA_EEPROM_DEVICE_DESCRIPTORS_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/source/memory/generic/generic_types.hpp>
#include <Aurora/source/memory/flash/eeprom/eeprom_generic_types.hpp>

namespace Aurora::Flash::EEPROM
{
  /*---------------------------------------------------------------------------
  Public Data
  ---------------------------------------------------------------------------*/
  static constexpr const Aurora::Memory::Properties ChipProperties[ Chip::EEPROM_CHIP_OPTIONS ] = {
    // AT24C02
    { .writeChunk      = Aurora::Memory::Chunk::PAGE,
      .readChunk       = Aurora::Memory::Chunk::PAGE,
      .eraseChunk      = Aurora::Memory::Chunk::PAGE,
      .jedec           = 0,
      .pageSize        = 1,
      .blockSize       = 0,
      .sectorSize      = 0,
      .startAddress    = 0,
      .endAddress      = 256,
      .startUpDelay    = 0,
      .pagePgmDelay    = 5 * Chimera::Thread::TIMEOUT_1MS,
      .blockEraseDelay = 0,
      .chipEraseDelay  = 0,
      .eventPoll       = nullptr },
    // 24LC128
    { .writeChunk      = Aurora::Memory::Chunk::PAGE,
      .readChunk       = Aurora::Memory::Chunk::PAGE,
      .eraseChunk      = Aurora::Memory::Chunk::PAGE,
      .jedec           = 0,
      .pageSize        = 1,
      .blockSize       = 0,
      .sectorSize      = 0,
      .startAddress    = 0,
      .endAddress      = 16 * 1024,
      .startUpDelay    = 0,
      .pagePgmDelay    = 5 * Chimera::Thread::TIMEOUT_1MS,
      .blockEraseDelay = 0,
      .chipEraseDelay  = 0,
      .eventPoll       = nullptr },
  };
}  // namespace Aurora::Flash::EEPROM

#endif  /* !AURORA_EEPROM_DEVICE_DESCRIPTORS_HPP */
