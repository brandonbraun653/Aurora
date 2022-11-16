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
#include <cstdio>
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
  Aliases
  ---------------------------------------------------------------------------*/
  namespace AM = Aurora::Memory;

  /*---------------------------------------------------------------------------
  Structures
  ---------------------------------------------------------------------------*/
  /**
   * @brief Internal representation of an LFS file.
   */
  struct File
  {
    FileId          fileDesc;          /**< Descriptor assigned to this file */
    Volume         *pVolume;           /**< Parent volume file belongs to */
    lfs_file_t      lfsFile;           /**< LFS file control block */
    lfs_file_config lfsCfg;            /**< LFS config data due to not allowing malloc */
    uint8_t         lfsCfgBuff[ 256 ]; /**< Static buffer for config structure */

    bool operator<( const File &rhs ) const
    {
      return fileDesc < rhs.fileDesc;
    }

    inline void clear()
    {
      fileDesc = -1;
      pVolume  = nullptr;
      memset( &lfsFile, 0, sizeof( lfsFile ) );
      memset( &lfsCfg, 0, sizeof( lfsCfg ) );
      memset( lfsCfgBuff, 0, sizeof( lfsCfgBuff ) );
    }
  };

  /*---------------------------------------------------------------------------
  Constants
  ---------------------------------------------------------------------------*/
  static constexpr bool                                 DEBUG_MODULE      = true;
  static const etl::string_view                         s_lfs_unknown_err = "Unknown error";
  static const etl::flat_map<int, etl::string_view, 15> s_lfs_err_to_str  = {
    { LFS_ERR_OK, "No error" },                // No error
    { LFS_ERR_IO, "Device IO error" },         // Error during device operation
    { LFS_ERR_CORRUPT, "Corrupted" },          // Corrupted
    { LFS_ERR_NOENT, "No dir entry" },         // No directory entry
    { LFS_ERR_EXIST, "Entry exists" },         // Entry already exists
    { LFS_ERR_NOTDIR, "Entry not a dir" },     // Entry is not a dir
    { LFS_ERR_ISDIR, "Entry is dir" },         // Entry is a dir
    { LFS_ERR_NOTEMPTY, "Dir not empty" },     // Dir is not empty
    { LFS_ERR_BADF, "Bad file number" },       // Bad file number
    { LFS_ERR_FBIG, "File too large" },        // File too large
    { LFS_ERR_INVAL, "Invalid param" },        // Invalid parameter
    { LFS_ERR_NOSPC, "No space on device" },   // No space left on device
    { LFS_ERR_NOMEM, "No memory" },            // No more memory available
    { LFS_ERR_NOATTR, "No attr available" },   // No data/attr available
    { LFS_ERR_NAMETOOLONG, "Name too long" },  // File name too long
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
    for ( Volume *iter : s_volumes )
    {
      if ( iter->_volumeID == id )
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
    for ( File &f : s_files )
    {
      if ( f.fileDesc == stream )
      {
        return &f;
      }
    }

    return nullptr;
  }


  static etl::string_view get_error_str( const int error )
  {
    auto iter = s_lfs_err_to_str.find( error );
    if ( iter != s_lfs_err_to_str.end() )
    {
      return iter->second;
    }
    else
    {
      return s_lfs_unknown_err;
    }
  }


#if defined( SIMULATOR )
  static int lfs_safe_read( const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size )
  {
    RT_HARD_ASSERT( c->context );
    Volume *vol = reinterpret_cast<Volume *>( c->context );

    /*-------------------------------------------------------------------------
    Calculate the absolute offset required to read from
    -------------------------------------------------------------------------*/
    size_t address = 0;
    if ( !Aurora::Flash::NOR::block2Address( vol->flash.deviceType(), block, &address ) )
    {
      LOG_TRACE( "Bad flash address\r\n" );
      return LFS_ERR_INVAL;
    }

    address += off;

    /*-------------------------------------------------------------------------
    Read from the file
    -------------------------------------------------------------------------*/
    FILE *file = ::fopen( vol->_dataFile.c_str(), "rb" );
    RT_HARD_ASSERT( file );

    ::fseek( file, address, SEEK_SET );
    RT_HARD_ASSERT( ::ftell( file ) == address );

    size_t bytes_read = ::fread( buffer, 1, size, file );
    ::fclose( file );

    if ( bytes_read != size )
    {
      LOG_TRACE( "Error reading from file: %d\r\n" );
      return LFS_ERR_IO;
    }

    return LFS_ERR_OK;
  }


  static int lfs_safe_prog( const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size )
  {
    RT_HARD_ASSERT( c->context );
    Volume *vol = reinterpret_cast<Volume *>( c->context );

    /*-------------------------------------------------------------------------
    Calculate the absolute offset required to read from
    -------------------------------------------------------------------------*/
    size_t address = 0;
    if ( !Aurora::Flash::NOR::block2Address( vol->flash.deviceType(), block, &address ) )
    {
      LOG_TRACE( "Bad flash address\r\n" );
      return LFS_ERR_INVAL;
    }

    address += off;

    /*-------------------------------------------------------------------------
    Write the file
    -------------------------------------------------------------------------*/
    FILE *file = ::fopen( vol->_dataFile.c_str(), "rb+" );
    RT_HARD_ASSERT( file );
    ::fseek( file, address, SEEK_SET );

    size_t bytes_written = ::fwrite( buffer, 1, size, file );

    ::fflush( file );
    ::fclose( file );

    if ( bytes_written != size )
    {
      LOG_TRACE( "Error writing to file: %d\r\n" );
      return LFS_ERR_IO;
    }

    return LFS_ERR_OK;
  }


  static int lfs_safe_erase( const struct lfs_config *c, lfs_block_t block )
  {
    RT_HARD_ASSERT( c->context );
    Volume *vol = reinterpret_cast<Volume *>( c->context );

    /*-------------------------------------------------------------------------
    Calculate the absolute offset required to read from
    -------------------------------------------------------------------------*/
    size_t address = 0;
    if ( !Aurora::Flash::NOR::block2Address( vol->flash.deviceType(), block, &address ) )
    {
      LOG_TRACE( "Bad flash address\r\n" );
      return LFS_ERR_INVAL;
    }

    /*-------------------------------------------------------------------------
    Open the file
    -------------------------------------------------------------------------*/
    FILE *file = ::fopen( vol->_dataFile.c_str(), "rb+" );
    ::fseek( file, address, SEEK_SET );

    const uint8_t data          = 0xFF;
    size_t        bytes_written = 0;
    for ( size_t x = 0; x < c->block_size; x++ )
    {
      bytes_written += ::fwrite( &data, 1, sizeof( data ), file );
    }

    ::fflush( file );
    ::fclose( file );

    if ( bytes_written != c->block_size )
    {
      LOG_TRACE( "Error erasing file: %d\r\n" );
      return LFS_ERR_IO;
    }

    return LFS_ERR_OK;
  }


  static int lfs_safe_sync( const struct lfs_config *c )
  {
    return LFS_ERR_OK;
  }

#else /* EMBEDDED */
  static int lfs_safe_read( const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size )
  {
    /*-------------------------------------------------------------------------
    Get some information from the context pointer
    -------------------------------------------------------------------------*/
    RT_DBG_ASSERT( c->context );
    Volume *vol = reinterpret_cast<Volume *>( c->context );

    /*-------------------------------------------------------------------------
    Invoke the flash read
    -------------------------------------------------------------------------*/
    Chimera::Thread::LockGuard _lck( vol->flash );

    auto lfs_err   = LFS_ERR_IO;
    auto flash_err = vol->flash.read( block, off, buffer, size );

    if ( flash_err == AM::Status::ERR_OK )
    {
      lfs_err = LFS_ERR_OK;
    }

    LOG_TRACE_IF( flash_err != AM::Status::ERR_OK, "NOR read error: %d\r\n", flash_err );
    return lfs_err;
  }


  static int lfs_safe_prog( const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size )
  {
    /*-------------------------------------------------------------------------
    Get some information from the context pointer
    -------------------------------------------------------------------------*/
    RT_DBG_ASSERT( c->context );
    Volume *vol   = reinterpret_cast<Volume *>( c->context );
    auto    props = Aurora::Flash::NOR::getProperties( vol->flash.deviceType() );

    /*-------------------------------------------------------------------------
    Invoke the device driver
    -------------------------------------------------------------------------*/
    Chimera::Thread::LockGuard _lck( vol->flash );

    auto lfs_err   = LFS_ERR_IO;
    auto flash_err = vol->flash.write( block, off, buffer, size );

    if ( flash_err == AM::Status::ERR_OK )
    {
      flash_err = vol->flash.pendEvent( AM::Event::MEM_WRITE_COMPLETE, Chimera::Thread::TIMEOUT_BLOCK );
      if ( flash_err == AM::Status::ERR_OK )
      {
        lfs_err = LFS_ERR_OK;

        if constexpr ( DEBUG_MODULE )
        {
          uint8_t read_buf[ 64 ];
          memset( read_buf, 0, sizeof( read_buf ) );
          RT_HARD_ASSERT( size <= sizeof( read_buf ) );

          vol->flash.read( block, off, read_buf, size );

          RT_HARD_ASSERT( 0 == memcmp( buffer, read_buf, size ) );
        }
      }
    }

    LOG_TRACE_IF( flash_err != AM::Status::ERR_OK, "NOR write error: %d\r\n", flash_err );
    return lfs_err;
  }


  static int lfs_safe_erase( const struct lfs_config *c, lfs_block_t block )
  {
    /*-------------------------------------------------------------------------
    Get some information from the context pointer
    -------------------------------------------------------------------------*/
    RT_DBG_ASSERT( c->context );
    Volume *vol   = reinterpret_cast<Volume *>( c->context );

    /*-------------------------------------------------------------------------
    Invoke the device driver
    -------------------------------------------------------------------------*/
    Chimera::Thread::LockGuard _lck( vol->flash );

    auto lfs_err   = LFS_ERR_IO;
    auto flash_err = vol->flash.erase( block );

    if ( flash_err == AM::Status::ERR_OK )
    {
      flash_err = vol->flash.pendEvent( AM::Event::MEM_ERASE_COMPLETE, Chimera::Thread::TIMEOUT_BLOCK );
      if ( flash_err == AM::Status::ERR_OK )
      {
        lfs_err = LFS_ERR_OK;
      }
    }

    LOG_TRACE_IF( flash_err != AM::Status::ERR_OK, "NOR erase error: %d\r\n", flash_err );
    return lfs_err;
  }


  static int lfs_safe_sync( const struct lfs_config *c )
  {
    /*-------------------------------------------------------------------------
    Get some information from the context pointer
    -------------------------------------------------------------------------*/
    RT_DBG_ASSERT( c->context );
    Volume *vol = reinterpret_cast<Volume *>( c->context );

    /*-------------------------------------------------------------------------
    Invoke the device driver
    -------------------------------------------------------------------------*/
    Chimera::Thread::LockGuard _lck( vol->flash );

    auto lfs_err   = LFS_ERR_IO;
    auto flash_err = vol->flash.flush();

    if ( flash_err == AM::Status::ERR_OK )
    {
      lfs_err = LFS_ERR_OK;
    }

    LOG_TRACE_IF( flash_err != AM::Status::ERR_OK, "NOR flush error: %d\r\n", flash_err );
    return lfs_err;
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
    if ( !vol )
    {
      return -1;
    }

    /*-------------------------------------------------------------------------
    Ensure the backing file exists with the expected properties
    -------------------------------------------------------------------------*/
#if defined( SIMULATOR )
    bool create = true;
    auto props  = Aurora::Flash::NOR::getProperties( vol->flash.deviceType() );
    assert( props );

    if ( std::filesystem::exists( vol->_dataFile ) )
    {
      const size_t size = std::filesystem::file_size( vol->_dataFile );
      if ( props->endAddress != size )
      {
        LOG_ERROR( "File size didn't match [%d != %d]. Destroying %s\r\n",
                   props->endAddress, size, vol->_dataFile.c_str() );
        std::filesystem::remove( vol->_dataFile );
      }
      else
      {
        create = false;
      }
    }

    if ( create )
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
#endif /* SIMULATOR */

    /*-------------------------------------------------------------------------
    Otherwise, attempt to mount the drive assuming it's already formatted.
    -------------------------------------------------------------------------*/
    auto lfs_err = lfs_mount( &( vol->fs ), &( vol->cfg ) );
    LOG_TRACE_IF( lfs_err != LFS_ERR_OK, "Mount error: %s\r\n", get_error_str( lfs_err ).data() );
    return lfs_err;
  }


  static int unmount( const VolumeId drive )
  {
    Chimera::Thread::LockGuard _lck( s_lock );

    /*-------------------------------------------------------------------------
    Only attempt an unmount if the volume actually exists
    -------------------------------------------------------------------------*/
    Volume *vol = get_volume( drive );
    if ( !vol )
    {
      return -1;
    }

    /*-------------------------------------------------------------------------
    Perform the unmount, but don't destroy the volume registration. That is a
    separate (lower layer) task not related to mounting/unmounting.
    -------------------------------------------------------------------------*/
    auto lfs_err = lfs_unmount( &( vol->fs ) );
    LOG_TRACE_IF( lfs_err != LFS_ERR_OK, "Unmount error: %s\r\n", get_error_str( lfs_err ).data() );
    return lfs_err;
  }


  static int fopen( const char *filename, const AccessFlags mode, const FileId stream, const VolumeId vol )
  {
    Chimera::Thread::LockGuard _lck( s_lock );

    /*-------------------------------------------------------------------------
    Input Protections
    -------------------------------------------------------------------------*/
    if ( s_files.full() )
    {
      return -1;
    }

    /*-------------------------------------------------------------------------
    Nothing to do if the file already exists in the registry
    -------------------------------------------------------------------------*/
    File *file = get_file( stream );
    if ( file )
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

    switch ( access )
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

    if ( modifier & O_APPEND )
    {
      flags |= LFS_O_APPEND;
    }

    if ( modifier & O_CREAT )
    {
      flags |= LFS_O_CREAT;
    }

    if ( modifier & O_EXCL )
    {
      flags |= LFS_O_EXCL;
    }

    if ( modifier & O_TRUNC )
    {
      flags |= LFS_O_TRUNC;
    }

    /*-------------------------------------------------------------------------
    Allocate the file in the vector. LFS needs file control data to be static.
    -------------------------------------------------------------------------*/
    /* Create the file on the stack with some basic information */
    File new_file;
    new_file.clear();
    new_file.fileDesc = stream;
    new_file.pVolume  = volume;

    /* Re-find the file in it's new (static) memory location*/
    s_files.push_back( new_file );
    etl::shell_sort( s_files.begin(), s_files.end() );
    file = get_file( stream );
    RT_DBG_ASSERT( file );

    /* Point the cfg buffer to the new address. Validate size requirements. */
    file->lfsCfg.buffer = file->lfsCfgBuff;
    RT_HARD_ASSERT( sizeof( file->lfsCfgBuff ) >= volume->cfg.cache_size );

    /*-------------------------------------------------------------------------
    Open the new file. On failure, deallocate the file structure.
    -------------------------------------------------------------------------*/
    int lfs_err = lfs_file_opencfg( &( volume->fs ), &file->lfsFile, filename, flags, &file->lfsCfg );
    if ( lfs_err != LFS_ERR_OK )
    {
      s_files.erase( file );
      etl::shell_sort( s_files.begin(), s_files.end() );
    }

    LOG_TRACE_IF( lfs_err != LFS_ERR_OK, "Open error: %s\r\n", get_error_str( lfs_err ).data() );
    return lfs_err;
  }


  static int fclose( FileId stream )
  {
    Chimera::Thread::LockGuard _lck( s_lock );

    /*-------------------------------------------------------------------------
    Look up the file in the registry
    -------------------------------------------------------------------------*/
    File *file = get_file( stream );
    if ( !file )
    {
      return 0;
    }

    /*-------------------------------------------------------------------------
    Perform the LFS operation
    -------------------------------------------------------------------------*/
    int lfs_err = lfs_file_close( &( file->pVolume->fs ), &( file->lfsFile ) );
    if ( lfs_err < 0 )
    {
      LOG_TRACE( "Close error: %s\r\n", get_error_str( lfs_err ).data() );
      return lfs_err;
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
    if ( !file )
    {
      return 0;
    }

    /*-------------------------------------------------------------------------
    Perform the LFS operation
    -------------------------------------------------------------------------*/
    auto lfs_err = lfs_file_sync( &( file->pVolume->fs ), &( file->lfsFile ) );
    LOG_TRACE_IF( lfs_err != LFS_ERR_OK, "Sync error: %s\r\n", get_error_str( lfs_err ).data() );
    return lfs_err;
  }


  static size_t fread( void *ptr, size_t size, size_t count, FileId stream )
  {
    /*-------------------------------------------------------------------------
    Look up the file in the registry
    -------------------------------------------------------------------------*/
    File *file = get_file( stream );
    if ( !file )
    {
      return 0;
    }

    /*-------------------------------------------------------------------------
    Perform the LFS operation
    -------------------------------------------------------------------------*/
    int bytes_read = lfs_file_read( &( file->pVolume->fs ), &( file->lfsFile ), ptr, ( size * count ) );
    if ( bytes_read < 0 )
    {
      LOG_TRACE( "Read error: %s\r\n", get_error_str( bytes_read ).data() );
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
    if ( !file )
    {
      return 0;
    }

    /*-------------------------------------------------------------------------
    Perform the LFS operation
    -------------------------------------------------------------------------*/
    int bytes_written = lfs_file_write( &( file->pVolume->fs ), &( file->lfsFile ), ptr, ( size * count ) );
    if ( bytes_written < 0 )
    {
      LOG_TRACE( "Write error: %s\r\n", get_error_str( bytes_written ).data() );
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
    if ( !file )
    {
      return 0;
    }

    /*-------------------------------------------------------------------------
    Perform the LFS operation
    -------------------------------------------------------------------------*/
    auto lfs_err = lfs_file_seek( &( file->pVolume->fs ), &( file->lfsFile ), offset, whence );
    LOG_TRACE_IF( lfs_err != LFS_ERR_OK, "Seek error: %s\r\n", get_error_str( lfs_err ).data() );
    return lfs_err;
  }


  static size_t ftell( FileId stream )
  {
    /*-------------------------------------------------------------------------
    Look up the file in the registry
    -------------------------------------------------------------------------*/
    File *file = get_file( stream );
    if ( !file )
    {
      return 0;
    }

    /*-------------------------------------------------------------------------
    Perform the LFS operation
    -------------------------------------------------------------------------*/
    int lfs_err = lfs_file_tell( &( file->pVolume->fs ), &( file->lfsFile ) );
    if ( lfs_err < 0 )
    {
      LOG_TRACE_IF( lfs_err != LFS_ERR_OK, "Tell error: %s\r\n", get_error_str( lfs_err ).data() );
      return 0;
    }

    return static_cast<size_t>( lfs_err );
  }


  static void frewind( FileId stream )
  {
    /*-------------------------------------------------------------------------
    Look up the file in the registry
    -------------------------------------------------------------------------*/
    File *file = get_file( stream );
    if ( !file )
    {
      return;
    }

    /*-------------------------------------------------------------------------
    Perform the LFS operation
    -------------------------------------------------------------------------*/
    int lfs_err = lfs_file_rewind( &( file->pVolume->fs ), &( file->lfsFile ) );
    LOG_TRACE_IF( lfs_err != LFS_ERR_OK, "Rewind error: %s\r\n", get_error_str( lfs_err ).data() );
  }


  static size_t fsize( const FileId stream )
  {
    /*-------------------------------------------------------------------------
    Look up the file in the registry
    -------------------------------------------------------------------------*/
    File *file = get_file( stream );
    if ( !file )
    {
      return 0;
    }

    /*-------------------------------------------------------------------------
    Perform the LFS operation
    -------------------------------------------------------------------------*/
    lfs_off_t size = lfs_file_size( &( file->pVolume->fs ), &( file->lfsFile ) );
    if ( size >= 0 )
    {
      return static_cast<size_t>( size );
    }

    return 0;
  }

  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/
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


  Interface getInterface( Volume *const vol )
  {
    Interface intf;
    intf.clear();

    intf.context    = reinterpret_cast<void *>( vol );
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
    intf.fsize      = ::Aurora::FileSystem::LFS::fsize;

    return intf;
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
    if ( s_volumes.full() || get_volume( vol->_volumeID ) )
    {
      return false;
    }

    /*-------------------------------------------------------------------------
    Register the IO callbacks & context pointer
    -------------------------------------------------------------------------*/
    vol->cfg.read    = lfs_safe_read;
    vol->cfg.prog    = lfs_safe_prog;
    vol->cfg.erase   = lfs_safe_erase;
    vol->cfg.sync    = lfs_safe_sync;
    vol->cfg.context = reinterpret_cast<void *>( vol );

    s_volumes.push_back( vol );
    return true;
  }


  bool formatVolume( Volume *const vol )
  {
    /*-------------------------------------------------------------------------
    Input Protection
    -------------------------------------------------------------------------*/
    if ( !vol )
    {
      return false;
    }

    /*-------------------------------------------------------------------------
    Invoke the format command, assuming the configuration is OK
    -------------------------------------------------------------------------*/
    auto lfs_err = lfs_format( &( vol->fs ), &( vol->cfg ) );
    LOG_TRACE_IF( lfs_err != LFS_ERR_OK, "Format error: %s\r\n", get_error_str( lfs_err ).data() );
    return lfs_err == LFS_ERR_OK;
  }
}  // namespace Aurora::FileSystem::LFS
