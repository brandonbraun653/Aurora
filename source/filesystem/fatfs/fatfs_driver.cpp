/******************************************************************************
 *  File Name:
 *    fatfs_driver.cpp
 *
 *  Description:
 *    FatFS driver implementation for Aurora
 *
 *  2023 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include "ff.h"     /* Obtains integer types */
#include "diskio.h" /* Declarations of disk functions */

#include <Aurora/filesystem>
#include <Aurora/logging>
#include <Chimera/thread>
#include <etl/array.h>
#include <etl/string.h>
#include <etl/vector.h>


namespace Aurora::FileSystem::FatFs
{
  /*---------------------------------------------------------------------------
  Constants
  ---------------------------------------------------------------------------*/

  static const etl::string_view                 s_fatfs_unknown_err = "Unknown error";
  static const etl::array<etl::string_view, 20> s_fatfs_err_to_str  = {
    "Success",
    "A hard error occurred in the low level disk I/O layer",
    "Assertion failed",
    "The physical drive cannot work",
    "Could not find the file",
    "Could not find the path",
    "The path name format is invalid",
    "Access denied due to prohibited access or directory full",
    "Access denied due to prohibited access",
    "The file/directory object is invalid",
    "The physical drive is write protected",
    "The logical drive number is invalid",
    "The volume has no work area",
    "There is no valid FAT volume",
    "The f_mkfs() aborted due to any problem",
    "Could not get a grant to access the volume within defined period",
    "The operation is rejected according to the file sharing policy",
    "LFN working buffer could not be allocated",
    "Number of open files > FF_FS_LOCK",
    "Given parameter is invalid"
  };


  /*---------------------------------------------------------------------------
  Structures
  ---------------------------------------------------------------------------*/

  /**
   * @brief Internal representation of an FatFs file.
   */
  struct File
  {
    FileId  fileDesc; /**< Descriptor assigned to this file */
    Volume *pVolume;  /**< Parent volume file belongs to */
    FIL     fatfs_file_cb;     /**< FatFs file control block */

    bool operator<( const File &rhs ) const
    {
      return fileDesc < rhs.fileDesc;
    }

    inline void clear()
    {
      fileDesc = -1;
      pVolume  = nullptr;
      memset( &fatfs_file_cb, 0, sizeof( fatfs_file_cb ) );
    }
  };

  /*---------------------------------------------------------------------------
  Static Data
  ---------------------------------------------------------------------------*/
  static Chimera::Thread::RecursiveMutex    s_lock;    /**< Module lock */
  static etl::vector<Volume *, MAX_VOLUMES> s_volumes; /**< Registered volumes */
  static etl::vector<File, MAX_OPEN_FILES>  s_files;   /**< Currently open files */


  /*---------------------------------------------------------------------------
  Static Functions
  ---------------------------------------------------------------------------*/

  /**
   * @brief Finds the volume structure from an ID
   *
   * @param id    Which ID is associated with the volume
   * @return Volume*
   */
  static Volume *get_volume( VolumeId id )
  {
    for ( Volume *iter : s_volumes )
    {
      if ( iter->volumeID == id )
      {
        return iter;
      }
    }

    return nullptr;
  }

  /**
   * @brief Finds the file structure from an ID
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


  /**
   * @brief Gets the string representation of a FatFS error code
   *
   * @param error Error code to look up
   * @return etl::string_view
   */
  static etl::string_view get_error_str( const FRESULT error )
  {
    if ( ( error >= 0 ) && ( error < s_fatfs_err_to_str.size() ) )
    {
      return s_fatfs_err_to_str[ error ];
    }
    else
    {
      return s_fatfs_unknown_err;
    }
  }


  static int init()
  {
    return 0;
  }


  static int mount( const VolumeId drive, void *context )
  {
    using namespace Aurora::Memory;

    Chimera::Thread::LockGuard _lck( s_lock );

    /*-------------------------------------------------------------------------
    Update the drive number for the Volume object passed in.
    -------------------------------------------------------------------------*/
    RT_HARD_ASSERT( context );
    Volume *vol   = reinterpret_cast<Volume *>( context );
    vol->volumeID = drive;
    vol->status   = STA_NOINIT;

    vol = get_volume( drive );
    if ( !vol )
    {
      return -1;
    }

    /*-------------------------------------------------------------------------
    Mount the volume immediately
    -------------------------------------------------------------------------*/
    const FRESULT res = f_mount( &vol->fs, vol->path.c_str(), 1u );
    LOG_ERROR_IF( res != FR_OK, "Failed to mount volume %s: %s", vol->path.c_str(), get_error_str( res ).cbegin() );

    return ( res == FR_OK ) ? 0 : -1;
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
      return 0;
    }

