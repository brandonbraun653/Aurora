/********************************************************************************
 *  File Name:
 *    file_driver.cpp
 *
 *  Description:
 *    Driver level implementation
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* STL Includes */
#include <cstdint>

/* Aurora Include */
#include <Aurora/memory>
#include <Aurora/filesystem>
#include <Aurora/logging>
#include <Aurora/source/filesystem/file_driver.hpp>
#include <Aurora/source/filesystem/file_config.hpp>

/* Chimera Includes */
#include <Chimera/assert>
#include <Chimera/thread>

/* ETL Includes */
#include <etl/flat_map.h>

/*-------------------------------------------------------------------------------
Static Data
-------------------------------------------------------------------------------*/
static bool s_initialized                          = false;   /**< Is the module initialized? */
static lfs_t *s_fs                                 = nullptr; /**< External LFS control block */
static const lfs_config *s_fs_cfg                  = nullptr; /**< External LFS memory config */
static const Aurora::Memory::Properties *sNORProps = nullptr; /**< External NOR flash properties */
static Aurora::Flash::NOR::Driver sNORFlash;                  /**< Flash memory driver supporting the file system */

static Chimera::Thread::RecursiveMutex s_file_mtx;
static etl::flat_map<const char *, lfs_file_t, 15> s_open_files;

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


namespace Aurora::FileSystem::LFS
{
  /*-------------------------------------------------------------------------------
  Driver Specific Implementation
  -------------------------------------------------------------------------------*/
  bool attachFS( lfs_t *const fs, const lfs_config *const cfg )
  {
    /*-------------------------------------------------
    Input Protection
    -------------------------------------------------*/
    if ( !fs || !cfg )
    {
      return false;
    }

    /*-------------------------------------------------
    Cache the pointers. This assumes the given data is
    never destroyed.
    -------------------------------------------------*/
    s_fs     = fs;
    s_fs_cfg = cfg;
    s_open_files.clear();
    return true;
  }


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


  std::string_view err2str( const int error )
  {
    return "INVALID";
  }


  static int mount()
  {
    using namespace Aurora::Logging;

    /*-------------------------------------------------
    Input Protection
    -------------------------------------------------*/
    RT_HARD_ASSERT( s_fs );
    RT_HARD_ASSERT( s_fs_cfg );
    RT_HARD_ASSERT( sNORProps );

    /*-------------------------------------------------
    Try mounting. It's possible to get a clean chip,
    which will need some formatting before mounting.
    -------------------------------------------------*/
    int err = lfs_mount( s_fs, s_fs_cfg );
    if ( err )
    {
      /*-------------------------------------------------
      Attempt to quick format
      -------------------------------------------------*/
      getRootSink()->flog( Level::LVL_DEBUG, "Initial mount failed with code %d. Reformatting.\r\n", err );
      err = lfs_format( s_fs, s_fs_cfg );
      if ( err )
      {
        getRootSink()->flog( Level::LVL_DEBUG, "Reformatting failed with code %d\r\n", err );
      }

      /*-------------------------------------------------
      Retry the mount
      -------------------------------------------------*/
      err = lfs_mount( s_fs, s_fs_cfg );
    }

    /*-------------------------------------------------
    Log the mount status for posterity
    -------------------------------------------------*/
    if ( err )
    {
      getRootSink()->flog( Level::LVL_DEBUG, "Mount failed with code %d\r\n", err );
    }
    else
    {
      getRootSink()->flog( Level::LVL_DEBUG, "File system mounted\r\n" );
    }

    return err == 0;
  }


  static int unmount()
  {
    return lfs_unmount( s_fs );
  }


