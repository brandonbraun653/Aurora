/******************************************************************************
 *  File Name:
 *    fs_eeprom_posix.cpp
 *
 *  Description:
 *    Posix implementations of the EEPROM file system driver
 *
 *  2022 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/filesystem>
#include <Aurora/logging>
#include <Aurora/memory>
#include <Chimera/common>

namespace Aurora::FileSystem::EEPROM
{
  /*-----------------------------------------------------------------------------
  Static Functions
  -----------------------------------------------------------------------------*/
  static int mount();
  static int unmount();
  static FileHandle fopen( const char *filename, const char *mode, const size_t size );
  static int fclose( FileHandle stream );
  static int fflush( FileHandle stream );
  static size_t fread( void *ptr, size_t size, size_t count, FileHandle stream );
  static size_t fwrite( const void *ptr, size_t size, size_t count, FileHandle stream );
  static int fseek( FileHandle stream, size_t offset, size_t origin );
  static size_t ftell( FileHandle stream );
  static void frewind( FileHandle stream );


  /*---------------------------------------------------------------------------
  Static Data
  ---------------------------------------------------------------------------*/
  static Manager sManager; /**< Manager for the sole EEPROM file system */


  /*-----------------------------------------------------------------------------
  Public Data
  -----------------------------------------------------------------------------*/
  extern const Interface implementation = { .mount   = ::Aurora::FileSystem::EEPROM::mount,
                                            .unmount = ::Aurora::FileSystem::EEPROM::unmount,
                                            .fopen   = ::Aurora::FileSystem::EEPROM::fopen,
                                            .fclose  = ::Aurora::FileSystem::EEPROM::fclose,
                                            .fflush  = ::Aurora::FileSystem::EEPROM::fflush,
                                            .fread   = ::Aurora::FileSystem::EEPROM::fread,
                                            .fwrite  = ::Aurora::FileSystem::EEPROM::fwrite,
                                            .fseek   = ::Aurora::FileSystem::EEPROM::fseek,
                                            .ftell   = ::Aurora::FileSystem::EEPROM::ftell,
                                            .frewind = ::Aurora::FileSystem::EEPROM::frewind };

  /*-----------------------------------------------------------------------------
  Posix Implementation
  -----------------------------------------------------------------------------*/
  static int mount()
  {
    sManager.configure( getEEPROMDriver(), getConfiguration().mbrCache );

    if ( !sManager.mount() )
    {
      LOG_WARN( "EEPROM filesystem not initialized. Creating root partition.\r\n" );
      sManager.softReset();

      if ( !sManager.mount() )
      {
        LOG_ERROR( "Root partition failed to create\r\n" );
        return -1;
      }
    }

    return 0;
  }


  static int unmount()
  {
    sManager.unmount();
    return 0;
  }


  static FileHandle fopen( const char *filename, const char *mode, const size_t size )
  {

  }


  static int fclose( FileHandle stream )
  {

  }


  static int fflush( FileHandle stream )
  {

  }


  static size_t fread( void *ptr, size_t size, size_t count, FileHandle stream )
  {

  }


  static size_t fwrite( const void *ptr, size_t size, size_t count, FileHandle stream )
  {

  }


  static int fseek( FileHandle stream, size_t offset, size_t origin )
  {

  }


  static size_t ftell( FileHandle stream )
  {

  }


  static void frewind( FileHandle stream )
  {

  }


}  // namespace Aurora::FileSystem::EEPROM
