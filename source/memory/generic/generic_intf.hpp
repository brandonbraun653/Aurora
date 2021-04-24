/********************************************************************************
 *  File Name:
 *    generic_intf.hpp
 *
 *  Description:
 *    Tools useful when interfacing with a device that has controllable memory
 *
 *  2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#pragma once
#ifndef AURORA_GENERIC_MEMORY_INTF_HPP
#define AURORA_GENERIC_MEMORY_INTF_HPP

/* C++ Includes */
#include <limits>
#include <vector>

/* Aurora Includes */
#include <Aurora/source/memory/generic/generic_types.hpp>

/* Chimera Includes */
#include <Chimera/thread>

namespace Aurora::Memory
{
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
    virtual Status open() = 0;

    /**
     *  Tears down the device so no one can access it further
     *  @return Status
     */
    virtual Status close() = 0;

    /**
     *  Writes data into the given chunk
     *
     *  @param[in]  chunk         The chunk id to write data into
     *  @param[in]  offset        Byte offset into the chunk
     *  @param[in]  data          The buffer of data that will be written
     *  @param[in]  length        Number of bytes to be written
     *  @return Status
     */
    virtual Status write( const size_t chunk, const size_t offset, const void *const data, const size_t length ) = 0;

    /**
     * @brief Writes data at an absolute address
     *
     * @param address       Address to start writing at
     * @param data          The buffer of data that will be written
     * @param length        Number of bytes to be written
     * @return Status
     */
    virtual Status write( const size_t address, const void *const data, const size_t length ) = 0;

    /**
     *  Reads a contiguous length of memory starting at the given chunk.
     *
     *  @param[in]  chunk         The chunk id to start the read from
     *  @param[in]  offset        Byte offset into the chunk
     *  @param[out] data          Buffer of data to read into
     *  @param[in]  length        How many bytes to read out
     *  @return Status
     */
    virtual Status read( const size_t chunk, const size_t offset, void *const data, const size_t length ) = 0;

    /**
     * @brief Reads a contiguous length of memory from an absolute address
     *
     * @param address     Address to begin reading from
     * @param data        Buffer of data to read into
     * @param length      How many bytes to read out
     * @return Status
     */
    virtual Status read( const size_t address, void *const data, const size_t length ) = 0;

    /**
     *  Erase a block of memory that corresponds with the device's erase block size
     *
     *  @param[in]  block         The block id to erase
     *  @return Status
     */
    virtual Status erase( const size_t block ) = 0;

    /**
     * @brief Erases a section of memory
     *
     * @param address     Address to start erasing at
     * @param length      Number of bytes to erase
     * @return Status
     */
    virtual Status erase( const size_t address, const size_t length ) = 0;

    /**
     *  Erases the entire chip. Typically this is a single command, so it saves on
     *  manual address calculations.
     *
     *  @return Status
     */
    virtual Status erase() = 0;

    /**
     *  Flushes any buffered memory to the device
     *
     *  @return Status
     */
    virtual Status flush() = 0;

    /**
     *  Blocks the current thread of execution until a memory event has
     *  happened. Typical implementations poll status registers of the
     *  device to see if an event has happened.
     *
     *  @param[in]  event         The event to wait on
     *  @param[in]  timeout       How long the caller is willing to wait
     *  @return Status
     */
    virtual Status pendEvent( const Event event, const size_t timeout ) = 0;
  };

}  // namespace Aurora::Memory

#endif /* !AURORA_GENERIC_MEMORY_INTF_HPP */
