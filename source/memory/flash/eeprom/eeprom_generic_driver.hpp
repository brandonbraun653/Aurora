/******************************************************************************
 *  File Name:
 *    eeprom_generic_driver.hpp
 *
 *  Description:
 *    Generic driver for EEPROM memory
 *
 *  2022 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef AURORA_MEMORY_EEPROM_GENERIC_DRIVER_HPP
#define AURORA_MEMORY_EEPROM_GENERIC_DRIVER_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Chimera/common>
#include <Chimera/thread>
#include <Chimera/i2c>
#include <Aurora/source/memory/generic/generic_intf.hpp>
#include <Aurora/source/memory/generic/generic_types.hpp>
#include <Aurora/source/memory/flash/eeprom/eeprom_generic_types.hpp>

namespace Aurora::Flash::EEPROM
{
  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/
  /**
   * @brief Get the Properties object for a specific chip
   *
   * @param device  The chip to look up
   * @return const Aurora::Memory::Properties*
   */
  const Aurora::Memory::Properties *getProperties( const Chip_t device );

  /*---------------------------------------------------------------------------
  Classes
  ---------------------------------------------------------------------------*/
  class Driver : public virtual Aurora::Memory::IGenericDevice, public Chimera::Thread::Lockable<Driver>
  {
  public:
    Driver();
    ~Driver();

    /*-------------------------------------------------------------------------
    Generic Memory Device Interface
    -------------------------------------------------------------------------*/
    Aurora::Memory::Status open() final override;
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

    /*-----------------------------------------------------------------------------
    EEPROM Driver Interface
    -----------------------------------------------------------------------------*/
    /**
     * @brief Attaches a device configuration to the class.
     * @note Must be called before open()
     *
     * @param config    Configuration to use
     * @return bool     Configuration pass/fail
     */
    bool configure( const DeviceConfig &config );

  private:
    friend Chimera::Thread::Lockable<Driver>;

    DeviceConfig              mConfig;  /**< Device configuration */
    Chimera::I2C::Driver_rPtr mDriver;  /**< Hardware driver instance */
  };
}  // namespace Aurora::Flash::EEPROM

#endif /* !AURORA_MEMORY_EEPROM_GENERIC_DRIVER_HPP */
