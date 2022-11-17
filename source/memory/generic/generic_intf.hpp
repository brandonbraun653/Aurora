/********************************************************************************
 *  File Name:
 *    generic_intf.hpp
 *
 *  Description:
 *    Tools useful when interfacing with a device that has controllable memory
 *
 *  2019-2022 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#pragma once
#ifndef AURORA_GENERIC_MEMORY_INTF_HPP
#define AURORA_GENERIC_MEMORY_INTF_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/source/memory/generic/generic_types.hpp>
#include <cstddef>

namespace Aurora::Memory
{
  /*---------------------------------------------------------------------------
  Structures
  ---------------------------------------------------------------------------*/
  /**
   * @brief Specifies simple access attributes about the device
   */
  struct DeviceAttr
  {
    size_t readSize;  /**< Chunk size used to read */
    size_t writeSize; /**< Chunk size used to write */
    size_t eraseSize; /**< Chunk size used to erase */
  };

  /*---------------------------------------------------------------------------
  Classes
  ---------------------------------------------------------------------------*/
  /**
   *  1. Models a memory device in a generic way that does not require the
   *     user to know particular details about the device's internal structure.
   *     Memory can be accessed with the assumption that data alignment will be
   *     taken care of behind the scenes where possible. For best performance
   *     however, it would we wise to use aligned transactions that best fit
   *     the underlying device hardware.
   *
   *  2. The device exposes a lock interface to support thread safe access.
   *
   *  3. Where possible, the accesses are considered atomic and will execute
   *     the appropriate event handler when an operation is complete or an
   *     asynchronous event occurs.
   */
  class IGenericDevice
  {
  public:
    virtual ~IGenericDevice() = default;

    /**
     *  Initializes the device for access
     *  @return Status
     */
    virtual Aurora::Memory::Status open( const DeviceAttr *const attributes ) = 0;

    /**
     *  Tears down the device so no one can access it further
     *  @return Status
     */
    virtual Aurora::Memory::Status close() = 0;

    /**
     *  Writes data into the given chunk
     *
     *  @param chunk         The chunk id to write data into
     *  @param offset        Byte offset into the chunk
     *  @param data          The buffer of data that will be written
     *  @param length        Number of bytes to be written
     *  @return Status
     */
    virtual Aurora::Memory::Status write( const size_t chunk, const size_t offset, const void *const data, const size_t length ) = 0;

    /**
     * @brief Writes data at an absolute address
     *
     * @param address       Address to start writing at
     * @param data          The buffer of data that will be written
     * @param length        Number of bytes to be written
     * @return Status
     */
    virtual Aurora::Memory::Status write( const size_t address, const void *const data, const size_t length ) = 0;

    /**
     *  Reads a contiguous length of memory starting at the given chunk.
     *
     *  @param chunk         The chunk id to start the read from
     *  @param offset        Byte offset into the chunk
     *  @param data          Buffer of data to read into
     *  @param length        How many bytes to read out
     *  @return Status
     */
    virtual Aurora::Memory::Status read( const size_t chunk, const size_t offset, void *const data, const size_t length ) = 0;

    /**
     * @brief Reads a contiguous length of memory from an absolute address
     *
     * @param address     Address to begin reading from
     * @param data        Buffer of data to read into
     * @param length      How many bytes to read out
     * @return Status
     */
    virtual Aurora::Memory::Status read( const size_t address, void *const data, const size_t length ) = 0;

    /**
     *  Erase a block of memory that corresponds with the device's erase block size
     *
     *  @param block         The block id to erase
     *  @return Status
     */
    virtual Aurora::Memory::Status erase( const size_t block ) = 0;

    /**
     * @brief Erases a section of memory
     *
     * @param address     Address to start erasing at
     * @param length      Number of bytes to erase
     * @return Status
     */
    virtual Aurora::Memory::Status erase( const size_t address, const size_t length ) = 0;

    /**
     *  Erases the entire chip. Typically this is a single command, so it saves on
     *  manual address calculations.
     *
     *  @return Status
     */
    virtual Aurora::Memory::Status erase() = 0;

    /**
     *  Flushes any buffered memory to the device
     *
     *  @return Status
     */
    virtual Aurora::Memory::Status flush() = 0;

    /**
     *  Blocks the current thread of execution until a memory event has
     *  happened. Typical implementations poll status registers of the
     *  device to see if an event has happened.
     *
     *  @param event         The event to wait on
     *  @param timeout       How long the caller is willing to wait
     *  @return Status
     */
    virtual Aurora::Memory::Status pendEvent( const Aurora::Memory::Event event, const size_t timeout ) = 0;
  };

}  // namespace Aurora::Memory

#endif /* !AURORA_GENERIC_MEMORY_INTF_HPP */
