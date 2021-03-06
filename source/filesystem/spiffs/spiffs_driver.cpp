/********************************************************************************
 *  File Name:
 *    spiffs_driver.cpp
 *
 *  Description:
 *    Filesystem implementation redirects into the SPIFFS driver
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* STL Includes */
#include <cstdint>

/* Aurora Include */
#include <Aurora/filesystem_spiffs>
#include <Aurora/logging>
#include <Aurora/memory>
#include <Aurora/source/filesystem/file_config.hpp>
#include <Aurora/source/filesystem/file_driver.hpp>

/* Chimera Includes */
#include <Chimera/assert>
#include <Chimera/thread>

/* SPIFFS Includes */
#include "spiffs.h"


namespace Aurora::FileSystem::SPIFFS
{
  /*-------------------------------------------------------------------------------
  Static Functions
  -------------------------------------------------------------------------------*/
  extern "C"
  {
    extern void SPIFFS_fs_lock( void *const fs );
    extern void SPIFFS_fs_unlock( void *const fs );
  }


  /*-------------------------------------------------------------------------------
  Static Data
  -------------------------------------------------------------------------------*/
  static Aurora::Flash::NOR::Driver sNORFlash;                  /**< Flash memory driver supporting the file system */
  static Aurora::Flash::NOR::Chip_t sDevice;
  static Chimera::Thread::RecursiveMutex s_spiffs_lock;


  /*-------------------------------------------------------------------------------
  Driver Specific Implementation
  -------------------------------------------------------------------------------*/
  bool attachDevice( const Aurora::Flash::NOR::Chip_t dev, const Chimera::SPI::Channel channel )
  {
    sDevice = dev;
    return sNORFlash.configure( dev, channel );
  }


  Aurora::Flash::NOR::Driver *getNORDriver()
  {
    return &sNORFlash;
  }


  bool fullChipErase( const size_t timeout )
  {
    using namespace Aurora::Memory;

    /*-------------------------------------------------
    Issue the erase command
    -------------------------------------------------*/
    if ( sNORFlash.erase() != Status::ERR_OK )
    {
      return false;
    }

    /*-------------------------------------------------
    Wait for the erase to complete
    -------------------------------------------------*/
    return sNORFlash.pendEvent( Event::MEM_ERASE_COMPLETE, timeout ) == Status::ERR_OK;
  }


  /*-------------------------------------------------------------------------------
  SPIFFS API Implementation
  -------------------------------------------------------------------------------*/
  extern "C"
  {
    extern void SPIFFS_fs_lock( void *const fs )
    {
      s_spiffs_lock.lock();
    }


    extern void SPIFFS_fs_unlock( void *const fs )
    {
      s_spiffs_lock.unlock();
    }
  }


  int nor_read( unsigned int addr, unsigned int size, uint8_t *dst )
  {
    using namespace Aurora::Memory;
    using namespace Aurora::Flash::NOR;

    return ( Status::ERR_OK == sNORFlash.read( addr, dst, size ) ) ? SPIFFS_OK : -1;
  }


  int nor_write( unsigned int addr, unsigned int size, uint8_t *src )
  {
    using namespace Aurora::Memory;
    using namespace Aurora::Flash::NOR;

    return ( Status::ERR_OK == sNORFlash.write( addr, src, size ) ) ? SPIFFS_OK : -1;
  }


  int nor_erase( unsigned int addr, unsigned int size )
  {
    using namespace Aurora::Memory;
    using namespace Aurora::Flash::NOR;

    return ( Status::ERR_OK == sNORFlash.erase( addr, size ) ) ? SPIFFS_OK : -1;
  }

}  // namespace Aurora::FileSystem::SPIFFS
