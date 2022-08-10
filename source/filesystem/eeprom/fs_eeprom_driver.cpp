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
  static FSConfig                      sConfig;      /**< Configuration for the file system */


  /*---------------------------------------------------------------------------
  Driver Specific Implementation
  ---------------------------------------------------------------------------*/
  bool configure( const FSConfig &config )
  {
    /*-------------------------------------------------------------------------
    Ensure the device is supported by the driver
    -------------------------------------------------------------------------*/
    RT_DBG_ASSERT( Aurora::Flash::EEPROM::getProperties( config.device ) );
    sConfig = config;

    /*-------------------------------------------------------------------------
    Build up the configuration for the driver and initialize it
    -------------------------------------------------------------------------*/
    Aurora::Flash::EEPROM::DeviceConfig cfg;
    cfg.clear();
    cfg.whichChip     = sConfig.device;
    cfg.deviceAddress = sConfig.address;
    cfg.i2cChannel    = sConfig.channel;

    return sEEPROMFlash.configure( cfg );
  }


  Aurora::Flash::EEPROM::Driver *getEEPROMDriver()
  {
    return &sEEPROMFlash;
  }


  const FSConfig &getConfiguration()
  {
    return sConfig;
  }

  /*---------------------------------------------------------------------------
  Class Implementations
  ---------------------------------------------------------------------------*/
  Manager::Manager() : mMBRCache( nullptr ), mNVMDriver( nullptr )
  {
  }


  Manager::~Manager()
  {
  }


  void Manager::configure( Aurora::Flash::EEPROM::Driver *const driver, iMBR *const mbrCache )
  {
    RT_DBG_ASSERT( driver && mbrCache );

    mNVMDriver = driver;
    mMBRCache  = mbrCache;
  }


  bool Manager::mount()
  {
    // Read in data from NVM into the MBR cache, then validate it.
    // Focus on how to reduce the number of virtual calls. Can I somehow call the child class
    // from the parent so I can keep the fun business logic in the parent and the data spec in
    // the child?

    return false;
  }


  void Manager::unmount()
  {
  }


  int Manager::softReset()
  {
    return 0;
  }


  bool Manager::refreshMBRCache()
  {
    return true;
  }

}  // namespace Aurora::FileSystem::EEPROM
