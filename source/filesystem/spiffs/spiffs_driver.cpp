/********************************************************************************
 *  File Name:
 *    spiffs_driver.cpp
 *
 *  Description:
 *    Filesystem implementation redirects into the SPIFFS driver
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* STL Includes */
#include <cstdint>

/* Aurora Include */
#include <Aurora/memory>
#include <Aurora/filesystem>
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


namespace Aurora::FileSystem::SPIFFS
{
  /*-------------------------------------------------------------------------------
  Constants
  -------------------------------------------------------------------------------*/
  static const std::string_view boot_file = "!@#$%^&*()_spiffs_boot_log.bin";
  static const std::string_view drive_name = "spiffs_fs_drive";

  /*-------------------------------------------------------------------------------
  Structures
  -------------------------------------------------------------------------------*/
  struct BootData
  {
    uint32_t boot_count;
    uint8_t drive_name[ 32 ];
  };

  /*-------------------------------------------------------------------------------
  Static Functions
  -------------------------------------------------------------------------------*/
  /*-------------------------------------------------
  Posix API
  -------------------------------------------------*/
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

  /*-------------------------------------------------
  SPIFFS API
  -------------------------------------------------*/
  static s32_t spiffs_read( u32_t addr, u32_t size, u8_t *dst );
  static s32_t spiffs_write( u32_t addr, u32_t size, u8_t *src );
  static s32_t spiffs_erase( u32_t addr, u32_t size );

  extern "C"
  {
    extern void SPIFFS_fs_lock( void *const fs );
    extern void SPIFFS_fs_unlock( void *const fs );
  }

  /*-------------------------------------------------------------------------------
  Static Data
  -------------------------------------------------------------------------------*/
  static bool s_initialized                          = false;   /**< Is the module initialized? */
  static const Aurora::Memory::Properties *sNORProps = nullptr; /**< External NOR flash properties */
  static Aurora::Flash::NOR::Driver sNORFlash;                  /**< Flash memory driver supporting the file system */
  static Aurora::Flash::NOR::Chip_t sDevice;
  static spiffs fs;
  static Chimera::Thread::RecursiveMutex s_file_mtx;
  static etl::flat_map<const char *, spiffs_file, 15> s_open_files;


#define LOG_PAGE_SIZE 256

  static u8_t spiffs_work_buf[ LOG_PAGE_SIZE * 2 ];
  static u8_t spiffs_fds[ 32 * 4 ];
  static u8_t spiffs_cache_buf[ ( LOG_PAGE_SIZE + 32 ) * 4 ];


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
  Driver Specific Implementation
  -------------------------------------------------------------------------------*/
  bool attachDevice( const Aurora::Flash::NOR::Chip_t dev, const Chimera::SPI::Channel channel )
  {
    sNORProps = Aurora::Flash::NOR::getProperties( dev );
    return sNORFlash.configure( dev, channel );
  }


  bool fullChipErase( const size_t timeout )
  {
    using namespace Aurora::Memory;

    /*-------------------------------------------------
    Issue the erase command
    -------------------------------------------------*/
    if ( sNORFlash.erase() != Status::ERR_OK )
    {
      return false;
    }

    /*-------------------------------------------------
    Wait for the erase to complete
    -------------------------------------------------*/
    return sNORFlash.pendEvent( Event::MEM_ERASE_COMPLETE, timeout ) == Status::ERR_OK;
  }


