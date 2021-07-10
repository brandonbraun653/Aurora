/********************************************************************************
 *  File Name:
 *    spiffs_posix.cpp
 *
 *  Description:
 *    Posix implementations for SPIFFS
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* STL Includes */
#include <cstdint>

/* Aurora Include */
#include <Aurora/memory>
#include <Aurora/filesystem_spiffs>
#include <Aurora/logging>
#include <Aurora/source/filesystem/file_driver.hpp>
#include <Aurora/source/filesystem/file_config.hpp>

/* Chimera Includes */
#include <Chimera/assert>
#include <Chimera/thread>
#include <Chimera/watchdog>

/* ETL Includes */
#include <etl/flat_map.h>

/* SPIFFS Includes */
#include "spiffs.h"


static constexpr bool LOG_ENABLE = true;
#define SPIFFS_LOG( x )       \
  if constexpr ( LOG_ENABLE ) \
  {                           \
    ( x );                    \
  }


namespace Aurora::FileSystem::SPIFFS
{
  /*-------------------------------------------------------------------------------
  Constants
  -------------------------------------------------------------------------------*/
  static constexpr size_t LOG_PAGE_SIZE    = 128;
  static const std::string_view boot_file  = "_spiffs_boot_log.bin";
  static const std::string_view drive_name = "spiffs_fs_drive";

  /*-------------------------------------------------------------------------------
  Structures
  -------------------------------------------------------------------------------*/
  struct BootData
  {
    uint32_t boot_count;
    char drive_name[ 32 ];
  };

  /*-------------------------------------------------------------------------------
  Static Functions
  -------------------------------------------------------------------------------*/
  static void checkSPIFFSErrors();

  static int mount();
  static int unmount();
  static FileHandle fopen( const char *filename, const char *mode );
  static int fclose( FileHandle stream );
  static int fflush( FileHandle stream );
  static size_t fread( void *ptr, size_t size, size_t count, FileHandle stream );
  static size_t fwrite( const void *ptr, size_t size, size_t count, FileHandle stream );
  static int fseek( FileHandle stream, size_t offset, size_t origin );
  static size_t ftell( FileHandle stream );
  static void frewind( FileHandle stream );


  /*-------------------------------------------------------------------------------
  Public Data
  -------------------------------------------------------------------------------*/
  extern const Interface implementation = { .mount   = ::Aurora::FileSystem::SPIFFS::mount,
                                            .unmount = ::Aurora::FileSystem::SPIFFS::unmount,
                                            .fopen   = ::Aurora::FileSystem::SPIFFS::fopen,
                                            .fclose  = ::Aurora::FileSystem::SPIFFS::fclose,
                                            .fflush  = ::Aurora::FileSystem::SPIFFS::fflush,
                                            .fread   = ::Aurora::FileSystem::SPIFFS::fread,
                                            .fwrite  = ::Aurora::FileSystem::SPIFFS::fwrite,
                                            .fseek   = ::Aurora::FileSystem::SPIFFS::fseek,
                                            .ftell   = ::Aurora::FileSystem::SPIFFS::ftell,
                                            .frewind = ::Aurora::FileSystem::SPIFFS::frewind };

  /*-------------------------------------------------------------------------------
  Static Data
  -------------------------------------------------------------------------------*/
  static const Aurora::Memory::Properties *sNORProps = nullptr; /**< External NOR flash properties */
  static etl::flat_map<const char *, spiffs_file, 15> s_open_files;
  static u8_t spiffs_work_buf[ LOG_PAGE_SIZE * 2 ];
  static u8_t spiffs_fds[ 32 * 4 ];
  static u8_t spiffs_cache_buf[ ( LOG_PAGE_SIZE + 32 ) * 4 ];
  static spiffs fs;
  static Chimera::Thread::RecursiveMutex s_file_mtx;


  /*-------------------------------------------------------------------------------
  POSIX API Implementation
  -------------------------------------------------------------------------------*/
  static void checkSPIFFSErrors()
  {
    LOG_ERROR_IF( SPIFFS_errno( &fs ) != SPIFFS_OK, "SPIFFS error: %d\r\n", SPIFFS_errno( &fs ) );
    SPIFFS_clearerr( &fs );
  }


