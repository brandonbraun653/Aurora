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
  if ( auto sts = sNORFlash.read( block, off, buffer, size ); sts != Status::ERR_OK )
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
  if ( auto sts = sNORFlash.write( block, off, buffer, size ); sts != Status::ERR_OK )
  {
    error = LFS_ERR_IO;
  }

  /*-------------------------------------------------
  Wait for the write to complete
  -------------------------------------------------*/
  sNORFlash.pendEvent( Event::MEM_WRITE_COMPLETE, Chimera::Threading::TIMEOUT_BLOCK );
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
  if ( auto sts = sNORFlash.erase( block ); sts != Status::ERR_OK )
  {
    error = LFS_ERR_IO;
  }

  /*-------------------------------------------------
  Wait for the erase to complete
  -------------------------------------------------*/
  sNORFlash.pendEvent( Event::MEM_ERASE_COMPLETE, Chimera::Threading::TIMEOUT_BLOCK );
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
  bool attachDevice( const Aurora::Flash::NOR::Chip_t dev, const Chimera::SPI::Channel channel, lfs_config &cfg )
  {
    return sNORFlash.configure( dev, channel );
  }
}  // namespace Aurora::Memory::LFS