  /*-------------------------------------------------------------------------------
  POSIX API Implementation
  -------------------------------------------------------------------------------*/
  static int mount()
  {
    using namespace Aurora::Logging;
    using namespace Aurora::Memory;

    namespace fileSys = ::Aurora::FileSystem;

    /*-------------------------------------------------
    Input Protection
    -------------------------------------------------*/
    RT_HARD_ASSERT( sNORProps );
    RT_HARD_ASSERT( sNORProps->pageSize == LOG_PAGE_SIZE );

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
    cfg.hal_read_f       = spiffs_read;
    cfg.hal_write_f      = spiffs_write;
    cfg.hal_erase_f      = spiffs_erase;

    /*-------------------------------------------------
    Try mounting. It's possible to get a clean chip,
    which will need some formatting before mounting.
    -------------------------------------------------*/
    for ( auto x = 0; x < 3; x++ )
    {
      getRootSink()->flog( Level::LVL_DEBUG, "Mounting filesystem\r\n" );
      SPIFFS_mount( &fs, &cfg, spiffs_work_buf, spiffs_fds, sizeof( spiffs_fds ), spiffs_cache_buf, sizeof( spiffs_cache_buf ),
                    0 );

      if ( SPIFFS_mounted( &fs ) )
      {
        getRootSink()->flog( Level::LVL_DEBUG, "Filesystem attached, validating...\r\n" );

        /*-------------------------------------------------
        Validate the drive is working properly by dumping
        some data to it and ensuring it comes back ok.
        -------------------------------------------------*/
        BootData write_data, read_data;
        memset( &write_data, 0, sizeof( BootData ) );
        memset( &read_data, 1, sizeof( BootData ) );

        write_data.boot_count = 351;
        memcpy( write_data.drive_name, drive_name.data(), drive_name.length() );

        FileHandle fd = fileSys::fopen( boot_file.data(), "w+" );
        fileSys::fwrite( &write_data, 1, sizeof( BootData ), fd );
        fileSys::fclose( fd );

        fd = fileSys::fopen( boot_file.data(), "rb" );
        fileSys::frewind( fd );
        fileSys::fread( &read_data, 1, sizeof( BootData ), fd );
        fileSys::fclose( fd );

        if( ( memcmp( &write_data, &read_data, sizeof(BootData)) == 0 ) && ( SPIFFS_errno( &fs ) == SPIFFS_OK ) )
        {
          getRootSink()->flog( Level::LVL_DEBUG, "Ok\r\n" );
          return 0;
        }
        else
        {
          getRootSink()->flog( Level::LVL_DEBUG, "Cannot access filesystem. Assuming corrupted. Erasing device...\r\n" );
          SPIFFS_unmount( &fs );
          sNORFlash.erase( 10000 );
          getRootSink()->flog( Level::LVL_DEBUG, "Erased\r\n" );
        }
      }
      else
      {
        /*-------------------------------------------------
        Can immediately attempt to format. Assuming either
        a corrupt or blank chip.
        -------------------------------------------------*/
        getRootSink()->flog( Level::LVL_DEBUG, "Mount failed. Formatting...\r\n" );
        SPIFFS_format( &fs );
      }
    }

    return -1;
  }


  static int unmount()
  {
    SPIFFS_unmount( &fs );
    return 0;
  }