  static int mount()
  {
    using namespace Aurora::Logging;
    using namespace Aurora::Memory;

    namespace fileSys = ::Aurora::FileSystem;

    /*-------------------------------------------------
    Load in the device properties
    -------------------------------------------------*/
    sNORProps = Aurora::Flash::NOR::getProperties( getNORDriver()->deviceType() );

    /*-------------------------------------------------
    What is the device's erase size?
    -------------------------------------------------*/
    size_t erase_size = 0;
    switch ( sNORProps->eraseChunk )
    {
      case Chunk::PAGE:
        erase_size = sNORProps->pageSize;
        break;

      case Chunk::BLOCK:
        erase_size = sNORProps->blockSize;
        break;

      case Chunk::SECTOR:
        erase_size = sNORProps->sectorSize;
        break;

      default:
        return -1;
        break;
    }

    /*-------------------------------------------------
    Configure the mount options
    -------------------------------------------------*/
    spiffs_config cfg;
    cfg.phys_size        = sNORProps->endAddress - sNORProps->startAddress;
    cfg.phys_addr        = sNORProps->startAddress;
    cfg.phys_erase_block = erase_size;
    cfg.log_block_size   = erase_size;
    cfg.log_page_size    = LOG_PAGE_SIZE;
    cfg.hal_read_f       = nor_read;
    cfg.hal_write_f      = nor_write;
    cfg.hal_erase_f      = nor_erase;

    /*-------------------------------------------------
    Debug only
    -------------------------------------------------*/
    // getNORDriver()->erase();
    // SPIFFS_format( &fs );

    /*-------------------------------------------------
    Try mounting. It's possible to get a clean chip,
    which will need some formatting before mounting.
    -------------------------------------------------*/
    int return_code = -1;

    SPIFFS_LOG( LOG_DEBUG( "Mounting filesystem\r\n" ) );
    auto mount_err = SPIFFS_mount( &fs, &cfg, spiffs_work_buf, spiffs_fds, sizeof( spiffs_fds ), spiffs_cache_buf,
                                   sizeof( spiffs_cache_buf ), 0 );

    /*-------------------------------------------------
    If the mount initially failed, follow the format
    procedure detailed in the project Wiki.
    -------------------------------------------------*/
    if ( mount_err != SPIFFS_OK )
    {
#if ( SPIFFS_USE_MAGIC == 0 )
      /* Mount expects a fully erased chip */
      getNORDriver()->erase();

      /* Mount to configure the runtime parts */
      SPIFFS_mount( &fs, &cfg, spiffs_work_buf, spiffs_fds, sizeof( spiffs_fds ), spiffs_cache_buf, sizeof( spiffs_cache_buf ),
                    0 );

      /* Format, which must be performed while unmounted */
      SPIFFS_unmount( &fs );
      SPIFFS_format( &fs );

      /* Remount */
      mount_err = SPIFFS_mount( &fs, &cfg, spiffs_work_buf, spiffs_fds, sizeof( spiffs_fds ), spiffs_cache_buf,
                                sizeof( spiffs_cache_buf ), 0 );

#elif ( SPIFFS_USE_MAGIC == 1 )
      if ( mount_err != SPIFFS_ERR_NOT_A_FS )
      {
        SPIFFS_unmount( &fs );
        getNORDriver()->erase();
      }

      SPIFFS_format( &fs );
      mount_err = SPIFFS_mount( &fs, &cfg, spiffs_work_buf, spiffs_fds, sizeof( spiffs_fds ), spiffs_cache_buf,
                                sizeof( spiffs_cache_buf ), 0 );
#else
#error "SPIFFS_USE_MAGIC must be defined and set to either 1 or 0"
#endif
    }

    /*-------------------------------------------------
    Re-check the mount procedure again
    -------------------------------------------------*/
    if ( ( mount_err != SPIFFS_OK ) || !SPIFFS_mounted( &fs ) )
    {
      SPIFFS_LOG( LOG_DEBUG( "Failed to mount filesystem\r\n" ) );
      return -1;
    }

    u32_t total, used;
    SPIFFS_info( &fs, &total, &used );
    LOG_DEBUG( "SPIFFS Metrics -- Used: %d, Total: %d\r\n", used, total );

    return return_code;
  }


  static int unmount()
  {
    SPIFFS_unmount( &fs );
    checkSPIFFSErrors();
    return 0;
  }


