/******************************************************************************
 *  File Name:
 *    device_test.hpp
 *
 *  Description:
 *    Utilities for testing out a flash memory device driver's read/write/erase
 *    capabilities and performance.
 *
 *  2022 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef AURORA_MEMORY_FLASH_DEVICE_TEST_HPP
#define AURORA_MEMORY_FLASH_DEVICE_TEST_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/source/memory/generic/generic_intf.hpp>

namespace Aurora::Memory::Flash
{
  /*---------------------------------------------------------------------------
  Classes
  ---------------------------------------------------------------------------*/
  class DeviceTest
  {
  public:
    struct Config
    {
      IGenericDevice *dut;         /**< Instantiated device under test */
      void           *writeBuffer; /**< Memory for generating write information */
      void           *readBuffer;  /**< Memory for accepting read information */
      size_t          bufferSize;  /**< Size of both buffers */
      size_t          maxAddress;  /**< Max address of the DUT */
      size_t          pageSize;    /**< Read/write page size of DUT */
      size_t          blockSize;   /**< Read/write block size of the DUT */
      size_t          sectorSize;  /**< Read/write sector size of the DUT */
      size_t          eraseSize;   /**< Erase size of the DUT. Must match device driver config. */
    };

    DeviceTest();
    ~DeviceTest();

    /**
     * @brief Prepares the tester for operation
     */
    void initialize( Config &cfg );

    /**
     * @brief Validates data transaction at the page level
     *
     * @param page    Page number to access
     * @param erase   Erase the page first
     * @return Aurora::Memory::Status
     */
    Aurora::Memory::Status pageAccess( const size_t page, const bool erase );

    /**
     * @brief Validates data transaction at the block level
     *
     * @param page    Block number to access
     * @param erase   Erase the block first
     * @return Aurora::Memory::Status
     */
    Aurora::Memory::Status blockAccess( const size_t block, const bool erase );

    /**
     * @brief Validates data transaction at the sector level
     *
     * @param page    Sector number to access
     * @param erase   Erase the sector first
     * @return Aurora::Memory::Status
     */
    Aurora::Memory::Status sectorAccess( const size_t sector, const bool erase );

    /**
     * @brief Validates random transactions across the whole device
     *
     * @param limit   Maximum number of transactions
     * @param erase   Erase the write regions first
     * @return Aurora::Memory::Status
     */
    Aurora::Memory::Status randomAccess( const size_t limit, const bool erase );

    /**
     * @brief Erases a given chunk a multiple of the DUT erase size
     *
     * @param chunk  Chunk number to erase
     * @return Aurora::Memory::Status
     */
    Aurora::Memory::Status erase( const size_t chunk );

  private:
    Config mCfg;

    /**
     * @brief General access function to validate a chunk of memory at an address
     *
     * @param address   Absolute address to access
     * @param size      Number of bytes to access
     * @return Aurora::Memory::Status
     */
    Aurora::Memory::Status dutAccess( const size_t address, const size_t size );
  };
}  // namespace Aurora::Memory::Flash

#endif /* !AURORA_MEMORY_FLASH_DEVICE_TEST_HPP */
