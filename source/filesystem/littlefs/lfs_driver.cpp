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
#include <Aurora/constants>
#include <Aurora/filesystem>
#include <Aurora/logging>
#include <Aurora/memory>
#include <Chimera/assert>
#include <Chimera/common>
#include <Chimera/thread>
#include <cstdint>
#include <etl/algorithm.h>
#include <etl/flat_map.h>
#include <etl/vector.h>

#if defined( SIMULATOR )
#include <assert.h>
#include <fstream>
#include <iostream>
#endif

namespace Aurora::FileSystem::LFS
{
  /*---------------------------------------------------------------------------
  Assertions
  ---------------------------------------------------------------------------*/
  static_assert( EnumValue( F_SEEK_SET ) == LFS_SEEK_SET );
  static_assert( EnumValue( F_SEEK_CUR ) == LFS_SEEK_CUR );
  static_assert( EnumValue( F_SEEK_END ) == LFS_SEEK_END );

  /*---------------------------------------------------------------------------
  Structures
  ---------------------------------------------------------------------------*/
  struct File
  {
    FileId     fileDesc;
    Volume *   pVolume;
    lfs_file_t lfsFile;

    bool operator<( const File &rhs ) const
    {
      return fileDesc < rhs.fileDesc;
    }

    inline void clear()
    {
      fileDesc = -1;
      pVolume  = nullptr;
      memset( &lfsFile, 0, sizeof( lfsFile ) );
    }
  };

  /*---------------------------------------------------------------------------
  Static Data
  ---------------------------------------------------------------------------*/
  static uint32_t                           s_init;    /**< Initialized key */
  static Chimera::Thread::RecursiveMutex    s_lock;    /**< Module lock */
  static etl::vector<Volume *, MAX_VOLUMES> s_volumes; /**< Registered volumes */
  static etl::vector<File, MAX_OPEN_FILES>  s_files;   /**< Currently open files */


  /*---------------------------------------------------------------------------
  Static Functions
  ---------------------------------------------------------------------------*/
  /**
   * @brief Finds the LFS volume structure from an ID
   *
   * @param id    Which ID is associated with the volume
   * @return Volume*
   */
  static Volume *get_volume( VolumeId id )
  {
    for( Volume* iter : s_volumes )
    {
      if( iter->_volumeID == id )
      {
        return iter;
      }
    }

    return nullptr;
  }

  /**
   * @brief Finds the LFS file structure from an ID
   *
   * @param id    Which ID is associated with the file
   * @return File*
   */
  static File *get_file( FileId stream )
  {
    for( File &f : s_files )
    {
      if( f.fileDesc == stream )
      {
        return &f;
      }
    }

    return nullptr;
  }


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


  static int fs_init()
  {
    LFS::initialize();
    return 0;
  }


  static int mount( const VolumeId drive, void *context )
  {
    Chimera::Thread::LockGuard _lck( s_lock );

    /*-------------------------------------------------------------------------
    Update the drive number for the Volume object passed in. If the drive is
    already registered, this will reveal itself in the "get_volume" call.
    -------------------------------------------------------------------------*/
    RT_HARD_ASSERT( context );
    Volume *vol    = reinterpret_cast<Volume *>( context );
    vol->_volumeID = drive;

    vol = get_volume( drive );
    if( !vol )
    {
      return -1;
    }

    /*-------------------------------------------------------------------------
    Ensure the backing file exists with the expected properties
    -------------------------------------------------------------------------*/
#if defined( SIMULATOR )
    auto props = Aurora::Flash::NOR::getProperties( vol->flash.deviceType() );
    assert( props );

    if( std::filesystem::exists( vol->_dataFile ) )
    {
      const size_t file_size = std::filesystem::file_size( vol->_dataFile );
      assert( file_size == props->endAddress );
    }
    else
    {
      /*-----------------------------------------------------------------------
      Create the parent directories
      -----------------------------------------------------------------------*/
      auto parent_dir = vol->_dataFile.parent_path();
      std::filesystem::create_directories( parent_dir );
      assert( std::filesystem::exists( parent_dir ) );

      /*-----------------------------------------------------------------------
      Fill the file with empty NOR memory all the way up to the device size
      -----------------------------------------------------------------------*/
      std::ofstream file;
      file.open( vol->_dataFile, std::ios::app | std::ios::binary );


      uint8_t data = 0xff;
      for ( size_t idx = 0; idx < props->endAddress; idx++ )
      {
        file.write( reinterpret_cast<const char *>( &data ), sizeof( data ) );
      }

      file.close();
    }
#endif  /* SIMULATOR */

    /*-------------------------------------------------------------------------
    Otherwise, attempt to mount the drive assuming it's already formatted.
    -------------------------------------------------------------------------*/
    return lfs_mount( &( vol->fs ), &( vol->cfg ) );
  }


