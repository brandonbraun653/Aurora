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

#include <Aurora/source/util/enum.hpp>

namespace Aurora::Memory
{
  /*-------------------------------------------------------------------------------
  Forward Declarations
  -------------------------------------------------------------------------------*/
  class IGenericDevice;


  /*-------------------------------------------------------------------------------
  Enumerations
  -------------------------------------------------------------------------------*/
  /**
   *  Possible status codes that could be returned by a memory function
   */
  enum class Status : uint8_t
  {
    ERR_BAD_ARG,
    ERR_DRIVER_ERR,
    ERR_HF_INIT_FAIL,
    ERR_NOT_PAGE_ALIGNED,
    ERR_OK,
    ERR_FAIL,
    ERR_OUT_OF_MEMORY,
    ERR_OVERRUN,
    ERR_PGM_ALIGNMENT,
    ERR_PGM_PARALLEL,
    ERR_PGM_SEQUENCE,
    ERR_READ_PROTECT,
    ERR_TIMEOUT,
    ERR_UNALIGNED_MEM,
    ERR_UNKNOWN_JEDEC,
    ERR_UNSUPPORTED,
    ERR_WRITE_PROTECT,
  };
  ENUM_CLS_BITWISE_OPERATOR( Status, | );

  /**
   *  Specifies a particular way to view/describe a section of memory
   */
  enum class Chunk : uint8_t
  {
    PAGE = 0,
    BLOCK,
    SECTOR,
    NONE
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
  Aliases
  -------------------------------------------------------------------------------*/
  using IGenericDevice_sPtr = std::shared_ptr<IGenericDevice>;

  using ByteOffset_t = size_t;
  using SysAddress_t = size_t;
  using ChunkIndex_t = size_t;

  /**
   *  Manufacturer specific polling for an Read/Write/Erase event flag. There isn't
   *  really a standard for checking the status of an operation (that the author is
   *  aware of), so this function will serve as a redirect into that functionality.
   *
   *  @param[in]  channel     Peripheral channel in use for the device (SPI, I2C, MMC, etc)
   *  @param[in]  device      Specific device identifier. This can influence the read protocol.
   *  @param[in]  event       Which event to look for
   *  @param[in]  timeout     How long in milliseconds to wait for the event to occur
   *  @return Status
   */
  using EventPollFunc = Status ( * )( const uint8_t channel, const uint8_t device, const Event event, const size_t timeout );

  /*-------------------------------------------------------------------------------
  Constants
  -------------------------------------------------------------------------------*/
  static constexpr ByteOffset_t BAD_OFFSET    = 0xDEADBEEF;
  static constexpr SysAddress_t BAD_ADDRESS   = std::numeric_limits<SysAddress_t>::max();
  static constexpr ChunkIndex_t BAD_CHUNK_IDX = std::numeric_limits<ChunkIndex_t>::max();


  /*-------------------------------------------------------------------------------
  Structures
  -------------------------------------------------------------------------------*/
  /**
   *  Several pieces of data that describes constants about a
   *  memory device at a high level.
   */
  struct Properties
  {
    /*-------------------------------------------------
    Data Fields
    -------------------------------------------------*/
    Chunk writeChunk;      /**< Desired unit size for writing */
    Chunk readChunk;       /**< Desired unit size for reading */
    Chunk eraseChunk;      /**< Desired unit size for erasing */
    uint8_t jedec;         /**< Device manufacturer's JEDEC code */
    uint16_t pageSize;     /**< Page size of the device in bytes */
    uint16_t blockSize;    /**< Block size of the device in bytes */
    uint16_t sectorSize;   /**< Sector size of the device in bytes */
    uint32_t startAddress; /**< Starting address of the device region in memory */
    uint32_t endAddress;   /**< Ending address of the device region in memory */

    size_t startUpDelay;    /**< How long the device needs to stabilize on power up*/
    size_t pagePgmDelay;    /**< Worst case page program delay */
    size_t blockEraseDelay; /**< Worst case block erase delay (sized for eraseChunk) */
    size_t chipEraseDelay;  /**< Worst case to erase whole chip */

    /*-------------------------------------------------
    Function Interface
    -------------------------------------------------*/
    EventPollFunc eventPoll; /**< Driver specific status polling interface */
  };

}  // namespace Aurora::Memory

#endif /* !AURORA_GENERIC_MEMORY_TYPES_HPP */
