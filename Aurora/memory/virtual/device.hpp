/********************************************************************************
 *  File Name:
 *    device.hpp
 *
 *  Description:
 *    Tools useful when interfacing with a device that has controllable memory
 *
 *  2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#pragma once
#ifndef CHIMERA_MOD_MEMORY_DEVICE_HPP
#define CHIMERA_MOD_MEMORY_DEVICE_HPP

/* C++ Includes */
#include <limits>
#include <vector>

/* Chimera Includes */
#include <Chimera/common>

namespace Aurora::Memory
{
  /* clang-format off */
  class Status : public Chimera::CommonStatusCodes
  {
  public:
    static constexpr Chimera::Status_t OUT_OF_MEMORY     = status_offset_module_memory_flash + 1; /**< Pretty self-explanatory... */
    static constexpr Chimera::Status_t OVERRUN           = status_offset_module_memory_flash + 2; /**< The end of a buffer was hit pre-maturely */
    static constexpr Chimera::Status_t UNALIGNED_MEM     = status_offset_module_memory_flash + 3; /**< Memory was not aligned correctly */
    static constexpr Chimera::Status_t UNKNOWN_JEDEC     = status_offset_module_memory_flash + 4; /**< Device reported an invalid JEDEC code */
    static constexpr Chimera::Status_t HF_INIT_FAIL      = status_offset_module_memory_flash + 5; /**< High frequency interface failed to initialize */
    static constexpr Chimera::Status_t NOT_PAGE_ALIGNED  = status_offset_module_memory_flash + 6; /**< Memory is not page aligned */
    static constexpr Chimera::Status_t ERR_READ_PROTECT  = status_offset_module_memory_flash + 7;
    static constexpr Chimera::Status_t ERR_PGM_SEQUENCE  = status_offset_module_memory_flash + 8;
    static constexpr Chimera::Status_t ERR_PGM_PARALLEL  = status_offset_module_memory_flash + 9;
    static constexpr Chimera::Status_t ERR_PGM_ALIGNMENT = status_offset_module_memory_flash + 10; 
    static constexpr Chimera::Status_t ERR_WRITE_PROTECT = status_offset_module_memory_flash + 11;
  };
  /* clang-format on */

  enum class Section_t : uint8_t
  {
    PAGE = 0,
    BLOCK,
    SECTOR
  };

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

  /**
   *  A helper class that generates memory ranging information useful in flash memory driver applications.
   *  While named with 'Block', this technically can apply to pages, sectors, etc.
   */
  class BlockRange
  {
  public:
    /**
     *	Simple constructor to generate a new class from scratch
     *
     *	@param[in]	startAddress    Absolute address to start at
     *	@param[in]	endAddress      Absolute address greater than startAddress to end at
     *	@param[in]	blockSize       The block size in bytes
     */
    BlockRange( const size_t startAddress, const size_t endAddress, const size_t blockSize );

    /**
     *	Copy constructor to generate a new class from an existing one
     *
     *	@param[in]	cls             The existing class object
     */
    BlockRange( const BlockRange &cls );

    ~BlockRange() = default;

    /**
     *  Calculates the block of the startAddress
     *
     *  In the example below, this function will return the block number
     *  associated with 'a' because this is the block that the startAddress
     *  currently resides.
     *
     *  ================== Example ==================
     *  Data Range: ***   StartAddr: p1   EndAddr: p2
     *  Don't Care: ---   BlockBoundaries: a,b,c,d,e
     *
     *  a    p1    b          c          d   p2     e
     *  |-----*****|**********|**********|****------|
     *
     *	@return uint32_t
     *
     *	|             Return Value             |                             Explanation                             |
     *  |:------------------------------------:|:-------------------------------------------------------------------:|
     *  | std::numeric_limits<uint32_t>::max() | The class was not initialized with valid address range / block size |
     *  |                     All other values | The calculated data                                                 |
     */
    size_t startBlock();

    /**
     *  Starting from the beginning of startblock(), calculates the number
     *  of bytes until the startAddress has been reached (point 'p1').
     *
     *  In the example below, it would return the difference between
     *  the address of 'p1' and the address of 'a'. Mathematically this is:
     *
     *  startOffset = p1 - a;
     *
     *  ================== Example ==================
     *  Data Range: ***   StartAddr: p1   EndAddr: p2
     *  Don't Care: ---   blockBoundaries: a,b,c,d,e
     *
     *  a    p1    b          c          d   p2     e
     *  |-----*****|**********|**********|****------|
     *
     *	@return uint32_t
     *
     *	|             Return Value             |                             Explanation                             |
     *  |:------------------------------------:|:-------------------------------------------------------------------:|
     *  | std::numeric_limits<uint32_t>::max() | The class was not initialized with valid address range / block size |
     *  |                     All other values | The calculated data                                                 |
     */
    size_t startOffset();

