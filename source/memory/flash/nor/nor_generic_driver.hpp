/********************************************************************************
 *  File Name:
 *    nor_generic_driver.hpp
 *
 *  Description:
 *    NOR flash generic device driver
 *
 *  2021-2022 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef NOR_FLASH_GENERIC_DEVICE_DRIVER_HPP
#define NOR_FLASH_GENERIC_DEVICE_DRIVER_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/source/memory/flash/jedec/jedec_cfi_cmds.hpp>
#include <Aurora/source/memory/flash/nor/nor_generic_types.hpp>
#include <Aurora/source/memory/generic/generic_intf.hpp>
#include <Aurora/source/memory/generic/generic_types.hpp>
#include <Chimera/common>
#include <Chimera/spi>
#include <Chimera/thread>

namespace Aurora::Memory::Flash::NOR
{
  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/
  /**
   * @brief Looks up the memory properties for the given chip
   *
   * @param device        Which device to look up
   * @return Properties * If exists, the property table
   */
  const Aurora::Memory::Properties *getProperties( const Chip_t device );

  /**
   * @brief Converts a chunk ID into a physical address for the given device
   *
   * @param device      The device to convert into
   * @param page        Page index number
   * @param address     The physical address of the chunk
   * @return bool       Validity flag for the address
   */
  bool page2Address( const Chip_t device, const size_t page, size_t *const address );

  /**
   * @brief Converts a block ID into a physical address for the given device
   *
   * @param device      The device to convert into
   * @param block       Block index number
   * @param address     The physical address of the chunk
   * @return bool       Validity flag for the address
   */
  bool block2Address( const Chip_t device, const size_t block, size_t *const address );

  /**
   *  @brief Converts a sector ID into a physical address for the given device
   *
   * @param device      The device to convert into
   * @param sector      Sector index number
   * @param address     The physical address of the chunk
   * @return bool       Validity flag for the address
   */
  bool sector2Address( const Chip_t device, const size_t sector, size_t *const address );

  /**
   * @brief Converts a raw memory address into the chunk id and byte offset for the device configured
   * write chunk size.
   *
   * @param device      The device being converted
   * @param address     The raw address to parse
   * @param chunk       Base write chunk ID the address lives in
   * @param offset      Byte offset inside the write chunk
   * @return bool
   */
  bool address2WriteChunkOffset( const Chip_t device, const size_t address, size_t *const chunk, size_t *const offset );

  /*---------------------------------------------------------------------------
  Classes
  ---------------------------------------------------------------------------*/
  /**
   * @brief A generic CFI memory driver to talk with NOR flash
   * @see https://en.wikipedia.org/wiki/Common_Flash_Memory_Interface
   */
  class Driver : public virtual Aurora::Memory::IGenericDevice, public Chimera::Thread::Lockable<Driver>
  {
  public:
    Driver();
    ~Driver();

    /*-------------------------------------------------------------------------
    IGenericDevice
    -------------------------------------------------------------------------*/
    Aurora::Memory::Status open( const DeviceAttr *const attributes ) final override;
    Aurora::Memory::Status close() final override;
    Aurora::Memory::Status write( const size_t chunk, const size_t offset, const void *const data,
                                  const size_t length ) final override;
    Aurora::Memory::Status write( const size_t address, const void *const data, const size_t length ) final override;
    Aurora::Memory::Status read( const size_t chunk, const size_t offset, void *const data,
                                 const size_t length ) final override;
    Aurora::Memory::Status read( const size_t address, void *const data, const size_t length ) final override;
    Aurora::Memory::Status erase( const size_t chunk ) final override;
    Aurora::Memory::Status erase( const size_t address, const size_t length ) final override;
    Aurora::Memory::Status erase() final override;
    Aurora::Memory::Status flush() final override;
    Aurora::Memory::Status pendEvent( const Aurora::Memory::Event event, const size_t timeout ) final override;

    /*-------------------------------------------------------------------------
    NOR Driver Interface
    -------------------------------------------------------------------------*/
    /**
     * @brief Configures the driver for operation.
     *
     * @param device      Which NOR chip is being used
     * @param channel     Which SPI channel is being used
     * @return bool
     */
    bool configure( const Chip_t device, const Chimera::SPI::Channel channel );

    /**
     * @brief Exposes the raw data bus interface to the user
     *
     * @param cmd     Buffer to the command to send to the NOR chip
     * @param output  Buffer to read in the RX half of the transfer
     * @param size    Size of both buffers
     */
    void transfer( const void *const cmd, void *const output, const size_t size );

    /**
     * @brief Manually assigns a chip select line for internal control
     *
     * @param port    GPIO port the CS is on
     * @param pin     GPIO pin mapped to the CS
     * @return bool
     */
    bool assignChipSelect( const Chimera::GPIO::Port port, const Chimera::GPIO::Pin pin );

    /**
     * @brief Gets the configured device type
     * @return Chip_t
     */
    Chip_t deviceType() const;

    /**
     * @brief Get the device attributes
     * @return DeviceAttr
     */
    DeviceAttr getAttr() const;

  private:
    friend Chimera::Thread::Lockable<Driver>;

    Chip_t                                mChip;       /**< Memory chip in use */
    DeviceAttr                            mAttr;       /**< Device attributes for access sizes*/
    const Properties                     *mProps;      /**< Device properties for timing and general info */
    Chimera::SPI::Channel                 mSPIChannel; /**< SPI driver channel */
    Chimera::SPI::Driver_rPtr             mSPI;        /**< SPI driver instance */
    Chimera::GPIO::Driver_rPtr            mCS;         /**< Chip select GPIO driver instance */
    std::array<uint8_t, CFI::MAX_CMD_LEN> cmdBuffer;   /**< Buffer for holding a command sequence */

    void issueWriteEnable();
  };
}  // namespace Aurora::Memory::Flash::NOR

#endif /* !NOR_FLASH_GENERIC_DEVICE_DRIVER_HPP */