  static FileHandle fopen( const char *filename, const char *mode )
  {
    using namespace Aurora::Logging;
    using namespace Chimera::Thread;

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
    if ( ( strcmp( mode, "w" ) == 0 ) || ( strcmp( mode, "wb" ) == 0 ) )
    {
      flag = SPIFFS_RDWR | SPIFFS_O_CREAT;
    }
    else if ( ( strcmp( mode, "w+" ) == 0 ) || ( strcmp( mode, "wb+" ) == 0 ) )
    {
      flag = SPIFFS_RDWR | SPIFFS_O_CREAT | SPIFFS_O_APPEND;
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
    s_open_files.insert( { filename, -1 } );
    iter = s_open_files.find( filename );

    iter->second = SPIFFS_open( &fs, filename, flag, 0 );
    if ( iter->second < 0 )
    {
      getRootSink()->flog( Level::LVL_DEBUG, "Failed to open %s with code %d\r\n", filename, iter->second );
      s_open_files.erase( iter );
      return -1;
    }

    return iter->second;
  }


  static int fclose( FileHandle stream )
  {
    using namespace Aurora::Logging;
    using namespace Chimera::Thread;

    LockGuard<RecursiveMutex> lk( s_file_mtx );
    int err = 0;
    for ( auto iter = s_open_files.begin(); iter != s_open_files.end(); iter++ )
    {
      if ( iter->second == stream )
      {
        err = SPIFFS_close( &fs, iter->second );
        if ( err )
        {
          getRootSink()->flog( Level::LVL_DEBUG, "Failed to close %s with code %d\r\n", iter->first, err );
        }
        else
        {
          s_open_files.erase( iter );
        }
        break;
      }
    }

    return err;
  }


  static int fflush( FileHandle stream )
  {
    return SPIFFS_fflush( &fs, stream );
  }


  static size_t fread( void *ptr, size_t size, size_t count, FileHandle stream )
  {
    int read_amount = SPIFFS_read( &fs, stream, ptr, size * count );
    if ( read_amount < 0 )
    {
      return 0;
    }
    else
    {
      return static_cast<size_t>( read_amount );
    }
  }


  static size_t fwrite( const void *ptr, size_t size, size_t count, FileHandle stream )
  {
    int write_amount = SPIFFS_write( &fs, stream, const_cast<void *>( ptr ), size * count );
    if ( write_amount < 0 )
    {
      return 0;
    }
    else
    {
      return static_cast<size_t>( write_amount );
    }
  }


  static int fseek( FileHandle stream, size_t offset, size_t origin )
  {
    return SPIFFS_lseek( &fs, stream, offset, origin );
  }


  static size_t ftell( FileHandle stream )
  {
    return SPIFFS_tell( &fs, stream );
  }


  static void frewind( FileHandle stream )
  {
    SPIFFS_lseek( &fs, stream, 0, SPIFFS_SEEK_SET );
  }


  /*-------------------------------------------------------------------------------
  SPIFFS API Implementation
  -------------------------------------------------------------------------------*/
  extern "C"
  {
    extern void SPIFFS_fs_lock( void *const fs )
    {
      s_file_mtx.lock();
    }


    extern void SPIFFS_fs_unlock( void *const fs )
    {
      s_file_mtx.unlock();
    }
  }


  static s32_t spiffs_read( u32_t addr, u32_t size, u8_t *dst )
  {
    using namespace Aurora::Flash::NOR;

    size_t chunk  = 0;
    size_t offset = 0;

    if ( address2WriteChunkOffset( sDevice, addr, &chunk, &offset ) )
    {
      sNORFlash.read( chunk, offset, dst, size );
      return SPIFFS_OK;
    }
    else
    {
      return -1;
    }
  }


  static s32_t spiffs_write( u32_t addr, u32_t size, u8_t *src )
  {
    using namespace Aurora::Flash::NOR;

    size_t chunk  = 0;
    size_t offset = 0;

    if ( address2WriteChunkOffset( sDevice, addr, &chunk, &offset ) )
    {
      sNORFlash.write( chunk, offset, src, size );
      return SPIFFS_OK;
    }
    else
    {
      return -1;
    }
  }


  static s32_t spiffs_erase( u32_t addr, u32_t size )
  {
    using namespace Aurora::Memory;
    using namespace Chimera::Watchdog;

    /*-------------------------------------------------
    Input protection
    -------------------------------------------------*/
    if ( !sNORProps || !size )
    {
      return -1;
    }

    /*-------------------------------------------------
    Figure out what the erasure unit is on the device
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
    Is the start address aligned with the erasure unit?
    -------------------------------------------------*/
    if ( !erase_size || ( ( addr % erase_size ) != 0 ) )
    {
      return -1;
    }

    /*-------------------------------------------------
    Erase the required number of chunks
    -------------------------------------------------*/
    size_t chunks_to_erase = ( size / erase_size ) + 1u;
    size_t start_chunk     = addr / erase_size;

    for ( size_t chunk_id = start_chunk; chunk_id < ( start_chunk + chunks_to_erase ); chunk_id++ )
    {
      if ( ( sNORFlash.erase( chunk_id ) != Status::ERR_OK )
           || ( sNORFlash.pendEvent( Event::MEM_ERASE_COMPLETE, sNORProps->blockEraseDelay ) != Status::ERR_OK ) )
      {
        return -1;
      }
    }

    return SPIFFS_OK;
  }

}  // namespace Aurora::FileSystem::SPIFFS
