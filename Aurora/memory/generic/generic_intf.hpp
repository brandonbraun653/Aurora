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
#include <Aurora/memory/generic/generic_types.hpp>

namespace Aurora::Memory
{
  /**
   *  Models interactions with a memory device from the perspective that it is
   *  one continuous block of memory. Paging, partitioning, and all other device specific
   *  information is left up to the inheriting driver. All the user cares about is that
   *  data can be written, read, and erased.
   */
  class IDevice
  {
  public:
    /**
     *	Virtual destructor necessary for GMock as well as inheritors
     */
    virtual ~IDevice() = default;

    /**
     *	Writes data into flash memory.
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
    virtual Status write( const size_t address, const uint8_t *const data, const size_t length ) = 0;

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
    virtual Status read( const size_t address, uint8_t *const data, const size_t length ) = 0;

    /**
     *  Erase a region of memory. The given address range will need to be page, block, or
     *  sector aligned, depending upon what the underlying system requires. Because this is
     *  an extremely generic function that cannot possibly anticipate all flash configurations,
     *  please check the back end driver implementation for exact behavioral details.
     *
     *	@param[in]	address       The address to start erasing at
     *	@param[in]	length        How many bytes to erase
     *	@return Status
     *
     *  | Return Value |                         Explanation                         |
     *  |:------------:|:-----------------------------------------------------------:|
     *  |           OK | The erase completed successfully                            |
     *  |         FAIL | The erase did not succeed for some reason (device specific) |
     *  |         BUSY | Flash is doing something at the moment. Try again later.    |
     *  |    UNALIGNED | The range wasn't aligned with the device's erasable regions |
     */
    virtual Status erase( const size_t address, const size_t length ) = 0;

    /**
     *	Register a callback to be executed when the write has been completed. The input parameter
     *  will let the function know how many bytes were actually written.
     *
     *	@param[in]	func          Function pointer to the callback
     *	@return Status
     *
     *  | Return Value |                    Explanation                   |
     *  |:------------:|:------------------------------------------------:|
     *  |           OK | The callback registration completed successfully |
     *  |         FAIL | The callback registration failed                 |
     */
    virtual Status writeCompleteCallback( void (*func)(const size_t) ) = 0;

    /**
     *	Register a callback to be executed when the read has been completed. The input parameter
     *  will let the function know how many bytes were actually read.
     *
     *	@param[in]	func          Function pointer to the callback
     *	@return Status
     *
     *  | Return Value |                    Explanation                   |
     *  |:------------:|:------------------------------------------------:|
     *  |           OK | The callback registration completed successfully |
     *  |         FAIL | The callback registration failed                 |
     */
    virtual Status readCompleteCallback( const Chimera::Function::void_func_uint32_t func ) = 0;

    /**
     *	Register a callback to be executed when the erase has been completed. The input parameter
     *  will let the function know how many bytes were actually erased.
     *
     *	@param[in]	func          Function pointer to the callback
     *	@return Status
     *
     *  | Return Value |                    Explanation                   |
     *  |:------------:|:------------------------------------------------:|
     *  |           OK | The callback registration completed successfully |
     *  |         FAIL | The callback registration failed                 |
     */
    virtual Status eraseCompleteCallback( const Chimera::Function::void_func_uint32_t func ) = 0;

    /**
     *	Checks if the device has been initialized properly and is ok to talk with
     *
     *	@return bool
     *
     *  | Return Value |           Explanation           |
     *  |:------------:|:-------------------------------:|
     *  |         true | The device has been initialized |
     *  |        false | The device is not initialized   |
     */
    virtual bool isInitialized() = 0;
  };


  /**
   *  Contains useful helper functions for interacting with a memory device that can be
   *  described with a DeviceDescriptor.
   */
  class Utilities
  {
  public:
    Utilities( const Descriptor &dev );
    ~Utilities() = default;

    /**
     *	Updates internal information about the memory device being modeled
     *
     *	@param[in]	dev         The device information
     *	@return void
     */
    void updateDeviceInfo( const Descriptor &dev );

    /**
     *	Returns the section number of the address
     *
     *	@param[in]	section     Which section you are interested in
     *	@param[in]	address     The address to be investigated
     *	@return uint32_t
     */
    size_t getSectionNumber( const Section_t section, const size_t address );

    /**
     *	Converts a section number into that section's start address
     *
     *	@param[in]	section     Which section you are interested in
     *	@param[in]	number      The specific section number to convert
     *	@return uint32_t
     */
    size_t getSectionStartAddress( const Section_t section, const size_t number );

    /**
     *	An algorithm for re-structuring an address range into the largest memory groupings possible
     *	to allow efficient access of the memory architecture.
     *
     *	This was originally designed to aide erase commands so that the programmer could erase in as
     *	few commands as possible. For instance, a large erase of 139kB might be broken down into
     *	2 sectors, several blocks, and a few pages. The alternative would be to erase one page at a
     *	time, which isn't very efficient. Most chips have the option to erase by page, block, or sector,
     *	so this minimizes the number of operations needed by software to erase a generic section of
     *	memory.
     *
     *	@note For simplicity, this algorithm requires that the address and length given are page aligned.
     *
     *	@warning The algorithm relies on dynamic memory allocation. With FreeRTOS, use heap4 or heap5.
     *
     *	@param[in]	address     The page aligned starting address
     *	@param[in]	len         The page aligned range of bytes to be composed
     *	@return Chimera::Modules::Memory::SectionList
     */
    SectionList getCompositeSections( const size_t address, const size_t len );

  private:
    Descriptor device;

    size_t pagesPerBlock;
    size_t pagesPerSector;
    size_t blocksPerSector;
  };


  class VirtualMemoryDevice : public Aurora::Memory::Device
  {
  public:
    template<std::size_t S>
    void initialize( std::array<uint8_t, S>& staticData )
    {
      rawData = staticData.data();
      deviceDescriptor.startAddress = reinterpret_cast<size_t>( rawData );
      deviceDescriptor.endAddress   = deviceDescriptor.startAddress + S;
      regionSize                    = S;

      initialized = true;
    }

    const Aurora::Memory::Descriptor &getSpecs()
    {
      return deviceDescriptor;
    }

    Status write( const size_t address, const uint8_t *const data, const size_t length ) final override;

    Status read( const size_t address, uint8_t *const data, const size_t length ) final override;

    Status erase( const size_t address, const size_t length ) final override;

    Status writeCompleteCallback( const Chimera::Function::void_func_uint32_t func ) final override;

    Status readCompleteCallback( const Chimera::Function::void_func_uint32_t func ) final override;

    Status eraseCompleteCallback( const Chimera::Function::void_func_uint32_t func ) final override;

    bool isInitialized() final override;

  protected:
    uint8_t *rawData = nullptr;
    size_t regionSize = 0;
    Aurora::Memory::Descriptor deviceDescriptor;
    bool initialized = false;
  };

  using VMD_sPtr = std::shared_ptr<VirtualMemoryDevice>;
  using VMD_uPtr = std::unique_ptr<VirtualMemoryDevice>;

  class AccessProtected
  {
  public:
    ~AccessProtected() = default;

    Status writeProtect( const bool state )
    {
      return Chimera::Status::NOT_SUPPORTED;
    }

    Status readProtect( const bool state )
    {
      return Chimera::Status::NOT_SUPPORTED;
    }
  };

}  // namespace Chimera::Modules::Memory

#endif /* !AURORA_GENERIC_MEMORY_INTF_HPP */
