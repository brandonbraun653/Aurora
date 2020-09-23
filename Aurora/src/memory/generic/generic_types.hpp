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
#include <cstring>
#include <limits>
#include <memory>
#include <vector>

namespace Aurora::Memory
{
  /*-------------------------------------------------------------------------------
  Forward Declarations
  -------------------------------------------------------------------------------*/
  class IGenericDevice;

  /*-------------------------------------------------------------------------------
  Aliases
  -------------------------------------------------------------------------------*/
  using IGenericDevice_sPtr = std::shared_ptr<IGenericDevice>;

  using ByteOffset_t = size_t;
  using SysAddress_t = size_t;
  using ChunkIndex_t = size_t;

  /*-------------------------------------------------------------------------------
  Constants
  -------------------------------------------------------------------------------*/
  static constexpr ByteOffset_t BAD_OFFSET    = 0xDEADBEEF;
  static constexpr SysAddress_t BAD_ADDRESS   = std::numeric_limits<SysAddress_t>::max();
  static constexpr ChunkIndex_t BAD_CHUNK_IDX = std::numeric_limits<ChunkIndex_t>::max();

  /*-------------------------------------------------------------------------------
  Enumerations
  -------------------------------------------------------------------------------*/
  /**
   *  Possible status codes that could be returned by a memory function
   */
  enum class Status : uint8_t
  {
    ERR_OK,
    ERR_OUT_OF_MEMORY,
    ERR_OVERRUN,
    ERR_UNALIGNED_MEM,
    ERR_UNKNOWN_JEDEC,
    ERR_HF_INIT_FAIL,
    ERR_NOT_PAGE_ALIGNED,
    ERR_READ_PROTECT,
    ERR_PGM_SEQUENCE,
    ERR_PGM_PARALLEL,
    ERR_PGM_ALIGNMENT,
    ERR_WRITE_PROTECT
  };

  /**
   *  Specifies a particular way to view/describe a section of memory
   */
  enum class Chunk : uint8_t
  {
    PAGE = 0,
    BLOCK,
    SECTOR
  };

  /**
   *  Possible events that could occur in a memory device
   */
  enum class Event : uint8_t
  {
    MEM_WRITE_COMPLETE,
    MEM_READ_COMPLETE,
    MEM_ERASE_COMPLETE,
    MEM_ERROR
  };

  /*-------------------------------------------------------------------------------
  Structures
  -------------------------------------------------------------------------------*/
  /**
   *  Several pieces of data that describes constants about a
   *  memory device at a high level.
   */
  struct Properties
  {
    size_t pageSize; /**< Page size of the device in bytes */
    size_t numPages; /**< Total number of pages */

    size_t blockSize; /**< Block size of the device in bytes */
    size_t numBlocks; /**< Total number of blocks */

    size_t sectorSize; /**< Sector size of the device in bytes */
    size_t numSectors; /**< Total number of sectors */

    size_t startAddress; /**< Starting address of the device region in memory */
    size_t endAddress;   /**< Ending address of the device region in memory */

    uint16_t jedec; /**< Device manufacturer's JEDEC code */

    Chunk writeChunk; /**< Min chunk size for writing */
    Chunk readChunk;  /**< Min chunk size for reading */
    Chunk eraseChunk; /**< Min chunk size for erasing */

    void clear()
    {
      memset( this, 0, sizeof( Properties ) );
    }
  };

}  // namespace Aurora::Memory

#endif  /* !AURORA_GENERIC_MEMORY_TYPES_HPP */