  static int unmount( const VolumeId drive )
  {
    Chimera::Thread::LockGuard _lck( s_lock );

    /*-------------------------------------------------------------------------
    Only attempt an unmount if the volume actually exists
    -------------------------------------------------------------------------*/
    Volume *vol = get_volume( drive );
    if( !vol )
    {
      return -1;
    }

    /*-------------------------------------------------------------------------
    Perform the unmount, but don't destroy the volume registration. That is a
    separate (lower layer) task not related to mounting/unmounting.
    -------------------------------------------------------------------------*/
    return lfs_unmount( &( vol->fs ) );
  }


  static int fopen( const char *filename, const AccessFlags mode, const FileId stream, const VolumeId vol )
  {
    Chimera::Thread::LockGuard _lck( s_lock );

    /*-------------------------------------------------------------------------
    Input Protections
    -------------------------------------------------------------------------*/
    if( s_files.full() )
    {
      return -1;
    }

    /*-------------------------------------------------------------------------
    Nothing to do if the file already exists in the registry
    -------------------------------------------------------------------------*/
    File *file = get_file( stream );
    if( file )
    {
      return 0;
    }

    Volume *volume = get_volume( vol );
    RT_DBG_ASSERT( volume );

    /*-------------------------------------------------------------------------
    Translate the mode flags to the LFS flags
    -------------------------------------------------------------------------*/
    int      flags    = 0;
    uint32_t access   = mode & O_ACCESS_MSK;
    uint32_t modifier = mode & O_MODIFY_MSK;

    switch( access )
    {
      case O_RDONLY:
        flags = LFS_O_RDONLY;
        break;

      case O_WRONLY:
        flags = LFS_O_WRONLY;
        break;

      case O_RDWR:
        flags = LFS_O_RDWR;
        break;

      default:
        return -1;
    }

    if( modifier & O_APPEND )
    {
      flags |= LFS_O_APPEND;
    }

    if( modifier & O_CREAT )
    {
      flags |= LFS_O_CREAT;
    }

    if( modifier & O_EXCL )
    {
      flags |= LFS_O_EXCL;
    }

    if( modifier & O_TRUNC )
    {
      flags |= LFS_O_TRUNC;
    }

    /*-------------------------------------------------------------------------
    Open the new file
    -------------------------------------------------------------------------*/
    File new_file;
    new_file.clear();

    int err = lfs_file_open( &( volume->fs ), &new_file.lfsFile, filename, flags );
    if( err == LFS_ERR_OK )
    {
      new_file.fileDesc = stream;
      new_file.pVolume  = volume;

      s_files.push_back( new_file );
      etl::shell_sort( s_files.begin(), s_files.end() );
    }

    return err;
  }


  static int fclose( FileId stream )
  {
    Chimera::Thread::LockGuard _lck( s_lock );

    /*-------------------------------------------------------------------------
    Look up the file in the registry
    -------------------------------------------------------------------------*/
    File *file = get_file( stream );
    if( !file )
    {
      return 0;
    }

    /*-------------------------------------------------------------------------
    Perform the LFS operation
    -------------------------------------------------------------------------*/
    int err = lfs_file_close( &( file->pVolume->fs ), &( file->lfsFile ) );
    if( err < 0 )
    {
      return err;
    }

    /*-------------------------------------------------------------------------
    Remove the file from the registry
    -------------------------------------------------------------------------*/
    s_files.erase( file );
    etl::shell_sort( s_files.begin(), s_files.end() );
    return 0;
  }


  static int fflush( FileId stream )
  {
    /*-------------------------------------------------------------------------
    Look up the file in the registry
    -------------------------------------------------------------------------*/
    File *file = get_file( stream );
    if( !file )
    {
      return 0;
    }

    /*-------------------------------------------------------------------------
    Perform the LFS operation
    -------------------------------------------------------------------------*/
    return lfs_file_sync( &( file->pVolume->fs ), &( file->lfsFile ) );
  }


  static size_t fread( void *ptr, size_t size, size_t count, FileId stream )
  {
    /*-------------------------------------------------------------------------
    Look up the file in the registry
    -------------------------------------------------------------------------*/
    File *file = get_file( stream );
    if( !file )
    {
      return 0;
    }

    /*-------------------------------------------------------------------------
    Perform the LFS operation
    -------------------------------------------------------------------------*/
    int bytes_read = lfs_file_read( &( file->pVolume->fs ), &( file->lfsFile ), ptr, ( size * count ) );
    if( bytes_read < 0 )
    {
      return 0;
    }

    return static_cast<size_t>( bytes_read );
  }


