/********************************************************************************
 *  File Name:
 *    file_driver.cpp
 *
 *  Description:
 *    Implements filesystem operations in terms of the LittleFS library.
 *
 *  2021-2022 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/filesystem>
#include <Aurora/logging>
#include <Aurora/memory>
#include <Chimera/assert>
#include <Chimera/thread>
#include <cstdint>
#include <etl/flat_map.h>

/*-------------------------------------------------------------------------------
Static Data
-------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------
Public Functions
-------------------------------------------------------------------------------*/
#if defined( SIMULATOR )
static int lfs_safe_read( const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size )
{
  return -1;
}


static int lfs_safe_prog( const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size )
{
  return -1;
}


static int lfs_safe_erase( const struct lfs_config *c, lfs_block_t block )
{
  return -1;
}


static int lfs_safe_sync( const struct lfs_config *c )
{
  return -1;
}

#else /* EMBEDDED */
static int lfs_safe_read( const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size )
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


static int lfs_safe_prog( const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size )
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


static int lfs_safe_erase( const struct lfs_config *c, lfs_block_t block )
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


static int lfs_safe_sync( const struct lfs_config *c )
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

#endif /* EMBEDDED */


namespace Aurora::FileSystem::LFS
{
  /*---------------------------------------------------------------------------
  LittleFS Specific Implementation
  ---------------------------------------------------------------------------*/
  bool attachFS( const std::string_view &drive, lfs_t *const fs, const lfs_config *const cfg )
  {
  }


  /*---------------------------------------------------------------------------
  Static Functions
  ---------------------------------------------------------------------------*/
  static int initialize()
  {
    return -1;
  }


  static int mount( const std::string_view &drive )
  {
    return -1;
  }


  static int unmount()
  {
    return -1;
  }


  static int fopen( const char *filename, const AccessFlags mode, FileId &file )
  {
    return -1;
  }


  static int fclose( FileId stream )
  {
    return -1;
  }


  static int fflush( FileId stream )
  {
    return -1;
  }


  static size_t fread( void *ptr, size_t size, size_t count, FileId stream )
  {
    return 0;
  }


  static size_t fwrite( const void *ptr, size_t size, size_t count, FileId stream )
  {
    return 0;
  }


  static int fseek( FileId stream, size_t offset, size_t origin )
  {
    return -1;
  }


  static size_t ftell( FileId stream )
  {
    return 0;
  }


  static void frewind( FileId stream )
  {
  }


  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/
  Interface getInterface()
  {
    Interface intf;

    intf.mount   = ::Aurora::FileSystem::LFS::mount;
    intf.unmount = ::Aurora::FileSystem::LFS::unmount;
    intf.fopen   = ::Aurora::FileSystem::LFS::fopen;
    intf.fclose  = ::Aurora::FileSystem::LFS::fclose;
    intf.fflush  = ::Aurora::FileSystem::LFS::fflush;
    intf.fread   = ::Aurora::FileSystem::LFS::fread;
    intf.fwrite  = ::Aurora::FileSystem::LFS::fwrite;
    intf.fseek   = ::Aurora::FileSystem::LFS::fseek;
    intf.ftell   = ::Aurora::FileSystem::LFS::ftell;
    intf.frewind = ::Aurora::FileSystem::LFS::frewind;

    return intf;
  }

}  // namespace Aurora::FileSystem::LFS
