/********************************************************************************
 *  File Name:
 *    file_intf.cpp
 *
 *  Description:
 *    File system interface definitions
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
      #if defined( EMBEDDED )
      case BackendType::DRIVER_SPIFFS:
        impl = &SPIFFS::implementation;
        break;

      case BackendType::DRIVER_EEFS:
        impl = &EEPROM::implementation;
        break;
      #endif  /* EMBEDDED */

      #if defined( SIMULATOR )
      case BackendType::DRIVER_OS:
        impl = &Generic::implementation;
        break;
      #endif  /* SIMULATOR */

      default:
        impl = nullptr;
        return false;
        break;
    }

    return true;
  }


  int mount()
  {
    RT_DBG_ASSERT( impl && impl->mount );
    return impl->mount();
  }


  int unmount()
  {
    RT_DBG_ASSERT( impl && impl->unmount );
    return impl->unmount();
  }


  void massErase()
  {
    SPIFFS::getNORDriver()->erase();
  }


  bool fIsValid( FileHandle &stream )
  {
    #if defined( SIMULATOR )
    return stream != nullptr;
    #else
    return stream >= 0;
    #endif
  }

  /*-------------------------------------------------------------------------------
  STDIO Interface
  -------------------------------------------------------------------------------*/
  FileHandle fopen( const char *filename, const char *mode, const size_t size )
  {
    RT_DBG_ASSERT( impl && impl->fopen );
    return impl->fopen( filename, mode, size );
  }


  int fclose( FileHandle stream )
  {
    RT_DBG_ASSERT( impl && impl->fclose && stream );
    return impl->fclose( stream );
  }


  int fflush( FileHandle stream )
  {
    RT_DBG_ASSERT( impl && impl->fflush && stream  );
    return impl->fflush( stream );
  }


  size_t fread( void *ptr, size_t size, size_t count, FileHandle stream )
  {
    RT_DBG_ASSERT( impl && impl->fread && stream  );
    return impl->fread( ptr, size, count, stream );
  }


  size_t fwrite( const void *ptr, size_t size, size_t count, FileHandle stream )
  {
    RT_DBG_ASSERT( impl && impl->fwrite && stream  );
    return impl->fwrite( ptr, size, count, stream );
  }


  int fseek( FileHandle stream, size_t offset, size_t origin )
  {
    RT_DBG_ASSERT( impl && impl->fseek && stream  );
    return impl->fseek( stream, offset, origin );
  }


  size_t ftell( FileHandle stream )
  {
    RT_DBG_ASSERT( impl && impl->ftell && stream );
    return impl->ftell( stream );
  }


  void frewind( FileHandle stream )
  {
    RT_DBG_ASSERT( impl && impl->frewind && stream );
    impl->frewind( stream );
  }

}  // namespace Aurora::FileSystem
