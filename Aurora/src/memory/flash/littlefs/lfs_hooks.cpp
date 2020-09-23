/********************************************************************************
 *  File Name:
 *    lfs_hooks.cpp
 *
 *  Description:
 *    Implementation of hooks for a generic memory device with LittleFS
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* LFS Includes */
#include "lfs.h"

/* Aurora Includes */
#include <Aurora/memory/flash/littlefs/lfs_hooks.hpp>
#include <Aurora/memory/generic/generic_intf.hpp>
#include <Aurora/memory/generic/generic_types.hpp>
#include <Aurora/memory/generic/generic_utils.hpp>

/*-------------------------------------------------------------------------------
Public Functions
-------------------------------------------------------------------------------*/
int lfs_safe_read( const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size )
{
  using namespace Aurora::Memory;

  /*-------------------------------------------------
  Input protection
  -------------------------------------------------*/
  if ( !c || c->context )
  {
    return LFS_ERR_INVAL;
  }

  /*-------------------------------------------------
  Ensure the device actually exists
  -------------------------------------------------*/
  IGenericDevice_sPtr device = *( reinterpret_cast<IGenericDevice_sPtr *>( c->context ) );
  if ( !device )
  {
    return LFS_ERR_INVAL;
  }

  /*-------------------------------------------------
  Figure out the real address & invoke the device driver
  -------------------------------------------------*/
  device->lock();

  auto error           = LFS_ERR_OK;
  auto properties      = device->getDeviceProperties();
  size_t chunk_address = chunkStartAddress( properties, properties.writeChunk, block );
  size_t address       = chunk_address + off;

  if ( auto sts = device->read( address, buffer, size ); sts != Status::ERR_OK )
  {
    error = LFS_ERR_IO;
  }

  device->unlock();
  return error;
}


int lfs_safe_prog( const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size )
{
  using namespace Aurora::Memory;

  /*-------------------------------------------------
  Input protection
  -------------------------------------------------*/
  if ( !c || c->context )
  {
    return LFS_ERR_INVAL;
  }

  /*-------------------------------------------------
  Ensure the device actually exists
  -------------------------------------------------*/
  IGenericDevice_sPtr device = *( reinterpret_cast<IGenericDevice_sPtr *>( c->context ) );
  if ( !device )
  {
    return LFS_ERR_INVAL;
  }

  /*-------------------------------------------------
  Figure out the real address & invoke the device driver
  -------------------------------------------------*/
  device->lock();

  auto error           = LFS_ERR_OK;
  auto properties      = device->getDeviceProperties();
  size_t chunk_address = chunkStartAddress( properties, properties.readChunk, block );
  size_t address       = chunk_address + off;

  if ( auto sts = device->write( address, buffer, size ); sts != Status::ERR_OK )
  {
    error = LFS_ERR_IO;
  }

  device->unlock();
  return error;
}


int lfs_safe_erase( const struct lfs_config *c, lfs_block_t block )
{
  using namespace Aurora::Memory;

  /*-------------------------------------------------
  Input protection
  -------------------------------------------------*/
  if ( !c || c->context )
  {
    return LFS_ERR_INVAL;
  }

  /*-------------------------------------------------
  Ensure the device actually exists
  -------------------------------------------------*/
  IGenericDevice_sPtr device = *( reinterpret_cast<IGenericDevice_sPtr *>( c->context ) );
  if ( !device )
  {
    return LFS_ERR_INVAL;
  }

  /*-------------------------------------------------
  Invoke the device driver
  -------------------------------------------------*/
  device->lock();

  auto error      = LFS_ERR_OK;
  auto properties = device->getDeviceProperties();
  if ( auto sts = device->erase( properties.eraseChunk, block ); sts != Status::ERR_OK )
  {
    error = LFS_ERR_IO;
  }

  device->unlock();
  return error;
}


int lfs_safe_sync( const struct lfs_config *c )
{
  using namespace Aurora::Memory;

  /*-------------------------------------------------
  Input protection
  -------------------------------------------------*/
  if ( !c || c->context )
  {
    return LFS_ERR_INVAL;
  }

  /*-------------------------------------------------
  Ensure the device actually exists
  -------------------------------------------------*/
  IGenericDevice_sPtr device = *( reinterpret_cast<IGenericDevice_sPtr *>( c->context ) );
  if ( !device )
  {
    return LFS_ERR_INVAL;
  }

  /*-------------------------------------------------
  Invoke the device driver
  -------------------------------------------------*/
  device->lock();

  auto error = LFS_ERR_OK;
  if ( auto sts = device->flush(); sts != Status::ERR_OK )
  {
    error = LFS_ERR_IO;
  }

  device->unlock();
  return error;
}


namespace Aurora::Memory::LFS
{
  bool attachDevice( IGenericDevice_sPtr &dev, lfs_config &cfg )
  {
    if ( dev )
    {
      dev->lock();
      cfg.context = reinterpret_cast<void *>( &dev );
      dev->unlock();

      return true;
    }
    else
    {
      return false;
    }
  }
}  // namespace Aurora::Memory::LFS
