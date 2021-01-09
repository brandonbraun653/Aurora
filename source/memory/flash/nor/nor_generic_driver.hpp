/********************************************************************************
 *  File Name:
 *    nor_generic_driver.hpp
 *
 *  Description:
 *    NOR flash generic device driver
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef NOR_FLASH_GENERIC_DEVICE_DRIVER_HPP
#define NOR_FLASH_GENERIC_DEVICE_DRIVER_HPP

/* Chimera Includes */
#include <Chimera/common>
#include <Chimera/thread>
#include <Chimera/spi>

/* Aurora Includes */
#include <Aurora/source/memory/flash/jedec/jedec_cfi_cmds.hpp>
#include <Aurora/source/memory/generic/generic_intf.hpp>
#include <Aurora/source/memory/generic/generic_types.hpp>
#include <Aurora/source/memory/flash/nor/nor_generic_types.hpp>

namespace Aurora::Flash::NOR
{
  /*-------------------------------------------------------------------------------
  Public Functions
  -------------------------------------------------------------------------------*/
  /**
   *  Looks up the memory properties for the given chip
   *
   *  @param[in]  device        Which device to look up
   *  @return Properties *      If exists, the property table
   */
  const Aurora::Memory::Properties *getProperties( const Chip_t device );

  /**
   *  Converts a chunk ID into a physical address for the given device
   *
   *  @param[in]  device        The device to convert into
   *  @param[in]  page          Page index number
   *  @param[out] address       The physical address of the chunk
   *  @return bool              Validity flag for the address
   */
  bool page2Address( const Chip_t device, const size_t page, size_t *const address );

  /**
   *  Converts a block ID into a physical address for the given device
   *
   *  @param[in]  device        The device to convert into
   *  @param[in]  block         Block index number
   *  @param[out] address       The physical address of the chunk
   *  @return bool              Validity flag for the address
   */
  bool block2Address( const Chip_t device, const size_t block, size_t *const address );

  /**
   *  Converts a sector ID into a physical address for the given device
   *
   *  @param[in]  device        The device to convert into
   *  @param[in]  sector        Sector index number
   *  @param[out] address       The physical address of the chunk
   *  @return bool              Validity flag for the address
   */
  bool sector2Address( const Chip_t device, const size_t sector, size_t *const address );

  /*-------------------------------------------------------------------------------
  Classes
  -------------------------------------------------------------------------------*/
  /**
   *  A generic CFI memory driver to talk with NOR flash
   */
  class Driver : public virtual Aurora::Memory::IGenericDevice, public Chimera::Threading::Lockable
  {
  public:
    Driver();
    ~Driver();

    /*-------------------------------------------------
    Generic Memory Device Interface
    -------------------------------------------------*/
    Aurora::Memory::Status open() final override;
    Aurora::Memory::Status close() final override;
    Aurora::Memory::Status write( const size_t chunk, const size_t offset, const void *const data, const size_t length ) final override;
    Aurora::Memory::Status read( const size_t chunk, const size_t offset, void *const data, const size_t length ) final override;
    Aurora::Memory::Status erase( const size_t chunk ) final override;
    Aurora::Memory::Status erase() final override;
    Aurora::Memory::Status flush() final override;

    /*-------------------------------------------------
    NOR Driver Interface
    -------------------------------------------------*/
    /**
     *  Configures the driver for operation.
     *
     *  @param[in]  device      Which NOR chip is being used
     *  @param[in]  channel     Which SPI channel is being used
     *  @return bool
     */
    bool configure( const Chip_t device, const Chimera::SPI::Channel channel );

    /**
     *  Reads the status register bytes
     *
     *  @return uint16_t
     */
    uint16_t readStatusRegister();

  private:
    Chip_t mChip;                                      /**< Memory chip in use */
    Chimera::SPI::Driver_sPtr mSPI;                  /**< SPI driver instance */
    std::array<uint8_t, CFI::MAX_CMD_LEN> cmdBuffer; /**< Buffer for holding a command sequence */

    /*-------------------------------------------------
    Private Functions
    -------------------------------------------------*/
    void issueWriteEnable();
  };
}  // namespace Aurora::NOR

#endif  /* !NOR_FLASH_GENERIC_DEVICE_DRIVER_HPP */
