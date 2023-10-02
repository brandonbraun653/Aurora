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
#include <etl/vector.h>


namespace Aurora::FileSystem::FatFs
{
  /*---------------------------------------------------------------------------
  Static Data
  ---------------------------------------------------------------------------*/
  static Chimera::Thread::RecursiveMutex    s_lock;    /**< Module lock */
  static etl::vector<Volume *, MAX_VOLUMES> s_volumes; /**< Registered volumes */


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
      if ( iter->volumeID == id )
      {
        return iter;
      }
    }

    return nullptr;
  }


  static int fs_init()
  {
    FatFs::initialize();
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
    vol->volumeID = drive;

    vol = get_volume( drive );
    if ( !vol )
    {
      return -1;
    }


    return -1;
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

    return -1;
  }


  static int fopen( const char *filename, const AccessFlags mode, const FileId stream, const VolumeId vol )
  {
    Chimera::Thread::LockGuard _lck( s_lock );

    Volume *volume = get_volume( vol );
    RT_DBG_ASSERT( volume );

    /*-------------------------------------------------------------------------
    Translate the mode flags to the LFS flags
    -------------------------------------------------------------------------*/
    // int      flags    = 0;
    // uint32_t access   = mode & O_ACCESS_MSK;
    // uint32_t modifier = mode & O_MODIFY_MSK;

    // switch ( access )
    // {
    //   case O_RDONLY:
    //     flags = LFS_O_RDONLY;
    //     break;

    //   case O_WRONLY:
    //     flags = LFS_O_WRONLY;
    //     break;

    //   case O_RDWR:
    //     flags = LFS_O_RDWR;
    //     break;

    //   default:
    //     return -1;
    // }

    // if ( modifier & O_APPEND )
    // {
    //   flags |= LFS_O_APPEND;
    // }

    // if ( modifier & O_CREAT )
    // {
    //   flags |= LFS_O_CREAT;
    // }

    // if ( modifier & O_EXCL )
    // {
    //   flags |= LFS_O_EXCL;
    // }

    // if ( modifier & O_TRUNC )
    // {
    //   flags |= LFS_O_TRUNC;
    // }


    return -1;
  }


  static int fclose( FileId stream )
  {
    Chimera::Thread::LockGuard _lck( s_lock );

    return -1;
  }


  static int fflush( FileId stream )
  {
    return -1;
  }


  static size_t fread( void *ptr, size_t size, size_t count, FileId stream )
  {
    return -1;
  }


  static size_t fwrite( const void *ptr, size_t size, size_t count, FileId stream )
  {
    return -1;
  }


  static int fseek( const FileId stream, const size_t offset, const WhenceFlags whence )
  {
    return -1;
  }


  static size_t ftell( FileId stream )
  {
    return -1;
  }


  static void frewind( FileId stream )
  {
  }


  static size_t fsize( const FileId stream )
  {
    return 0;
  }


  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/

  void initialize()
  {
    s_lock.unlock();
    s_volumes.clear();
  }


  Interface getInterface( Volume *const vol )
  {
    Interface intf;
    intf.clear();

    intf.context    = reinterpret_cast<void *>( vol );
    intf.initialize = ::Aurora::FileSystem::FatFs::fs_init;
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

    // TODO
    return false;
  }

}  // namespace Aurora::FileSystem::FatFs


/*-----------------------------------------------------------------------------
FatFS Hooks
-----------------------------------------------------------------------------*/
extern "C" DSTATUS disk_initialize( BYTE pdrv )
{
  return STA_NOINIT;
}


extern "C" DSTATUS disk_status( BYTE pdrv )
{
  return STA_NOINIT;
}


extern "C" DRESULT disk_read( BYTE pdrv, BYTE *buff, LBA_t sector, UINT count )
{
  return RES_PARERR;
}


extern "C" DRESULT disk_write( BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count )
{
  return RES_PARERR;
}


extern "C" DRESULT disk_ioctl( BYTE pdrv, BYTE cmd, void *buff )
{
  return RES_PARERR;
}
