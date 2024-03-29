/******************************************************************************
 *  File Name:
 *    generic_driver.cpp
 *
 *  Description:
 *    A very thin wrapper around stdio file operations
 *
 *  2021-2022 | Brandon Braun | brandonbraun653@gmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <cstdio>
#include <string>
#include <map>
#include <Aurora/filesystem>

namespace Aurora::FileSystem::Generic
{
  /*---------------------------------------------------------------------------
  Static Data
  ---------------------------------------------------------------------------*/
  static std::map<FileId, FILE *>        s_file_desc_map;
  static std::map<uint32_t, std::string> s_mode_map;

  /*---------------------------------------------------------------------------
  Static Functions
  ---------------------------------------------------------------------------*/
  static int initialize()
  {
    /*-------------------------------------------------------------------------
    Reset the module memory
    -------------------------------------------------------------------------*/
    s_file_desc_map.clear();
    s_mode_map.clear();

    /*-------------------------------------------------------------------------
    Add the supported mode flag translations
    -------------------------------------------------------------------------*/
    s_mode_map.insert( std::pair( ( O_RDONLY ), "r" ) );
    s_mode_map.insert( std::pair( ( O_WRONLY | O_CREAT ), "w" ) );
    s_mode_map.insert( std::pair( ( O_APPEND | O_EXCL ), "w+" ) );
    s_mode_map.insert( std::pair( ( O_WRONLY | O_EXCL ), "a" ) );

    return 0;
  }


  static int mount( const VolumeId drive, void *context )
  {
    return 0;
  }


  static int unmount( const VolumeId drive )
  {
    while ( !s_file_desc_map.empty() )
    {
      auto iter = s_file_desc_map.begin();
      ::fclose( iter->second );
      s_file_desc_map.erase( iter );
    }

    return 0;
  }


  static int fopen( const char *filename, const AccessFlags mode, const FileId file, const VolumeId vol )
  {
    auto mode_string = s_mode_map.at( mode );

    FILE *f = ::fopen( filename, mode_string.data() );
    s_file_desc_map.insert( std::pair( file, f ) );

    return 0;
  }


  static int fclose( FileId stream )
  {
    auto iter = s_file_desc_map.find( stream );
    if ( iter != s_file_desc_map.end() )
    {
      s_file_desc_map.erase( iter );
      return ::fclose( iter->second );
    }
    else
    {
      return -1;
    }
  }


  static int fflush( FileId stream )
  {
    auto iter = s_file_desc_map.find( stream );
    if ( iter != s_file_desc_map.end() )
    {
      return ::fflush( iter->second );
    }
    else
    {
      return -1;
    }
  }


  static size_t fread( void *ptr, size_t size, size_t count, FileId stream )
  {
    auto iter = s_file_desc_map.find( stream );
    if ( iter != s_file_desc_map.end() )
    {
      return ::fread( ptr, size, count, iter->second );
    }
    else
    {
      return 0;
    }
  }


  static size_t fwrite( const void *ptr, size_t size, size_t count, FileId stream )
  {
    auto iter = s_file_desc_map.find( stream );
    if ( iter != s_file_desc_map.end() )
    {
      return ::fwrite( ptr, size, count, iter->second );
    }
    else
    {
      return 0;
    }
  }


  static int fseek( FileId stream, size_t offset, const WhenceFlags whence )
  {
    auto iter = s_file_desc_map.find( stream );
    if ( iter != s_file_desc_map.end() )
    {
      return ::fseek( iter->second, offset, whence );
    }
    else
    {
      return 0;
    }
  }


  static size_t ftell( FileId stream )
  {
    auto iter = s_file_desc_map.find( stream );
    if ( iter != s_file_desc_map.end() )
    {
      return ::ftell( iter->second );
    }
    else
    {
      return 0;
    }
  }


  static void frewind( FileId stream )
  {
    auto iter = s_file_desc_map.find( stream );
    if ( iter != s_file_desc_map.end() )
    {
      ::rewind( iter->second );
    }
  }

  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/
  Interface getInterface()
  {
    Interface intf;
    intf.clear();

    intf.initialize = ::Aurora::FileSystem::Generic::initialize;
    intf.mount      = ::Aurora::FileSystem::Generic::mount;
    intf.unmount    = ::Aurora::FileSystem::Generic::unmount;
    intf.fopen      = ::Aurora::FileSystem::Generic::fopen;
    intf.fclose     = ::Aurora::FileSystem::Generic::fclose;
    intf.fflush     = ::Aurora::FileSystem::Generic::fflush;
    intf.fread      = ::Aurora::FileSystem::Generic::fread;
    intf.fwrite     = ::Aurora::FileSystem::Generic::fwrite;
    intf.fseek      = ::Aurora::FileSystem::Generic::fseek;
    intf.ftell      = ::Aurora::FileSystem::Generic::ftell;
    intf.frewind    = ::Aurora::FileSystem::Generic::frewind;

    return intf;
  }
}  // namespace Aurora::FileSystem::Generic
