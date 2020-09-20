/********************************************************************************
 *  File Name:
 *    generic_types.hpp
 *
 *  Description:
 *    Types associated with generic memory interfaces
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_GENERIC_MEMORY_TYPES_HPP
#define AURORA_GENERIC_MEMORY_TYPES_HPP

/* STL Includes */
#include <cstddef>
#include <cstdint>
#include <vector>

namespace Aurora::Memory
{
  /*-------------------------------------------------------------------------------
  Enumerations
  -------------------------------------------------------------------------------*/
  enum class Status : uint8_t
  {
    OUT_OF_MEMORY,
    OVERRUN,
    UNALIGNED_MEM,
    UNKNOWN_JEDEC,
    HF_INIT_FAIL,
    NOT_PAGE_ALIGNED,
    ERR_READ_PROTECT,
    ERR_PGM_SEQUENCE,
    ERR_PGM_PARALLEL,
    ERR_PGM_ALIGNMENT,
    ERR_WRITE_PROTECT
  };

  enum class Section : uint8_t
  {
    PAGE = 0,
    BLOCK,
    SECTOR
  };

  /*-------------------------------------------------------------------------------
  Structures
  -------------------------------------------------------------------------------*/
  struct SectionList
  {
    std::vector<size_t> pages;
    std::vector<size_t> blocks;
    std::vector<size_t> sectors;
  };

  struct Descriptor
  {
    size_t pageSize     = 0; /**< Page size of the device in bytes */
    size_t blockSize    = 0; /**< Block size of the device in bytes */
    size_t sectorSize   = 0; /**< Sector size of the device in bytes */
    size_t startAddress = 0; /**< Starting address of the device region in memory */
    size_t endAddress   = 0; /**< Ending address of the device region in memory */
  };

}  // namespace Aurora::Memory

#endif  /* !AURORA_GENERIC_MEMORY_TYPES_HPP */
