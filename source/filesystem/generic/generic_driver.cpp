/********************************************************************************
 *  File Name:
 *    generic_driver.cpp
 *
 *  Description:
 *    A very thin wrapper around stdio file operations
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* STL Includes */
#include <cstdio>

/* Aurora Includes */
#include <Aurora/filesystem>

namespace Aurora::FileSystem::Generic
{
  /*-------------------------------------------------------------------------------
  Static Functions
  -------------------------------------------------------------------------------*/
  static int mount()
  {
    return 0;
  }


  static int unmount()
  {
    return 0;
  }


  static FileHandle fopen( const char *filename, const char *mode )
  {
    return reinterpret_cast<FileHandle>( ::fopen( filename, mode ) );
  }


  static int fclose( FileHandle stream )
  {
    return ::fclose( reinterpret_cast<FILE *>( stream ) );
  }


  static int fflush( FileHandle stream )
  {
    return ::fflush( reinterpret_cast<FILE *>( stream ) );
  }


  static size_t fread( void *ptr, size_t size, size_t count, FileHandle stream )
  {
    return ::fread( ptr, size, count, reinterpret_cast<FILE *>( stream ) );
  }


  static size_t fwrite( const void *ptr, size_t size, size_t count, FileHandle stream )
  {
    return ::fwrite( ptr, size, count, reinterpret_cast<FILE *>( stream ) );
  }


  static int fseek( FileHandle stream, size_t offset, size_t origin )
  {
    return ::fseek( reinterpret_cast<FILE *>( stream ), offset, origin );
  }


  static size_t ftell( FileHandle stream )
  {
    return ::ftell( reinterpret_cast<FILE *>( stream ) );
  }


  static void frewind( FileHandle stream )
  {
    ::rewind( reinterpret_cast<FILE *>( stream ) );
  }


  /*-------------------------------------------------------------------------------
  Public Data
  -------------------------------------------------------------------------------*/
  extern const Interface implementation = { .mount   = ::Aurora::FileSystem::Generic::mount,
                                            .unmount = ::Aurora::FileSystem::Generic::unmount,
                                            .fopen   = ::Aurora::FileSystem::Generic::fopen,
                                            .fclose  = ::Aurora::FileSystem::Generic::fclose,
                                            .fflush  = ::Aurora::FileSystem::Generic::fflush,
                                            .fread   = ::Aurora::FileSystem::Generic::fread,
                                            .fwrite  = ::Aurora::FileSystem::Generic::fwrite,
                                            .fseek   = ::Aurora::FileSystem::Generic::fseek,
                                            .ftell   = ::Aurora::FileSystem::Generic::ftell,
                                            .frewind = ::Aurora::FileSystem::Generic::frewind };
}  // namespace Aurora::FileSystem::Generic
