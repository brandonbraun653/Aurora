/******************************************************************************
 *  File Name:
 *    fs_eeprom_driver.cpp
 *
 *  Description:
 *    Implementation of the EEPROM file system driver
 *
 *  2022 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/filesystem>
#include <Aurora/memory>
#include <Chimera/common>

namespace Aurora::FileSystem::EEPROM
{
  /*---------------------------------------------------------------------------
  Static Data
  ---------------------------------------------------------------------------*/
  static Aurora::Flash::EEPROM::Driver sEEPROMFlash; /**< Flash memory driver supporting the file system */
  static Aurora::Flash::EEPROM::Chip_t sDevice;      /**< Flash memory device to use for the file system */


  /*---------------------------------------------------------------------------
  Driver Specific Implementation
  ---------------------------------------------------------------------------*/
  bool attachDevice( const uint16_t address, const Aurora::Flash::EEPROM::Chip_t dev, const Chimera::I2C::Channel channel )
  {
    /*-------------------------------------------------------------------------
    Ensure the device is supported by the driver
    -------------------------------------------------------------------------*/
    auto props = Aurora::Flash::EEPROM::getProperties( dev );
    RT_DBG_ASSERT( props );
    sDevice = dev;

    /*-------------------------------------------------------------------------
    Build up the configuration for the driver and initialize it
    -------------------------------------------------------------------------*/
    Aurora::Flash::EEPROM::DeviceConfig cfg;
    cfg.clear();
    cfg.whichChip     = dev;
    cfg.deviceAddress = address;
    cfg.i2cChannel    = channel;

    return sEEPROMFlash.configure( cfg );
  }


  Aurora::Flash::EEPROM::Driver* getEEPROMDriver()
  {
    return &sEEPROMFlash;
  }

}  // namespace Aurora::FileSystem::EEPROM