  static FileHandle fopen( const char *filename, const char *mode )
  {
    using namespace Aurora::Logging;
    using namespace Chimera::Thread;

    /*-------------------------------------------------
    Mount protection
    -------------------------------------------------*/
    if ( !SPIFFS_mounted( &fs ) )
    {
      return std::numeric_limits<FileHandle>::max();
    }

    LockGuard<RecursiveMutex> lk( s_file_mtx );

    /*-------------------------------------------------
    Does the file exist already in the open list?
    -------------------------------------------------*/
    auto iter = s_open_files.find( filename );
    if ( iter != s_open_files.end() )
    {
      return iter->second;
    }

    /*-------------------------------------------------
    Doesn't exist, but can it be added?
    -------------------------------------------------*/
    if ( s_open_files.full() )
    {
      return std::numeric_limits<FileHandle>::max();
    }

    /*-------------------------------------------------
    Figure out the mode to open with
    -------------------------------------------------*/
    int flag = 0;
    if ( ( strcmp( mode, "w" ) == 0 ) || ( strcmp( mode, "wb" ) == 0 ) || ( strcmp( mode, "w+" ) == 0 )
         || ( strcmp( mode, "wb+" ) == 0 ) )
    {
      flag = SPIFFS_RDWR | SPIFFS_O_CREAT | SPIFFS_TRUNC;
    }
    else if ( ( strcmp( mode, "r" ) == 0 ) || ( strcmp( mode, "rb" ) == 0 ) )
    {
      flag = SPIFFS_O_RDONLY;
    }
    else
    {
      // Some mode combination was used that's not supported
      RT_HARD_ASSERT( false );
    }

    /*-------------------------------------------------
    Try to open the file
    -------------------------------------------------*/
    FileHandle ret_val = -1;

    s_open_files.insert( { filename, -1 } );
    iter = s_open_files.find( filename );

    iter->second = SPIFFS_open( &fs, filename, flag, 0 );
    if ( iter->second < 0 )
    {
      SPIFFS_LOG( LOG_DEBUG( "Failed to open %s with code %d\r\n", filename, iter->second ) );
      s_open_files.erase( iter );
    }
    else
    {
      ret_val = iter->second;
    }

    checkSPIFFSErrors();
    return ret_val;
  }


  static int fclose( FileHandle stream )
  {
    using namespace Aurora::Logging;
    using namespace Chimera::Thread;

    /*-------------------------------------------------
    Mount protection
    -------------------------------------------------*/
    if ( !SPIFFS_mounted( &fs ) )
    {
      return -1;
    }

    LockGuard<RecursiveMutex> lk( s_file_mtx );
    int err = 0;
    for ( auto iter = s_open_files.begin(); iter != s_open_files.end(); iter++ )
    {
      if ( iter->second == stream )
      {
        SPIFFS_fflush( &fs, stream );
        err = SPIFFS_close( &fs, stream );
        if ( err )
        {
          SPIFFS_LOG( LOG_DEBUG( "Failed to close %s with code %d\r\n", iter->first, err ) );
        }
        else
        {
          s_open_files.erase( iter );
        }
        break;
      }
    }

    checkSPIFFSErrors();
    return err;
  }


  static int fflush( FileHandle stream )
  {
    /*-------------------------------------------------
    Mount protection
    -------------------------------------------------*/
    if ( !SPIFFS_mounted( &fs ) )
    {
      return -1;
    }

    auto err = SPIFFS_fflush( &fs, stream );

    checkSPIFFSErrors();
    return err;
  }


  static size_t fread( void *ptr, size_t size, size_t count, FileHandle stream )
  {
    /*-------------------------------------------------
    Mount protection
    -------------------------------------------------*/
    if ( !SPIFFS_mounted( &fs ) )
    {
      return std::numeric_limits<FileHandle>::max();
    }

    int read_amount = SPIFFS_read( &fs, stream, ptr, size * count );

    checkSPIFFSErrors();
    return ( read_amount < 0 ) ? 0 : static_cast<size_t>( read_amount );
  }


  static size_t fwrite( const void *ptr, size_t size, size_t count, FileHandle stream )
  {
    /*-------------------------------------------------
    Mount protection
    -------------------------------------------------*/
    if ( !SPIFFS_mounted( &fs ) )
    {
      return std::numeric_limits<FileHandle>::max();
    }

    int write_amount = SPIFFS_write( &fs, stream, const_cast<void *>( ptr ), size * count );

    checkSPIFFSErrors();
    return ( write_amount < 0 ) ? 0 : static_cast<size_t>( write_amount );
  }


  static int fseek( FileHandle stream, size_t offset, size_t origin )
  {
    /*-------------------------------------------------
    Mount protection
    -------------------------------------------------*/
    if ( !SPIFFS_mounted( &fs ) )
    {
      return -1;
    }

    auto err = SPIFFS_lseek( &fs, stream, offset, origin );

    checkSPIFFSErrors();
    return err;
  }


  static size_t ftell( FileHandle stream )
  {
    /*-------------------------------------------------
    Mount protection
    -------------------------------------------------*/
    if ( !SPIFFS_mounted( &fs ) )
    {
      return std::numeric_limits<FileHandle>::max();
    }

    auto pos = SPIFFS_tell( &fs, stream );

    checkSPIFFSErrors();
    return ( pos < 0 ) ? 0 : static_cast<size_t>( pos );
  }


  static void frewind( FileHandle stream )
  {
    /*-------------------------------------------------
    Mount protection
    -------------------------------------------------*/
    if ( !SPIFFS_mounted( &fs ) )
    {
      return;
    }

    SPIFFS_lseek( &fs, stream, 0, SPIFFS_SEEK_SET );
    checkSPIFFSErrors();
  }


}  // namespace Aurora::FileSystem::SPIFFS