    /**
     *  Starting from startAddress, calculates the number of bytes until the next block
     *  boundary point has been reached.
     *
     *  In the example below, it would return the difference between
     *  the address of 'b' and the address of 'p1'. Mathematically this is:
     *
     *  startBytes = b - p1;
     *
     *  ================== Example ==================
     *  Data Range: ***   StartAddr: p1   EndAddr: p2
     *  Don't Care: ---   blockBoundaries: a,b,c,d,e
     *
     *  a    p1    b          c          d   p2     e
     *  |-----*****|**********|**********|****------|
     *
     *	@return uint32_t
     *
     *	|             Return Value             |                             Explanation                             |
     *  |:------------------------------------:|:-------------------------------------------------------------------:|
     *  | std::numeric_limits<uint32_t>::max() | The class was not initialized with valid address range / block size |
     *  |                     All other values | The calculated data                                                 |
     */
    size_t startBytes();

    /**
     *  Calculates the block of the endAddress
     *
     *  In the example below, this function will return the block number
     *  associated with 'd' because this is the block that the endAddress
     *  currently resides.
     *
     *  ================== Example ==================
     *  Data Range: ***   StartAddr: p1   EndAddr: p2
     *  Don't Care: ---   blockBoundaries: a,b,c,d,e
     *
     *  a    p1    b          c          d   p2     e
     *  |-----*****|**********|**********|****------|
     *
     *	@return uint32_t
     *
     *	|             Return Value             |                             Explanation                             |
     *  |:------------------------------------:|:-------------------------------------------------------------------:|
     *  | std::numeric_limits<uint32_t>::max() | The class was not initialized with valid address range / block size |
     *  |                     All other values | The calculated data                                                 |
     */
    size_t endBlock();

    /**
     *  Starting from the beginning of endblock(), calculates the number
     *  of bytes until the endAddress has been reached (point 'p2').
     *
     *  In the example below, it would return the difference between
     *  the address of 'p2' and the address of 'd'. Mathematically this is:
     *
     *  endOffset = p2 - d;
     *
     *  ================== Example ==================
     *  Data Range: ***   StartAddr: p1   EndAddr: p2
     *  Don't Care: ---   blockBoundaries: a,b,c,d,e
     *
     *  a    p1    b          c          d   p2     e
     *  |-----*****|**********|**********|****------|
     *
     *	@return uint32_t
     *
     *	|             Return Value             |                             Explanation                             |
     *  |:------------------------------------:|:-------------------------------------------------------------------:|
     *  | std::numeric_limits<uint32_t>::max() | The class was not initialized with valid address range / block size |
     *  |                     All other values | The calculated data                                                 |
     */
    size_t endOffset();

    /**
     *  Starting from endAddress, calculates the number of bytes until the next block
     *  boundary point has been reached.
     *
     *  In the example below, it would return the difference between
     *  the address of 'e' and the address of 'p2'. Mathematically this is:
     *
     *  startBytes = e - p2;
     *
     *  ================== Example ==================
     *  Data Range: ***   StartAddr: p1   EndAddr: p2
     *  Don't Care: ---   blockBoundaries: a,b,c,d,e
     *
     *  a    p1    b          c          d   p2     e
     *  |-----*****|**********|**********|****------|
     *
     *	@return uint32_t
     *
     *	|             Return Value             |                             Explanation                             |
     *  |:------------------------------------:|:-------------------------------------------------------------------:|
     *  | std::numeric_limits<uint32_t>::max() | The class was not initialized with valid address range / block size |
     *  |                     All other values | The calculated data                                                 |
     */
    size_t endBytes();

  protected:
    static constexpr size_t SIZE_T_MAX = std::numeric_limits<size_t>::max();

    bool initialized = false;

    size_t _blockSize    = SIZE_T_MAX;
    size_t _startBlock   = SIZE_T_MAX;
    size_t _startAddress = SIZE_T_MAX;
    size_t _startOffset  = SIZE_T_MAX;
    size_t _startBytes   = SIZE_T_MAX;
    size_t _endBlock     = SIZE_T_MAX;
    size_t _endAddress   = SIZE_T_MAX;
    size_t _endOffset    = SIZE_T_MAX;
    size_t _endBytes     = SIZE_T_MAX;
  };

  /**
   *  Models interactions with a memory device from the perspective that it is
   *  one continuous block of memory. Paging, partitioning, and all other device specific
   *  information is left up to the inheriting driver. All the user cares about is that
   *  data can be written, read, and erased.
   */
  class Device
  {
  public:
    /**
     *	Virtual destructor necessary for GMock as well as inheritors
     */
    virtual ~Device() = default;

    /**
     *	Writes data into flash memory.
     *
     *  @note   Does not require page alignment, but be aware of the possibility that unaligned
     *          data could possibly take longer to write. This is dependent upon the specific device.
     *
     *	@param[in]	address       The start address to write data into
     *	@param[in]	data          The buffer of data that will be written
     *	@param[in]	length        Number of bytes to be written
     *	@return Chimera::Status_t
     *
     *  |  Return Value |                             Explanation                            |
     *  |:-------------:|:------------------------------------------------------------------:|
     *  |            OK | The write completed successfully                                   |
     *  |          FAIL | The write did not succeed for some reason (device specific)        |
     *  |          BUSY | Flash is doing something at the moment. Try again later.           |
     *  | OUT_OF_MEMORY | Zero or more bytes were written, but not the full amount requested |
     */
    virtual Chimera::Status_t write( const size_t address, const uint8_t *const data, const size_t length ) = 0;

