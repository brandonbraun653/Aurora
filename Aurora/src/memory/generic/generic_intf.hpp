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
#include <Aurora/src/memory/generic/generic_types.hpp>

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
  class IGenericDevice : public virtual Chimera::Threading::LockableInterface
  {
  public:
    virtual ~IGenericDevice() = default;

    /**
     *  Initializes the device for access
     *
     *  @return Status
     */
    virtual Status open() = 0;

    /**
     *  Tears down the device so no one can access it further
     *
     *  @return Status
     */
    virtual Status close() = 0;

    /**
     *	Writes data into flash memory over an arbitrary address range.
     *
     *  @note   Does not require page alignment, but be aware of the possibility that unaligned
     *          data could possibly take longer to write. This is dependent upon the specific device.
     *
     *	@param[in]	address       The start address to write data into
     *	@param[in]	data          The buffer of data that will be written
     *	@param[in]	length        Number of bytes to be written
     *	@return Status
     *
     *  |  Return Value |                             Explanation                            |
     *  |:-------------:|:------------------------------------------------------------------:|
     *  |            OK | The write completed successfully                                   |
     *  |          FAIL | The write did not succeed for some reason (device specific)        |
     *  |          BUSY | Flash is doing something at the moment. Try again later.           |
     *  | OUT_OF_MEMORY | Zero or more bytes were written, but not the full amount requested |
     */
    virtual Status write( const size_t address, const void *const data, const size_t length ) = 0;

    /**
     *  Reads data in a contiguous block, starting from the given address. Should *not* be able to
     *  read across the end of the device memory and wrap around to the beginning.
     *
     *	@param[in]	address       The address to start the read from
     *	@param[out]	data          Buffer of data to read into
     *	@param[in]	length        How many bytes to read out
     *	@return Status
     *
     *  | Return Value |                         Explanation                         |
     *  |:------------:|:-----------------------------------------------------------:|
     *  |           OK | The read completed successfully                             |
     *  |         FAIL | The read did not succeed for some reason (device specific)  |
     *  |         BUSY | Flash is doing something at the moment. Try again later.    |
     *  |      OVERRUN | A boundary was reached and the read halted.                 |
     */
    virtual Status read( const size_t address, void *const data, const size_t length ) = 0;

    /**
     *  Erase a region of memory. The given address range will need to be page, block, or
     *  sector aligned, depending upon what the underlying system requires. Because this is
     *  an extremely generic function that cannot possibly anticipate all flash configurations,
     *  please check the back end driver implementation for exact behavioral details.
     *
     *	@param[in]	address       The address to start erasing at
     *	@param[in]	length        How many bytes to erase
     *	@return Status
     */
    virtual Status erase( const size_t address, const size_t length ) = 0;

    /**
     *  Erase a chunk of memory
     *
     *  @param[in]  chunk         Which granularity to erase (page, block, etc)
     *  @param[in]  id            Index of the particular chunk to erase
     *  @return Status
     */
    virtual Status erase( const Chunk chunk, const size_t id ) = 0;

    /**
     *  Erases the entire chip. Typically this is a single command, so it saves on
     *  manual address calculations.
     *
     *  @return Status
     */
    virtual Status eraseChip() = 0;

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

    /**
     *	Register a callback to be executed on an event. If the event regards a memory
     *  transaction of some kind, the number of bytes involved will be passed in as
     *  an argument to the callback.
     *
     *	@param[in]	func          Function pointer to the callback
     *	@return Status
     *
     *  | Return Value |                    Explanation                   |
     *  |:------------:|:------------------------------------------------:|
     *  |           OK | The callback registration completed successfully |
     *  |         FAIL | The callback registration failed                 |
     */
    virtual Status onEvent( const Event event, void ( *func )( const size_t ) ) = 0;

    /**
     *  Enables/disables write protection on a particular chunk of memory
     *
     *  @param[in]  enable        Enable (true) or disable (false)
     *  @param[in]  chunk         Memory chunk resolution (page, block, sector, etc)
     *  @param[in]  id            Index of which chunk to act on
     *  @return Status
     */
    virtual Status writeProtect( const bool enable, const Chunk chunk, const size_t id ) = 0;

    /**
     *  Enables/disables read protection on a particular chunk of memory
     *
     *  @param[in]  enable        Enable (true) or disable (false)
     *  @param[in]  chunk         Memory chunk resolution (page, block, sector, etc)
     *  @param[in]  id            Index of which chunk to act on
     *  @return Status
     */
    virtual Status readProtect( const bool enable, const Chunk chunk, const size_t id ) = 0;

    /**
     *  Returns information about the device and its memory structure
     *
     *  @return Configuration
     */
    virtual Properties getDeviceProperties() = 0;
  };

}  // namespace Aurora::Memory

#endif /* !AURORA_GENERIC_MEMORY_INTF_HPP */