    /*-------------------------------------------------------------------------
    Unmount the volume
    -------------------------------------------------------------------------*/
    const FRESULT res = f_unmount( vol->path.c_str() );
    LOG_ERROR_IF( res != FR_OK, "Failed to unmount volume %s: %s", vol->path.c_str(), get_error_str( res ).cbegin() );

    return ( res == FR_OK ) ? 0 : -1;
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
    Translate the mode flags to the FatFs flags
    -------------------------------------------------------------------------*/
    BYTE     flags    = 0;
    uint32_t access   = mode & O_ACCESS_MSK;
    uint32_t modifier = mode & O_MODIFY_MSK;

    switch ( access )
    {
      case O_RDONLY:
        flags = FA_READ;
        break;

      case O_WRONLY:
        flags = FA_WRITE;
        break;

      case O_RDWR:
        flags = FA_READ | FA_WRITE;
        break;

      default:
        return -1;
    }

    if ( modifier & O_APPEND )
    {
      flags |= FA_OPEN_APPEND;
    }

    if ( modifier & O_CREAT )
    {
      flags |= FA_CREATE_NEW;
    }

    if ( modifier & O_EXCL )
    {
      flags |= FA_OPEN_EXISTING;
    }

    if ( modifier & O_TRUNC )
    {
      flags |= FA_CREATE_ALWAYS;
    }

    /*-------------------------------------------------------------------------
    Allocate a file in the local vector
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

    /*-------------------------------------------------------------------------
    Open the new file. On failure, deallocate the file structure.
    -------------------------------------------------------------------------*/
    const FRESULT error = f_open( &file->fatfs_file_cb, filename, flags );
    if ( error != FR_OK )
    {
      s_files.erase( file );
      etl::shell_sort( s_files.begin(), s_files.end() );
    }

