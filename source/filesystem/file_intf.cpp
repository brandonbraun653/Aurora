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

namespace Aurora::FileSystem
{
  /*---------------------------------------------------------------------------
  Structures
  ---------------------------------------------------------------------------*/

  /*---------------------------------------------------------------------------
  Static Data
  ---------------------------------------------------------------------------*/

  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/
  VolumeId mount( const std::string_view &drive, const Interface *const intf )
  {
  }


  void unmount( const VolumeId volume )
  {
  }


  int fopen( const char *filename, const AccessFlags mode, FileId &file )
  {
  }


  int fclose( const FileId stream )
  {
  }


  int fflush( const FileId stream )
  {
  }


  size_t fread( void *const ptr, const size_t size, const size_t count, const FileId stream )
  {
  }


  size_t fwrite( const void *const ptr, const size_t size, const size_t count, const FileId stream )
  {
  }


  int fseek( const FileId stream, const size_t offset, const size_t origin )
  {
  }


  size_t ftell( const FileId stream )
  {
  }


  void frewind( const FileId stream )
  {
  }

}  // namespace Aurora::FileSystem
