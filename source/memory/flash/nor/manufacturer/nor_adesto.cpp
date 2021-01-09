/********************************************************************************
 *  File Name:
 *    nor_adesto.cpp
 *
 *  Description:
 *    Adesto description information
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* STL Includes */
#include <cstdint>

/* Aurora Includes */
#include <Aurora/source/memory/flash/nor/manufacturer/nor_adesto.hpp>

namespace Aurora::Flash::NOR::Adesto
{
  /*-------------------------------------------------------------------------------
  Public Data
  -------------------------------------------------------------------------------*/
  /**
   *  Device descriptors for Adesto memory chips. Must match the order of devices
   *  found in the Chip enum.
   */
  const Aurora::Memory::Properties ChipProperties[ static_cast<size_t>( Chip::ADESTO_END - Chip::ADESTO_START ) ] = {
    // AT25SF081
    { .writeChunk   = Aurora::Memory::Chunk::PAGE,
      .readChunk    = Aurora::Memory::Chunk::PAGE,
      .eraseChunk   = Aurora::Memory::Chunk::SECTOR,
      .jedec        = JEDEC_CODE,
      .pageSize     = 256,
      .blockSize    = 4 * 1024,
      .sectorSize   = 32 * 1024,
      .startAddress = 0,
      .endAddress   = 8 * 1024 * 1024 }
  };

}  // namespace Aurora::Flash::NOR::Adesto
