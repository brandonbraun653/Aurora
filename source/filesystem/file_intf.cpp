/********************************************************************************
 *  File Name:
 *    file_intf.cpp
 *
 *  Description:
 *    File system interface implementation
 *
 *  2021-2022 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/filesystem>
#include <Chimera/assert>
#include <Chimera/thread>
#include <etl/algorithm.h>
#include <etl/vector.h>
#include <etl/string.h>

namespace Aurora::FileSystem
{
  /*---------------------------------------------------------------------------
  Constants
  ---------------------------------------------------------------------------*/
  static constexpr size_t MAX_VOLUMES          = 5;
  static constexpr size_t MAX_OPEN_FILES       = 5;
  static constexpr size_t MAX_FILE_NAME_LEN    = 32;
  static constexpr size_t MAX_DRIVE_PREFIX_LEN = 8;

  /*---------------------------------------------------------------------------
  Aliases
  ---------------------------------------------------------------------------*/
  using FilePath = etl::string<MAX_FILE_NAME_LEN>;
  using DriveStr = etl::string<MAX_DRIVE_PREFIX_LEN>;

  /*---------------------------------------------------------------------------
  Structures
  ---------------------------------------------------------------------------*/
  /**
   * @brief A light veneer to abstract information about open files
   */
  struct File
  {
    FileId   fileDesc; /**< File descriptor index for this object */
    VolumeId volDesc;  /**< Volume ID associated with the file */
    FilePath path;     /**< Path associated with this file */

    bool operator<( const File &rhs ) const
    {
      return fileDesc < rhs.fileDesc;
    }

    inline void clear()
    {
      fileDesc = -1;
      volDesc  = -1;
      path.clear();
    }
  };


  /**
   * @brief A lightweight container to hold information about a mounted drive
   */
  struct Volume
  {
    VolumeId  volDesc;     /**< Volume ID associated with this object */
    DriveStr  drivePrefix; /**< This volume's drive letter/prefix */
    Interface fsImpl;      /**< Implementation specifics of the FS */


    bool operator<( const Volume &rhs ) const
    {
      return volDesc < rhs.volDesc;
    }

    inline void clear()
    {
      volDesc = -1;
      drivePrefix.clear();
      fsImpl.clear();
    }
  };


  /*---------------------------------------------------------------------------
  Static Data
  ---------------------------------------------------------------------------*/
  static Chimera::Thread::RecursiveMutex   s_lock;         /**< Module lock */
  static etl::vector<Volume, MAX_VOLUMES>  s_volumes;      /**< Registered volumes */
  static etl::vector<File, MAX_OPEN_FILES> s_files;        /**< Currently open files */
  static FileId                            s_next_file_id; /**< Next descriptor to assign to a new file */
  static VolumeId                          s_next_vol_id;  /**< Next descriptor to assign to a new volume */


  /*---------------------------------------------------------------------------
  Static Functions
  ---------------------------------------------------------------------------*/
  /**
   * @brief Search open files to find the control data for this file
   *
   * @param file    File being searched for
   * @return const File*
   */
  const File *get_file( const FileId file )
  {
    // s_files.remove_if( [ stream ]( const File &i ) { return i.fileDesc == stream; } );

    for ( const File &f : s_files )
    {
      if ( f.fileDesc == file )
      {
        return &f;
      }
    }

    return nullptr;
  }


  /**
   * @brief Get the volume object associated with a file
   *
   * @param file    File to find the volume object for
   * @return const Volume*
   */
  const Volume *get_volume( const FileId file )
  {
    /*-------------------------------------------------------------------------
    Look up the file first to see if it exists
    -------------------------------------------------------------------------*/
    auto f = get_file( file );
    if ( !f )
    {
      return nullptr;
    }

    /*-------------------------------------------------------------------------
    Otherwise it exists, which means the volume must exist
    -------------------------------------------------------------------------*/
    for ( const Volume &v : s_volumes )
    {
      if ( v.volDesc == f->volDesc )
      {
        return &v;
      }
    }

    /* Should never get here in reality */
    RT_DBG_ASSERT( false );
    return nullptr;
  }


  /**
   * @brief Get the filesystem implementation associated with a file id
   *
   * @param file    Which file to look up an interface for
   * @return const Interface&
   */
  const Interface *get_interface( const FileId file )
  {
    Chimera::Thread::LockGuard _lck( s_lock );
    auto                       vol = get_volume( file );
    return vol ? &vol->fsImpl : nullptr;
  }


  /**
   * @brief Checks if the given interface is valid
   *
   * @param intf    Interface to check
   * @return boolean
   */
  inline bool is_intf_valid( const Interface &intf )
  {
    return ( intf.fclose && intf.fflush && intf.fopen && intf.fread && intf.frewind && intf.fseek && intf.ftell && intf.fwrite
             && intf.initialize && intf.mount && intf.unmount );
  }


  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/
  void initialize()
  {
    /*-------------------------------------------------------------------------
    Clear module memory
    -------------------------------------------------------------------------*/
    s_volumes.clear();
    s_files.clear();
    s_lock.unlock();
    s_next_file_id = 0;
  }


  VolumeId mount( const std::string_view &drive, Interface &&intf )
  {
    Chimera::Thread::LockGuard _lck( s_lock );

    /*-------------------------------------------------------------------------
    Validate the inputs
    -------------------------------------------------------------------------*/
    if ( s_volumes.full() || !is_intf_valid( intf ) || ( drive.size() > MAX_DRIVE_PREFIX_LEN ) )
    {
      return -1;
    }

    /*-------------------------------------------------------------------------
    Invoke the interface's mount capabilities
    -------------------------------------------------------------------------*/
    if ( ( intf.initialize() != 0 ) || ( intf.mount( drive ) != 0 ) )
    {
      return -1;
    }

    /*-------------------------------------------------------------------------
    Build up the new volume
    -------------------------------------------------------------------------*/
    Volume vol;

    vol.fsImpl      = intf;
    vol.volDesc     = s_next_vol_id++;
    vol.drivePrefix = drive.data();

    if ( vol.drivePrefix.is_truncated() )
    {
      s_next_vol_id--;
      return -1;
    }

    s_volumes.push_back( vol );
    etl::shell_sort( s_volumes.begin(), s_volumes.end() );
    return vol.volDesc;
  }


  void unmount( const VolumeId volume )
  {
    Chimera::Thread::LockGuard _lck( s_lock );

    /*-------------------------------------------------------------------------
    Close all files associated with this volume
    -------------------------------------------------------------------------*/
    while ( true )
    {
      auto iter = etl::find_if( s_files.begin(), s_files.end(), [ volume ]( const File &f ) { return f.volDesc == volume; } );
      if ( iter == s_files.end() )
      {
        break;
      }

      fclose( iter->fileDesc );
    }

    /*-------------------------------------------------------------------------
    Destroy the volume
    -------------------------------------------------------------------------*/
    auto iter = etl::find_if( s_volumes.begin(), s_volumes.end(),
                              [ volume ]( const Volume &v ) { return v.volDesc == volume; } );
    if ( iter != s_volumes.end() )
    {
      iter->fsImpl.unmount();
      s_volumes.erase( iter );
      etl::shell_sort( s_volumes.begin(), s_volumes.end() );
    }
  }


  int fopen( const char *filename, const AccessFlags mode, FileId &file )
  {
    Chimera::Thread::LockGuard _lck( s_lock );

    /*-------------------------------------------------------------------------
    Input Protections
    -------------------------------------------------------------------------*/
    if ( !filename || !mode )
    {
      return -1;
    }

    /*-------------------------------------------------------------------------
    Check if the file already exists
    -------------------------------------------------------------------------*/
    auto f_iter = etl::find_if( s_files.begin(), s_files.end(),
                                [ filename ]( const File &f ) { return f.path.compare( filename ) == 0; } );
    if ( f_iter != s_files.end() )
    {
      file = f_iter->fileDesc;
      return 0;
    }
    else if ( s_files.full() )
    {
      return -1; /* No space for a new entry */
    }

    /*-------------------------------------------------------------------------
    Create the new entry
    -------------------------------------------------------------------------*/
    File f;

    f.clear();
    f.path = filename;
    if ( f.path.is_truncated() )
    {
      return -1; /* File path too long */
    }

    etl::string_view fstr( filename );
    auto             v_iter = etl::find_if( s_volumes.begin(), s_volumes.end(),
                                            [ fstr ]( const Volume &v ) { return fstr.starts_with( v.drivePrefix ); } );
    if ( v_iter == s_volumes.end() )
    {
      return -1; /* Volume associated with this path doesn't exist */
    }

    f.volDesc  = v_iter->volDesc;
    f.fileDesc = s_next_file_id++;

    /*-------------------------------------------------------------------------
    Update the system registry
    -------------------------------------------------------------------------*/
    file = f.fileDesc;
    s_files.push_back( f );
    etl::shell_sort( s_files.begin(), s_files.end() );
    return 0;
  }


  int fclose( const FileId stream )
  {
    Chimera::Thread::LockGuard _lck( s_lock );

    /*-------------------------------------------------------------------------
    Find the file and close the stream
    -------------------------------------------------------------------------*/
    auto impl = get_interface( stream );
    RT_DBG_ASSERT( impl );

    const int cached_close_result = impl->fclose( stream );

    /*-------------------------------------------------------------------------
    Remove the file from the open file list
    -------------------------------------------------------------------------*/
    auto file = get_file( stream );
    s_files.erase( file );
    etl::shell_sort( s_files.begin(), s_files.end() );

    return cached_close_result;
  }


  int fflush( const FileId stream )
  {
    auto impl = get_interface( stream );
    RT_DBG_ASSERT( impl );
    return impl->fflush( stream );
  }


  size_t fread( void *const ptr, const size_t size, const size_t count, const FileId stream )
  {
    auto impl = get_interface( stream );
    RT_DBG_ASSERT( impl );
    return impl->fread( ptr, size, count, stream );
  }


  size_t fwrite( const void *const ptr, const size_t size, const size_t count, const FileId stream )
  {
    auto impl = get_interface( stream );
    RT_DBG_ASSERT( impl );
    return impl->fwrite( ptr, size, count, stream );
  }


  int fseek( const FileId stream, const size_t offset, const size_t origin )
  {
    auto impl = get_interface( stream );
    RT_DBG_ASSERT( impl );
    return impl->fseek( stream, offset, origin );
  }


  size_t ftell( const FileId stream )
  {
    auto impl = get_interface( stream );
    RT_DBG_ASSERT( impl );
    return impl->ftell( stream );
  }


  void frewind( const FileId stream )
  {
    auto impl = get_interface( stream );
    RT_DBG_ASSERT( impl );
    impl->frewind( stream );
  }

}  // namespace Aurora::FileSystem
