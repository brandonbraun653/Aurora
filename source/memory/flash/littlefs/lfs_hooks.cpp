/********************************************************************************
 *  File Name:
 *    lfs_hooks.cpp
 *
 *  Description:
 *    Implementation of hooks for a generic memory device with LittleFS
 *
 *  2020-2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* STL Includes */
#include <array>

/* LFS Includes */
#include "lfs.h"

/* Aurora Includes */
#include <Aurora/memory>
#include <Aurora/source/memory/flash/littlefs/lfs_hooks.hpp>

/*-------------------------------------------------------------------------------
Static Data
-------------------------------------------------------------------------------*/
/**
 *  Most projects really only have one file system, so don't bother with
 *  multiple device support unless it's actually needed.
 */
static Aurora::Flash::NOR::Driver sNORFlash;
static const Aurora::Memory::Properties *sNORProps;


/*-------------------------------------------------------------------------------
Public Functions
-------------------------------------------------------------------------------*/
int lfs_safe_read( const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size )
{
  using namespace Aurora::Memory;

  /*-------------------------------------------------
  Input protection
  -------------------------------------------------*/
  if ( !c )
  {
    return LFS_ERR_INVAL;
  }

  /*-------------------------------------------------
  Figure out the real address & invoke the device driver
  -------------------------------------------------*/
  sNORFlash.lock();

  auto error = LFS_ERR_OK;
  if ( Status::ERR_OK != sNORFlash.read( block, off, buffer, size ) )
  {
    error = LFS_ERR_IO;
  }

  sNORFlash.unlock();
  return error;
}


int lfs_safe_prog( const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size )
{
  using namespace Aurora::Memory;

  /*-------------------------------------------------
  Input protection
  -------------------------------------------------*/
  if ( !c )
  {
    return LFS_ERR_INVAL;
  }

  /*-------------------------------------------------
  Figure out the real address & invoke the device driver
  -------------------------------------------------*/
  sNORFlash.lock();

  auto error = LFS_ERR_OK;
  if ( ( Status::ERR_OK != sNORFlash.write( block, off, buffer, size ) )
       || ( Status::ERR_OK != sNORFlash.pendEvent( Event::MEM_WRITE_COMPLETE, sNORProps->pagePgmDelay ) ) )
  {
    error = LFS_ERR_IO;
  }

  sNORFlash.unlock();
  return error;
}


int lfs_safe_erase( const struct lfs_config *c, lfs_block_t block )
{
  using namespace Aurora::Memory;

  /*-------------------------------------------------
  Input protection
  -------------------------------------------------*/
  if ( !c )
  {
    return LFS_ERR_INVAL;
  }

  /*-------------------------------------------------
  Invoke the device driver
  -------------------------------------------------*/
  sNORFlash.lock();

  auto error = LFS_ERR_OK;
  if ( ( Status::ERR_OK != sNORFlash.erase( block ) )
       || ( Status::ERR_OK != sNORFlash.pendEvent( Event::MEM_ERASE_COMPLETE, sNORProps->blockEraseDelay ) ) )
  {
    error = LFS_ERR_IO;
  }

  sNORFlash.unlock();
  return error;
}


int lfs_safe_sync( const struct lfs_config *c )
{
  using namespace Aurora::Memory;

  /*-------------------------------------------------
  Input protection
  -------------------------------------------------*/
  if ( !c )
  {
    return LFS_ERR_INVAL;
  }

  /*-------------------------------------------------
  Invoke the device driver
  -------------------------------------------------*/
  sNORFlash.lock();

  auto error = LFS_ERR_OK;
  if ( auto sts = sNORFlash.flush(); sts != Status::ERR_OK )
  {
    error = LFS_ERR_IO;
  }

  sNORFlash.unlock();
  return error;
}


namespace Aurora::Memory::LFS
{
  bool attachDevice( const Aurora::Flash::NOR::Chip_t dev, const Chimera::SPI::Channel channel, const lfs_config &cfg )
  {
    sNORProps = Aurora::Flash::NOR::getProperties( dev );
    return sNORFlash.configure( dev, channel );
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
}  // namespace Aurora::Memory::LFS
