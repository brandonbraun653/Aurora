/********************************************************************************
 *  File Name:
 *    lfs_hooks.cpp
 *
 *  Description:
 *    Implementation of hooks for a generic memory device with LittleFS
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* STL Includes */
#include <array>

/* LFS Includes */
#include "lfs.h"

/* Aurora Includes */
#include <Aurora/memory>
#include <Aurora/src/memory/flash/littlefs/lfs_hooks.hpp>

/*-------------------------------------------------------------------------------
Aliases
-------------------------------------------------------------------------------*/
using DriverIndex_t = size_t;


/*-------------------------------------------------------------------------------
Static Data
-------------------------------------------------------------------------------*/
static DriverIndex_t DRIVER_INDEX_0 = 0;
static DriverIndex_t DRIVER_INDEX_1 = 1;
static DriverIndex_t DRIVER_INDEX_2 = 2;
static std::array<Aurora::Memory::IGenericDevice_sPtr, 3> s_mem_drivers;


/*-------------------------------------------------------------------------------
Public Functions
-------------------------------------------------------------------------------*/
int lfs_safe_read( const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size )
{
  using namespace Aurora::Memory;

  /*-------------------------------------------------
  Input protection
  -------------------------------------------------*/
  if ( !c || !c->context )
  {
    return LFS_ERR_INVAL;
  }

  /*-------------------------------------------------
  Ensure the device actually exists
  -------------------------------------------------*/
  DriverIndex_t idx = *( reinterpret_cast<DriverIndex_t *>( c->context ) );
  if ( !( idx < s_mem_drivers.size() ) )
  {
    return LFS_ERR_INVAL;
  }

  /*-------------------------------------------------
  Figure out the real address & invoke the device driver
  -------------------------------------------------*/
  s_mem_drivers[ idx ]->lock();

  auto error           = LFS_ERR_OK;
  auto properties      = s_mem_drivers[ idx ]->getDeviceProperties();
  size_t chunk_address = chunkStartAddress( properties, properties.writeChunk, block );
  size_t address       = chunk_address + off;

  if ( auto sts = s_mem_drivers[ idx ]->read( address, buffer, size ); sts != Status::ERR_OK )
  {
    error = LFS_ERR_IO;
  }

  s_mem_drivers[ idx ]->unlock();
  return error;
}


int lfs_safe_prog( const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size )
{
  using namespace Aurora::Memory;

  /*-------------------------------------------------
  Input protection
  -------------------------------------------------*/
  if ( !c || !c->context )
  {
    return LFS_ERR_INVAL;
  }

  /*-------------------------------------------------
  Ensure the device actually exists
  -------------------------------------------------*/
  DriverIndex_t idx = *( reinterpret_cast<DriverIndex_t *>( c->context ) );
  if ( !( idx < s_mem_drivers.size() ) )
  {
    return LFS_ERR_INVAL;
  }

  /*-------------------------------------------------
  Figure out the real address & invoke the device driver
  -------------------------------------------------*/
  s_mem_drivers[ idx ]->lock();

  auto error           = LFS_ERR_OK;
  auto properties      = s_mem_drivers[ idx ]->getDeviceProperties();
  size_t chunk_address = chunkStartAddress( properties, properties.readChunk, block );
  size_t address       = chunk_address + off;

  if ( auto sts = s_mem_drivers[ idx ]->write( address, buffer, size ); sts != Status::ERR_OK )
  {
    error = LFS_ERR_IO;
  }

  /*-------------------------------------------------
  Wait for the write to complete
  -------------------------------------------------*/
  s_mem_drivers[ idx ]->pendEvent( Event::MEM_WRITE_COMPLETE, Chimera::Threading::TIMEOUT_BLOCK );
  s_mem_drivers[ idx ]->unlock();
  return error;
}


int lfs_safe_erase( const struct lfs_config *c, lfs_block_t block )
{
  using namespace Aurora::Memory;

  /*-------------------------------------------------
  Input protection
  -------------------------------------------------*/
  if ( !c || !c->context )
  {
    return LFS_ERR_INVAL;
  }

  /*-------------------------------------------------
  Ensure the device actually exists
  -------------------------------------------------*/
  DriverIndex_t idx = *( reinterpret_cast<DriverIndex_t *>( c->context ) );
  if ( !( idx < s_mem_drivers.size() ) )
  {
    return LFS_ERR_INVAL;
  }

  /*-------------------------------------------------
  Invoke the device driver
  -------------------------------------------------*/
  s_mem_drivers[ idx ]->lock();

  auto error      = LFS_ERR_OK;
  auto properties = s_mem_drivers[ idx ]->getDeviceProperties();
  if ( auto sts = s_mem_drivers[ idx ]->erase( properties.eraseChunk, block ); sts != Status::ERR_OK )
  {
    error = LFS_ERR_IO;
  }

  /*-------------------------------------------------
  Wait for the erase to complete
  -------------------------------------------------*/
  s_mem_drivers[ idx ]->pendEvent( Event::MEM_ERASE_COMPLETE, Chimera::Threading::TIMEOUT_BLOCK );
  s_mem_drivers[ idx ]->unlock();
  return error;
}


int lfs_safe_sync( const struct lfs_config *c )
{
  using namespace Aurora::Memory;

  /*-------------------------------------------------
  Input protection
  -------------------------------------------------*/
  if ( !c || !c->context )
  {
    return LFS_ERR_INVAL;
  }

  /*-------------------------------------------------
  Ensure the device actually exists
  -------------------------------------------------*/
  DriverIndex_t idx = *( reinterpret_cast<DriverIndex_t *>( c->context ) );
  if ( !( idx < s_mem_drivers.size() ) )
  {
    return LFS_ERR_INVAL;
  }

  /*-------------------------------------------------
  Invoke the device driver
  -------------------------------------------------*/
  s_mem_drivers[ idx ]->lock();

  auto error = LFS_ERR_OK;
  if ( auto sts = s_mem_drivers[ idx ]->flush(); sts != Status::ERR_OK )
  {
    error = LFS_ERR_IO;
  }

  s_mem_drivers[ idx ]->unlock();
  return error;
}


namespace Aurora::Memory::LFS
{
  bool attachDevice( IGenericDevice_sPtr dev, lfs_config &cfg )
  {
    /*-------------------------------------------------
    Input protection
    -------------------------------------------------*/
    if ( !dev )
    {
      return false;
    }

    /*-------------------------------------------------
    Iterate over the registration list and find the
    next empty spot.
    -------------------------------------------------*/
    for ( DriverIndex_t x = 0; x < s_mem_drivers.size(); x++ )
    {
      if ( !s_mem_drivers[ x ] )
      {
        s_mem_drivers[ x ] = dev;

        switch ( x )
        {
          case 0:
            cfg.context = &DRIVER_INDEX_0;
            break;

          case 1:
            cfg.context = &DRIVER_INDEX_1;
            break;

          case 2:
            cfg.context = &DRIVER_INDEX_2;
            break;

          default:
            return false;
            break;
        }

        return true;
      }
    }

    /*-------------------------------------------------
    If we get here, there is no available slot
    -------------------------------------------------*/
    return false;
  }
}  // namespace Aurora::Memory::LFS