    /**
     *  Reads data in a contiguous block, starting from the given address. Should *not* be able to
     *  read across the end of the device memory and wrap around to the beginning.
     *
     *	@param[in]	address       The address to start the read from
     *	@param[out]	data          Buffer of data to read into
     *	@param[in]	length        How many bytes to read out
     *	@return Chimera::Status_t
     *
     *  | Return Value |                         Explanation                         |
     *  |:------------:|:-----------------------------------------------------------:|
     *  |           OK | The read completed successfully                             |
     *  |         FAIL | The read did not succeed for some reason (device specific)  |
     *  |         BUSY | Flash is doing something at the moment. Try again later.    |
     *  |      OVERRUN | A boundary was reached and the read halted.                 |
     */
    virtual Chimera::Status_t read( const size_t address, uint8_t *const data, const size_t length ) = 0;

    /**
     *  Erase a region of memory. The given address range will need to be page, block, or
     *  sector aligned, depending upon what the underlying system requires. Because this is
     *  an extremely generic function that cannot possibly anticipate all flash configurations,
     *  please check the back end driver implementation for exact behavioral details.
     *
     *	@param[in]	address       The address to start erasing at
     *	@param[in]	length        How many bytes to erase
     *	@return Chimera::Status_t
     *
     *  | Return Value |                         Explanation                         |
     *  |:------------:|:-----------------------------------------------------------:|
     *  |           OK | The erase completed successfully                            |
     *  |         FAIL | The erase did not succeed for some reason (device specific) |
     *  |         BUSY | Flash is doing something at the moment. Try again later.    |
     *  |    UNALIGNED | The range wasn't aligned with the device's erasable regions |
     */
    virtual Chimera::Status_t erase( const size_t address, const size_t length ) = 0;

    /**
     *	Register a callback to be executed when the write has been completed. The input parameter
     *  will let the function know how many bytes were actually written.
     *
     *	@param[in]	func          Function pointer to the callback
     *	@return Chimera::Status_t
     *
     *  | Return Value |                    Explanation                   |
     *  |:------------:|:------------------------------------------------:|
     *  |           OK | The callback registration completed successfully |
     *  |         FAIL | The callback registration failed                 |
     */
    virtual Chimera::Status_t writeCompleteCallback( const Chimera::Function::void_func_uint32_t func ) = 0;

    /**
     *	Register a callback to be executed when the read has been completed. The input parameter
     *  will let the function know how many bytes were actually read.
     *
     *	@param[in]	func          Function pointer to the callback
     *	@return Chimera::Status_t
     *
     *  | Return Value |                    Explanation                   |
     *  |:------------:|:------------------------------------------------:|
     *  |           OK | The callback registration completed successfully |
     *  |         FAIL | The callback registration failed                 |
     */
    virtual Chimera::Status_t readCompleteCallback( const Chimera::Function::void_func_uint32_t func ) = 0;

    /**
     *	Register a callback to be executed when the erase has been completed. The input parameter
     *  will let the function know how many bytes were actually erased.
     *
     *	@param[in]	func          Function pointer to the callback
     *	@return Chimera::Status_t
     *
     *  | Return Value |                    Explanation                   |
     *  |:------------:|:------------------------------------------------:|
     *  |           OK | The callback registration completed successfully |
     *  |         FAIL | The callback registration failed                 |
     */
    virtual Chimera::Status_t eraseCompleteCallback( const Chimera::Function::void_func_uint32_t func ) = 0;

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

  using Device_sPtr = std::shared_ptr<Device>;
  using Device_uPtr = std::unique_ptr<Device>;

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

    Chimera::Status_t write( const size_t address, const uint8_t *const data, const size_t length ) final override;

    Chimera::Status_t read( const size_t address, uint8_t *const data, const size_t length ) final override;

    Chimera::Status_t erase( const size_t address, const size_t length ) final override;

    Chimera::Status_t writeCompleteCallback( const Chimera::Function::void_func_uint32_t func ) final override;

    Chimera::Status_t readCompleteCallback( const Chimera::Function::void_func_uint32_t func ) final override;

    Chimera::Status_t eraseCompleteCallback( const Chimera::Function::void_func_uint32_t func ) final override;

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

    Chimera::Status_t writeProtect( const bool state )
    {
      return Chimera::CommonStatusCodes::NOT_SUPPORTED;
    }

    Chimera::Status_t readProtect( const bool state )
    {
      return Chimera::CommonStatusCodes::NOT_SUPPORTED;
    }
  };

}  // namespace Chimera::Modules::Memory

#endif /* !CHIMERA_MOD_MEMORY_DEVICE_HPP */