  static size_t fwrite( const void *ptr, size_t size, size_t count, FileId stream )
  {
    /*-------------------------------------------------------------------------
    Look up the file in the registry
    -------------------------------------------------------------------------*/
    File *file = get_file( stream );
    if( !file )
    {
      return 0;
    }

    /*-------------------------------------------------------------------------
    Perform the LFS operation
    -------------------------------------------------------------------------*/
    int bytes_written = lfs_file_write( &( file->pVolume->fs ), &( file->lfsFile ), ptr, ( size * count ) );
    if( bytes_written < 0 )
    {
      return 0;
    }

    return static_cast<size_t>( bytes_written );
  }


  static int fseek( const FileId stream, const size_t offset, const WhenceFlags whence )
  {
    /*-------------------------------------------------------------------------
    Look up the file in the registry
    -------------------------------------------------------------------------*/
    File *file = get_file( stream );
    if( !file )
    {
      return 0;
    }

    /*-------------------------------------------------------------------------
    Perform the LFS operation
    -------------------------------------------------------------------------*/
    return lfs_file_seek( &( file->pVolume->fs ), &( file->lfsFile ), offset, whence );
  }


  static size_t ftell( FileId stream )
  {
    /*-------------------------------------------------------------------------
    Look up the file in the registry
    -------------------------------------------------------------------------*/
    File *file = get_file( stream );
    if( !file )
    {
      return 0;
    }

    /*-------------------------------------------------------------------------
    Perform the LFS operation
    -------------------------------------------------------------------------*/
    int err = lfs_file_tell( &( file->pVolume->fs ), &( file->lfsFile ) );
    if( err < 0 )
    {
      return 0;
    }

    return static_cast<size_t>( err );
  }


  static void frewind( FileId stream )
  {
    /*-------------------------------------------------------------------------
    Look up the file in the registry
    -------------------------------------------------------------------------*/
    File *file = get_file( stream );
    if( !file )
    {
      return;
    }

    /*-------------------------------------------------------------------------
    Perform the LFS operation
    -------------------------------------------------------------------------*/
    int err = lfs_file_rewind( &( file->pVolume->fs ), &( file->lfsFile ) );
    RT_DBG_ASSERT( err == LFS_ERR_OK );
  }


  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/
  Interface getInterface()
  {
    Interface intf;
    intf.clear();

    intf.initialize = ::Aurora::FileSystem::LFS::fs_init;
    intf.mount      = ::Aurora::FileSystem::LFS::mount;
    intf.unmount    = ::Aurora::FileSystem::LFS::unmount;
    intf.fopen      = ::Aurora::FileSystem::LFS::fopen;
    intf.fclose     = ::Aurora::FileSystem::LFS::fclose;
    intf.fflush     = ::Aurora::FileSystem::LFS::fflush;
    intf.fread      = ::Aurora::FileSystem::LFS::fread;
    intf.fwrite     = ::Aurora::FileSystem::LFS::fwrite;
    intf.fseek      = ::Aurora::FileSystem::LFS::fseek;
    intf.ftell      = ::Aurora::FileSystem::LFS::ftell;
    intf.frewind    = ::Aurora::FileSystem::LFS::frewind;

    return intf;
  }


  void initialize()
  {
    if ( s_init != Chimera::DRIVER_INITIALIZED_KEY )
    {
      s_lock.unlock();
      s_volumes.clear();
      s_files.clear();
      s_init = Chimera::DRIVER_INITIALIZED_KEY;
    }
  }


  bool attachVolume( Volume *const vol )
  {
    /*-------------------------------------------------------------------------
    Input Protection
    -------------------------------------------------------------------------*/
    if ( !vol || vol->cfg.read || vol->cfg.prog || vol->cfg.erase || vol->cfg.sync )
    {
      return false;
    }

    /*-------------------------------------------------------------------------
    Ensure we can store the new volume and it hasn't already been registered.
    -------------------------------------------------------------------------*/
    if( s_volumes.full() || get_volume( vol->_volumeID ) )
    {
      return false;
    }

    /*-------------------------------------------------------------------------
    Register the IO callbacks & context pointer
    -------------------------------------------------------------------------*/
    vol->cfg.read  = lfs_safe_read;
    vol->cfg.prog  = lfs_safe_prog;
    vol->cfg.erase = lfs_safe_erase;
    vol->cfg.sync  = lfs_safe_sync;
    vol->cfg.context = reinterpret_cast<void *>( vol );

    s_volumes.push_back( vol );
    return true;
  }


  bool formatVolume( Volume *const vol )
  {
    /*-------------------------------------------------------------------------
    Input Protection
    -------------------------------------------------------------------------*/
    if( !vol )
    {
      return false;
    }

    /*-------------------------------------------------------------------------
    Invoke the format command, assuming the configuration is OK
    -------------------------------------------------------------------------*/
    return LFS_ERR_OK == lfs_format( &( vol->fs ), &( vol->cfg ) );
  }
}  // namespace Aurora::FileSystem::LFS