  static FileHandle fopen( const char *filename, const char *mode )
  {
    using namespace Aurora::Logging;
    using namespace Chimera::Thread;

    LockGuard<RecursiveMutex> lk( s_file_mtx );

    /*-------------------------------------------------
    Does the file exist already in the open list?
    -------------------------------------------------*/
    auto iter = s_open_files.find( filename );
    if ( iter != s_open_files.end() )
    {
      return &iter->second;
    }

    /*-------------------------------------------------
    Doesn't exist, but can it be added?
    -------------------------------------------------*/
    if ( s_open_files.full() )
    {
      return nullptr;
    }

    /*-------------------------------------------------
    Figure out the mode to open with
    -------------------------------------------------*/
    int flag = 0;
    if ( ( strcmp( mode, "w" ) == 0 ) || ( strcmp( mode, "wb" ) == 0 ) )
    {
      flag = LFS_O_WRONLY | LFS_O_CREAT;
    }
    else if ( ( strcmp( mode, "w+" ) == 0 ) || ( strcmp( mode, "wb+" ) == 0 ) )
    {
      flag = LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND;
    }
    else if ( ( strcmp( mode, "r" ) == 0 ) || ( strcmp( mode, "rb" ) == 0 ) )
    {
      flag = LFS_O_RDONLY;
    }
    else
    {
      // Some mode combination was used that's not supported
      RT_HARD_ASSERT( false );
    }

    /*-------------------------------------------------
    Try to open the file
    -------------------------------------------------*/
    s_open_files.insert( { filename, {} } );
    iter = s_open_files.find( filename );

    int err = lfs_file_open( s_fs, &iter->second, filename, flag );
    if ( err )
    {
      s_open_files.erase( iter );
      getRootSink()->flog( Level::LVL_DEBUG, "Failed to open %s with code %d\r\n", filename, err );
      return nullptr;
    }

    return &iter->second;
  }


  static int fclose( FileHandle stream )
  {
    using namespace Aurora::Logging;
    using namespace Chimera::Thread;

    LockGuard<RecursiveMutex> lk( s_file_mtx );
    int err = 0;
    for ( auto iter = s_open_files.begin(); iter != s_open_files.end(); iter++ )
    {
      if ( &iter->second == stream )
      {
        err = lfs_file_close( s_fs, &iter->second );
        if ( err )
        {
          getRootSink()->flog( Level::LVL_DEBUG, "Failed to close %s with code %d\r\n", iter->first, err );
        }
        else
        {
          s_open_files.erase( iter );
        }
        break;
      }
    }

    return err;
  }


  static int fflush( FileHandle stream )
  {
    return lfs_file_sync( s_fs, reinterpret_cast<lfs_file_t *>( stream ) );
  }


  static size_t fread( void *ptr, size_t size, size_t count, FileHandle stream )
  {
    return lfs_file_read( s_fs, reinterpret_cast<lfs_file_t *>( stream ), ptr, size * count );
  }


  static size_t fwrite( const void *ptr, size_t size, size_t count, FileHandle stream )
  {
    return lfs_file_write( s_fs, reinterpret_cast<lfs_file_t *>( stream ), ptr, size * count );
  }


  static int fseek( FileHandle stream, size_t offset, size_t origin )
  {
    return lfs_file_seek( s_fs, reinterpret_cast<lfs_file_t *>( stream ), offset, origin );
  }


  static size_t ftell( FileHandle stream )
  {
    return lfs_file_tell( s_fs, reinterpret_cast<lfs_file_t *>( stream ) );
  }


  static void frewind( FileHandle stream )
  {
    lfs_file_rewind( s_fs, reinterpret_cast<lfs_file_t *>( stream ) );
  }

  /*-------------------------------------------------------------------------------
  Public Data
  -------------------------------------------------------------------------------*/
  extern const Interface implementation = { .mount   = ::Aurora::FileSystem::LFS::mount,
                                            .unmount = ::Aurora::FileSystem::LFS::unmount,
                                            .fopen   = ::Aurora::FileSystem::LFS::fopen,
                                            .fclose  = ::Aurora::FileSystem::LFS::fclose,
                                            .fflush  = ::Aurora::FileSystem::LFS::fflush,
                                            .fread   = ::Aurora::FileSystem::LFS::fread,
                                            .fwrite  = ::Aurora::FileSystem::LFS::fwrite,
                                            .fseek   = ::Aurora::FileSystem::LFS::fseek,
                                            .ftell   = ::Aurora::FileSystem::LFS::ftell,
                                            .frewind = ::Aurora::FileSystem::LFS::frewind };
}  // namespace Aurora::FileSystem::LFS