    LOG_TRACE_IF( error != FR_OK, "Open error: %s", get_error_str( error ).data() );
    return ( error == FR_OK ) ? 0 : -1;
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
    Perform the FatFs operation
    -------------------------------------------------------------------------*/
    const FRESULT error = f_close( &file->fatfs_file_cb );
    if ( error != FR_OK )
    {
      LOG_TRACE( "Close error: %s", get_error_str( error ).data() );
      return -1;
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
    Perform the FatFs operation
    -------------------------------------------------------------------------*/
    const FRESULT error = f_sync( &file->fatfs_file_cb );
    LOG_TRACE_IF( error != FR_OK, "Sync error: %s", get_error_str( error ).data() );
    return -1;
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
    Perform the FatFs operation
    -------------------------------------------------------------------------*/
    UINT          bytes_read = 0;
    const FRESULT error      = f_read( &file->fatfs_file_cb, ptr, ( size * count ), &bytes_read );
    if ( error != FR_OK )
    {
      LOG_TRACE( "Read error: %s", get_error_str( error ).data() );
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
    Perform the FatFs operation
    -------------------------------------------------------------------------*/
    UINT          bytes_written = 0;
    const FRESULT error         = f_write( &file->fatfs_file_cb, ptr, ( size * count ), &bytes_written );
    if ( error != FR_OK )
    {
      LOG_TRACE( "Write error: %s", get_error_str( error ).data() );
      return 0;
    }

    return static_cast<size_t>( bytes_written );
  }


  static int fseek( const FileId stream, const size_t offset, const WhenceFlags whence )
  {
    /*-------------------------------------------------------------------------
    Gate the whence flags. FatFs only supports SEEK_SET.
    -------------------------------------------------------------------------*/
    if( whence != WhenceFlags::F_SEEK_SET )
    {
      return -1;
    }

    /*-------------------------------------------------------------------------
    Look up the file in the registry
    -------------------------------------------------------------------------*/
    File *file = get_file( stream );
    if ( !file )
    {
      return -1;
    }

    /*-------------------------------------------------------------------------
    Perform the FatFs operation. This returns the new offset if successful, which
    is a breaking change from the standard fseek. Override it.
    -------------------------------------------------------------------------*/
    FRESULT error = f_lseek( &file->fatfs_file_cb, offset );
    LOG_TRACE_IF( error < 0, "Seek error: %s", get_error_str( error ).data() );
    return ( error < 0 ) ? -1 : 0;
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
    Perform the FatFs operation
    -------------------------------------------------------------------------*/
    const FSIZE_t size = f_tell( &file->fatfs_file_cb );
    if ( size >= 0 )
    {
      return static_cast<size_t>( size );
    }

    return 0;
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
    Perform the FatFs operation
    -------------------------------------------------------------------------*/
    const FRESULT error = f_rewind( &file->fatfs_file_cb );
    LOG_TRACE_IF( error != FR_OK, "Rewind error: %s", get_error_str( error ).data() );
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
    Perform the FatFs operation
    -------------------------------------------------------------------------*/
    FSIZE_t size = f_size( &file->fatfs_file_cb );
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
    s_lock.unlock();
    s_volumes.clear();
    s_files.clear();
  }


  Interface getInterface( Volume *const vol )
  {
    Interface intf;
    intf.clear();

    intf.context    = reinterpret_cast<void *>( vol );
    intf.initialize = ::Aurora::FileSystem::FatFs::init;
    intf.mount      = ::Aurora::FileSystem::FatFs::mount;
    intf.unmount    = ::Aurora::FileSystem::FatFs::unmount;
    intf.fopen      = ::Aurora::FileSystem::FatFs::fopen;
    intf.fclose     = ::Aurora::FileSystem::FatFs::fclose;
    intf.fflush     = ::Aurora::FileSystem::FatFs::fflush;
    intf.fread      = ::Aurora::FileSystem::FatFs::fread;
    intf.fwrite     = ::Aurora::FileSystem::FatFs::fwrite;
    intf.fseek      = ::Aurora::FileSystem::FatFs::fseek;
    intf.ftell      = ::Aurora::FileSystem::FatFs::ftell;
    intf.frewind    = ::Aurora::FileSystem::FatFs::frewind;
    intf.fsize      = ::Aurora::FileSystem::FatFs::fsize;

    return intf;
  }


  bool attachVolume( Volume *const vol )
  {
    /*-------------------------------------------------------------------------
    Input Protection
    -------------------------------------------------------------------------*/
    if ( !vol || !vol->device )
    {
      return false;
    }

    /*-------------------------------------------------------------------------
    Ensure we can store the new volume and it hasn't already been registered.
    -------------------------------------------------------------------------*/
    if ( s_volumes.full() || get_volume( vol->volumeID ) )
    {
      return false;
    }

    /*-------------------------------------------------------------------------
    Initialize the volume
    -------------------------------------------------------------------------*/
    memset( &vol->fs, 0, sizeof( vol->fs ) );

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
    Configure the filesystem
    -------------------------------------------------------------------------*/
    // TODO: Need to set these parameters from the card attributes
    // MKFS_PARM opt;
    // memset( &opt, 0, sizeof( opt ) );

    /*-------------------------------------------------------------------------
    Make the filesystem using defaults
    -------------------------------------------------------------------------*/
    uint8_t work[ FF_MAX_SS ];
    memset( work, 0, sizeof( work ) );

    FRESULT res = f_mkfs( vol->path.c_str(), nullptr, work, sizeof( work ) );
    if ( res != FR_OK )
    {
      LOG_ERROR( "Failed to format volume %s: %s", vol->path.c_str(), get_error_str( res ).cbegin() );
      return false;
    }

    return true;
  }

}  // namespace Aurora::FileSystem::FatFs


/*-----------------------------------------------------------------------------
FatFS Hooks
-----------------------------------------------------------------------------*/
using namespace Aurora::FileSystem::FatFs;
using namespace Aurora::Memory;


/**
 * @brief Simple helper to get the volume from the drive number
 *
 * @param pdrv  Which drive number to look up
 * @return Volume*
 */
static inline Volume *get_volume( BYTE pdrv )
{
  for ( Volume *const iter : s_volumes )
  {
    if ( iter->fs.pdrv == pdrv )
    {
      return iter;
    }
  }

  return nullptr;
}


extern "C" DSTATUS disk_initialize( BYTE pdrv )
{
  constexpr size_t OPEN_ATTEMPTS = 3;

  /*---------------------------------------------------------------------------
  Volume should exist at this point
  ---------------------------------------------------------------------------*/
  Volume *vol = get_volume( pdrv );
  if ( !vol )
  {
    return STA_NOINIT;
  }

  /*-------------------------------------------------------------------------
  Attempt to open the memory device backing the volume
  -------------------------------------------------------------------------*/
  vol->status = STA_NOINIT;

  for ( size_t x = 0; x < OPEN_ATTEMPTS; x++ )
  {
    if ( vol->device->open( nullptr ) == Status::ERR_OK )
    {
      vol->status = 0;
      break;
    }

    LOG_DEBUG( "Re-attempt opening device %s", vol->path.c_str() );
    Chimera::delayMilliseconds( vol->mount_retry_delay );
  }

  if ( vol->status & STA_NOINIT )
  {
    LOG_ERROR( "Failed to open device: %s", vol->path.c_str() );
  }

  return vol->status;
}


extern "C" DSTATUS disk_status( BYTE pdrv )
{
  /*---------------------------------------------------------------------------
  Return the status of the volume if it exists, otherwise default to zero,
  which is as close to "no status" as we can get.
  ---------------------------------------------------------------------------*/
  Volume *vol = get_volume( pdrv );
  return ( vol ) ? vol->status : 0;
}


extern "C" DRESULT disk_read( BYTE pdrv, BYTE *buff, LBA_t sector, UINT count )
{
  /*---------------------------------------------------------------------------
  Input Protection
  ---------------------------------------------------------------------------*/
  if ( !buff || !count )
  {
    return RES_PARERR;
  }

  Volume *vol = get_volume( pdrv );
  if ( !vol )
  {
    return RES_NOTRDY;
  }

  const DeviceAttr attr = vol->device->getAttributes();
  if ( attr.readSize < 512 || attr.readSize > 4096 )
  {
    LOG_ERROR( "%s device block read size invalid: %d", vol->path.c_str(), attr.readSize );
    return RES_PARERR;
  }

  /*---------------------------------------------------------------------------
  Read the data from the memory device
  ---------------------------------------------------------------------------*/
  const Status result = vol->device->read( sector, 0, buff, count * attr.readSize );

  if ( result == Status::ERR_OK )
  {
    return RES_OK;
  }
  else
  {
    LOG_ERROR( "%s read fail: %d:%d [sector:count]", vol->path.c_str(), sector, count );
    return RES_ERROR;
  }
}


extern "C" DRESULT disk_write( BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count )
{
  /*---------------------------------------------------------------------------
  Input Protection
  ---------------------------------------------------------------------------*/
  if ( !buff || !count )
  {
    return RES_PARERR;
  }

  Volume *vol = get_volume( pdrv );
  if ( !vol )
  {
    return RES_NOTRDY;
  }

  /*---------------------------------------------------------------------------
  Write the data to the memory device
  ---------------------------------------------------------------------------*/
  const DeviceAttr attr   = vol->device->getAttributes();
  const Status     result = vol->device->write( sector, 0, buff, count * attr.writeSize );

  if ( result == Status::ERR_OK )
  {
    return RES_OK;
  }
  else
  {
    LOG_ERROR( "%s write fail: %d:%d [sector:count]", vol->path.c_str(), sector, count );
    return RES_ERROR;
  }
}


extern "C" DRESULT disk_ioctl( BYTE pdrv, BYTE cmd, void *buff )
{
  /*---------------------------------------------------------------------------
  Input Protection
  Notes:
    - buff can be null for some commands
  ---------------------------------------------------------------------------*/
  Volume *vol = get_volume( pdrv );
  if ( !vol )
  {
    return RES_NOTRDY;
  }

  /*---------------------------------------------------------------------------
  Handle the command
  ---------------------------------------------------------------------------*/
  switch ( cmd )
  {
    case CTRL_SYNC:
      return RES_OK;

    case GET_SECTOR_COUNT: {
      RT_DBG_ASSERT( buff );

      const DeviceAttr attr              = vol->device->getAttributes();
      *reinterpret_cast<LBA_t *>( buff ) = attr.blockCount;
      return RES_OK;
    }

    case GET_SECTOR_SIZE: {
      RT_DBG_ASSERT( buff );

      const DeviceAttr attr             = vol->device->getAttributes();
      *reinterpret_cast<WORD *>( buff ) = attr.readSize;
      return RES_OK;
    }

    case GET_BLOCK_SIZE: {
      RT_DBG_ASSERT( buff );

      const DeviceAttr attr              = vol->device->getAttributes();
      *reinterpret_cast<DWORD *>( buff ) = attr.eraseSize;
      return RES_OK;
    }

    default:
      return RES_PARERR;
  }
}
