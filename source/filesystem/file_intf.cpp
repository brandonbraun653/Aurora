/********************************************************************************
 *  File Name:
 *    file_intf.cpp
 *
 *  Description:
 *    File system interface definitions
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* Aurora Includes */
#include <Aurora/filesystem>
#include <Aurora/filesystem_spiffs>
#include <Aurora/source/filesystem/generic/generic_driver.hpp>
#include <Aurora/source/filesystem/littlefs/lfs_driver.hpp>
#include <Aurora/source/filesystem/spiffs/spiffs_driver.hpp>
#include <Aurora/source/filesystem/yaffs/yaffs2_driver.hpp>

/* Chimera Includes */
#include <Chimera/assert>

namespace Aurora::FileSystem
{
  /*-------------------------------------------------------------------------------
  Static Data
  -------------------------------------------------------------------------------*/
  static const Interface *impl = nullptr;

  /*-------------------------------------------------------------------------------
  Public Functions
  -------------------------------------------------------------------------------*/
  bool configureDriver( const BackendType type )
  {
    // TODO: Come back later and turn this into a registration function
    switch ( type )
    {
      case BackendType::DRIVER_SPIFFS:
        impl = &SPIFFS::implementation;
        break;

      case BackendType::DRIVER_LITTLE_FS:
        impl = nullptr; //&LFS::implementation;
        break;

      case BackendType::DRIVER_YAFFS2:
        impl = nullptr; //&YAFFS::implementation;
        break;

      case BackendType::DRIVER_OS:
        impl = nullptr; //&Generic::implementation;
        break;

      default:
        impl = nullptr;
        return false;
        break;
    }

    return true;
  }


  int mount()
  {
    RT_HARD_ASSERT( impl && impl->mount );
    return impl->mount();
  }


  int unmount()
  {
    RT_HARD_ASSERT( impl && impl->unmount );
    return impl->unmount();
  }


  void massErase()
  {
    SPIFFS::getNORDriver()->erase();
  }


  bool fIsValid( FileHandle &stream )
  {
    return stream >= 0;
  }

  /*-------------------------------------------------------------------------------
  STDIO Interface
  -------------------------------------------------------------------------------*/
  FileHandle fopen( const char *filename, const char *mode )
  {
    RT_HARD_ASSERT( impl && impl->fopen );
    return impl->fopen( filename, mode );
  }


  int fclose( FileHandle stream )
  {
    RT_HARD_ASSERT( impl && impl->fclose && stream );
    return impl->fclose( stream );
  }


  int fflush( FileHandle stream )
  {
    RT_HARD_ASSERT( impl && impl->fflush && stream  );
    return impl->fflush( stream );
  }


  size_t fread( void *ptr, size_t size, size_t count, FileHandle stream )
  {
    RT_HARD_ASSERT( impl && impl->fread && stream  );
    return impl->fread( ptr, size, count, stream );
  }


  size_t fwrite( const void *ptr, size_t size, size_t count, FileHandle stream )
  {
    RT_HARD_ASSERT( impl && impl->fwrite && stream  );
    return impl->fwrite( ptr, size, count, stream );
  }


  int fseek( FileHandle stream, size_t offset, size_t origin )
  {
    RT_HARD_ASSERT( impl && impl->fseek && stream  );
    return impl->fseek( stream, offset, origin );
  }


  size_t ftell( FileHandle stream )
  {
    RT_HARD_ASSERT( impl && impl->ftell && stream );
    return impl->ftell( stream );
  }


  void frewind( FileHandle stream )
  {
    RT_HARD_ASSERT( impl && impl->frewind && stream );
    impl->frewind( stream );
  }

}  // namespace Aurora::FileSystem
