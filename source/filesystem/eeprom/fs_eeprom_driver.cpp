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
#include <Aurora/logging>
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
    if( refreshMBRCache() && mMBRCache->isValid() )
    {
      return true;
    }

    LOG_ERROR( "Failed to mount. MBR could not be validated.\r\n" );
    return false;
  }


  void Manager::unmount()
  {
    mMBRCache->reset();
  }


  int Manager::softReset()
  {
    /*-------------------------------------------------------------------------
    Reset the MBR cache, then write it back to NVM. Results in an "empty" MBR
    but all the original data from files in the previous MBR will be preserved.
    -------------------------------------------------------------------------*/
    mMBRCache->reset();
    auto sts = mNVMDriver->write( mMBRCache->getStartOffset(), mMBRCache->cacheData(), mMBRCache->cacheSize() );
    if ( sts != Aurora::Memory::Status::ERR_OK )
    {
      LOG_ERROR( "Failed to write cleared MBR to NVM\r\n" );
      return -1;
    }

    return 0;
  }


  bool Manager::refreshMBRCache()
  {
    auto sts = mNVMDriver->read( mMBRCache->getStartOffset(), mMBRCache->cacheData(), mMBRCache->cacheSize() );
    if ( sts != Aurora::Memory::Status::ERR_OK )
    {
      LOG_ERROR( "Failed to read MBR from NVM\r\n" );
      return false;
    }

    return true;
  }

}  // namespace Aurora::FileSystem::EEPROM
