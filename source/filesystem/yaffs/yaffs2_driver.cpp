/********************************************************************************
 *  File Name:
 *    yaffs2_driver.cpp
 *
 *  Description:
 *    Implementation of the YAFFS2 filesystem driver for Aurora
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
#include <Aurora/source/filesystem/yaffs/yaffs2_driver.hpp>

/* Chimera Includes */
#include <Chimera/assert>
#include <Chimera/thread>

/* ETL Includes */
#include <etl/flat_map.h>

/* Yaffs Includes */
#ifdef __cplusplus
extern "C"
{
#endif

#include "yportenv.h"
#include "yaffs_guts.h"

#ifdef __cplusplus
}
#endif


namespace Aurora::FileSystem::YAFFS
{
  /*-------------------------------------------------------------------------------
  Constants
  -------------------------------------------------------------------------------*/
  static constexpr uint32_t FORMAT_VALUE          = 0x1234;
  static constexpr uint32_t SPARE_BYTES_PER_CHUNK = 16;


  /*-------------------------------------------------------------------------------
  Static Data
  -------------------------------------------------------------------------------*/
  static bool s_initialized                          = false;   /**< Is the module initialized? */
  static const Aurora::Memory::Properties *sNORProps = nullptr; /**< External NOR flash properties */
  static Aurora::Flash::NOR::Driver sNORFlash;                  /**< Flash memory driver supporting the file system */
  static yaffs_dev sYaffsDevice;

  static uint32_t FormatOffset    = 0;
  static uint32_t BytesPerChunk   = 0;
  static uint32_t BytesPerBlock   = 0;
  static uint32_t ChunksPerBlock  = 0;
  static uint32_t BlocksInDevice  = 0;
  static uint32_t SpareAreaOffset = 0;

#define YNOR_PREMARKER ( 0xF6 )
#define YNOR_POSTMARKER ( 0xF0 )

  static Chimera::Thread::RecursiveMutex s_file_mtx;
  static etl::flat_map<const char *, int, 15> s_open_files;

  /*-------------------------------------------------------------------------------
  Static Function Declarations
  -------------------------------------------------------------------------------*/
  /*-------------------------------------------------
  YAFFS2 Hooks
  -------------------------------------------------*/
  static void install_driver( const Aurora::Flash::NOR::Chip_t dev );
  static u32 Block2Addr( yaffs_dev *dev, int blockNumber );
  static u32 Block2FormatAddr( yaffs_dev *dev, int blockNumber );
  static u32 Chunk2DataAddr( yaffs_dev *dev, int chunk_id );
  static u32 Chunk2SpareAddr( yaffs_dev *dev, int chunk_id );
  static void AndBytes( u8 *target, const u8 *src, int nbytes );
  static int WriteChunk( yaffs_dev *dev, int nand_chunk, const u8 *data, int data_len, const u8 *oob, int oob_len );
  static int ReadChunk( yaffs_dev *dev, int nand_chunk, u8 *data, int data_len, u8 *oob, int oob_len,
                        yaffs_ecc_result *ecc_result );
  static int FormatBlock( yaffs_dev *dev, int blockNumber );
  static int UnformatBlock( yaffs_dev *dev, int blockNumber );
  static int IsBlockFormatted( yaffs_dev *dev, int blockNumber );
  static int EraseBlock( yaffs_dev *dev, int blockNumber );
  static int MarkBad( yaffs_dev *dev, int block_no );
  static int CheckBad( yaffs_dev *dev, int block_no );
  static int Initialise( yaffs_dev *dev );
  static int Deinitialise( yaffs_dev *dev );

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

  /*-------------------------------------------------------------------------------
  Public Data
  -------------------------------------------------------------------------------*/
  extern const Interface implementation = { .mount   = ::Aurora::FileSystem::YAFFS::mount,
                                            .unmount = ::Aurora::FileSystem::YAFFS::unmount,
                                            .fopen   = ::Aurora::FileSystem::YAFFS::fopen,
                                            .fclose  = ::Aurora::FileSystem::YAFFS::fclose,
                                            .fflush  = ::Aurora::FileSystem::YAFFS::fflush,
                                            .fread   = ::Aurora::FileSystem::YAFFS::fread,
                                            .fwrite  = ::Aurora::FileSystem::YAFFS::fwrite,
                                            .fseek   = ::Aurora::FileSystem::YAFFS::fseek,
                                            .ftell   = ::Aurora::FileSystem::YAFFS::ftell,
                                            .frewind = ::Aurora::FileSystem::YAFFS::frewind };


  /*-------------------------------------------------------------------------------
  Public Functions
  -------------------------------------------------------------------------------*/
  bool attachDevice( const Aurora::Flash::NOR::Chip_t dev, const Chimera::SPI::Channel channel )
  {
    install_driver( dev );
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
  Static Function Implementation
  -------------------------------------------------------------------------------*/
  static size_t Block2FormatOffset( int blockNumber )
  {
    return 0;  // is this the right offset?????
  }


  static size_t Block2FormatChunk( int blockNumber )
  {
    // This might need to be +1
    return ( blockNumber * BytesPerBlock ) / BytesPerChunk;
  }


  static void install_driver( const Aurora::Flash::NOR::Chip_t dev )
  {
    using namespace Aurora::Flash;

    /*-------------------------------------------------
    Reset
    -------------------------------------------------*/
    memset( &sYaffsDevice, 0, sizeof( sYaffsDevice ) );

    /*-------------------------------------------------
    Figure out a few device properties
    -------------------------------------------------*/
    const auto props = Aurora::Flash::NOR::getProperties( dev );

    switch ( props->writeChunk )
    {
      case Aurora::Memory::Chunk::PAGE:
        BytesPerChunk = 512;
        break;

      case Aurora::Memory::Chunk::BLOCK:
        BytesPerChunk = props->blockSize;
        break;

      case Aurora::Memory::Chunk::SECTOR:
        BytesPerChunk = props->sectorSize;
        break;

      default:
        return;
        break;
    };

    switch ( props->eraseChunk )
    {
      case Aurora::Memory::Chunk::PAGE:
        BytesPerBlock = props->pageSize;
        break;

      case Aurora::Memory::Chunk::BLOCK:
        BytesPerBlock = props->blockSize;
        break;

      case Aurora::Memory::Chunk::SECTOR:
        BytesPerBlock = props->sectorSize;
        break;

      default:
        return;
        break;
    };

    size_t address_range = ( props->endAddress - props->startAddress );

    RT_HARD_ASSERT( BytesPerChunk != 0 );
    RT_HARD_ASSERT( BytesPerBlock != 0 );
    RT_HARD_ASSERT( BytesPerBlock >= BytesPerChunk );
    RT_HARD_ASSERT( address_range >= BytesPerBlock );

    ChunksPerBlock  = BytesPerBlock / BytesPerChunk;
    BlocksInDevice  = address_range / BytesPerBlock;
    SpareAreaOffset = BytesPerChunk - SPARE_BYTES_PER_CHUNK;

    /*-------------------------------------------------
    Configure device properties
    -------------------------------------------------*/
    sYaffsDevice.param.name                  = "root";
    sYaffsDevice.param.inband_tags           = 0;
    sYaffsDevice.param.total_bytes_per_chunk = BytesPerChunk;
    sYaffsDevice.param.spare_bytes_per_chunk = SPARE_BYTES_PER_CHUNK;
    sYaffsDevice.param.chunks_per_block      = ChunksPerBlock;
    sYaffsDevice.param.n_reserved_blocks     = 2;
    sYaffsDevice.param.start_block           = 0;                   // Can use block 0
    sYaffsDevice.param.end_block             = BlocksInDevice - 1;  // Last block
    sYaffsDevice.param.use_nand_ecc          = 0;                   // use YAFFS's ECC
    sYaffsDevice.param.disable_soft_del      = 1;
    sYaffsDevice.param.n_caches              = 10;
    sYaffsDevice.param.is_yaffs2             = 0;  // Example is in yaffs 1 mode. Get that working first.

    /*-------------------------------------------------
    Register interface functions
    -------------------------------------------------*/
    sYaffsDevice.drv.drv_write_chunk_fn  = WriteChunk;
    sYaffsDevice.drv.drv_read_chunk_fn   = ReadChunk;
    sYaffsDevice.drv.drv_erase_fn        = EraseBlock;
    sYaffsDevice.drv.drv_initialise_fn   = Initialise;
    sYaffsDevice.drv.drv_deinitialise_fn = Deinitialise;
    sYaffsDevice.drv.drv_mark_bad_fn     = MarkBad;
    sYaffsDevice.drv.drv_check_bad_fn    = CheckBad;

    /*-------------------------------------------------
    Assign user context if needed (multiple devices)
    -------------------------------------------------*/
    sYaffsDevice.driver_context = nullptr;

    /*-------------------------------------------------
    Register the device
    -------------------------------------------------*/
    yaffs_add_device( &sYaffsDevice );
  }


  static void AndBytes( u8 *target, const u8 *src, int nbytes )
  {
    while ( nbytes > 0 )
    {
      *target &= *src;
      target++;
      src++;
      nbytes--;
    }
  }


  static int WriteChunk( yaffs_dev *dev, int nand_chunk, const u8 *data, int data_len, const u8 *oob, int oob_len )
  {
    yaffs_spare *spare = ( yaffs_spare * )oob;
    yaffs_spare tmpSpare;

    if ( data && oob )
    {
      /* Write a pre-marker */
      memset( &tmpSpare, 0xff, sizeof( tmpSpare ) );
      tmpSpare.page_status = YNOR_PREMARKER;
      sNORFlash.write( nand_chunk, SpareAreaOffset, &tmpSpare, sizeof( tmpSpare ) );

      /* Write the data */
      sNORFlash.write( nand_chunk, 0, data, data_len );

      /* Write the real tags, but override the premarker */
      memcpy( &tmpSpare, spare, sizeof( tmpSpare ) );
      tmpSpare.page_status = YNOR_PREMARKER;
      sNORFlash.write( nand_chunk, SpareAreaOffset, &tmpSpare, sizeof( tmpSpare ) );

      /* Write the postmarker */
      tmpSpare.page_status = YNOR_POSTMARKER;
      sNORFlash.write( nand_chunk, SpareAreaOffset, &tmpSpare, sizeof( tmpSpare ) );
    }
    else if ( spare )
    {
      /* This has to be RMW to handle NOR-ness */
      sNORFlash.read( nand_chunk, SpareAreaOffset, &tmpSpare, sizeof( tmpSpare ) );

      AndBytes( ( u8 * )&tmpSpare, ( u8 * )spare, sizeof( tmpSpare ) );

      sNORFlash.write( nand_chunk, SpareAreaOffset, &tmpSpare, sizeof( tmpSpare ) );
    }
    else
    {
      Chimera::insert_debug_breakpoint();
    }
  }


  static int ReadChunk( yaffs_dev *dev, int nand_chunk, u8 *data, int data_len, u8 *oob, int oob_len,
                        yaffs_ecc_result *ecc_result )
  {
    yaffs_spare *spare = ( yaffs_spare * )oob;
    static_assert( sizeof( yaffs_spare ) == SPARE_BYTES_PER_CHUNK );

    if ( data )
    {
      sNORFlash.read( nand_chunk, 0, data, dev->param.total_bytes_per_chunk );
    }

    if ( oob )
    {
      sNORFlash.read( nand_chunk, SpareAreaOffset, spare, oob_len );

      if ( spare->page_status == YNOR_POSTMARKER )
      {
        spare->page_status = 0xff;
      }
      else if ( spare->page_status != 0xff && ( spare->page_status | YNOR_PREMARKER ) != 0xff )
      {
        spare->page_status = YNOR_PREMARKER;
      }
    }

    if ( ecc_result )
    {
      *ecc_result = YAFFS_ECC_RESULT_NO_ERROR;
    }

    return YAFFS_OK;
  }


  static int FormatBlock( yaffs_dev *dev, int blockNumber )
  {
    Chimera::insert_debug_breakpoint();

    size_t chunkNumber = Block2FormatChunk( blockNumber );
    size_t offset      = Block2FormatOffset( blockNumber );

    sNORFlash.erase( blockNumber );
    sNORFlash.write( chunkNumber, offset, &FORMAT_VALUE, sizeof( FORMAT_VALUE ) );

    return YAFFS_OK;
  }


  static int UnformatBlock( yaffs_dev *dev, int blockNumber )
  {
    Chimera::insert_debug_breakpoint();
    size_t chunkNumber = Block2FormatChunk( blockNumber );
    size_t offset      = Block2FormatOffset( blockNumber );

    u32 formatVal = 0;
    sNORFlash.write( chunkNumber, offset, &formatVal, sizeof( formatVal ) );

    return YAFFS_OK;
  }


  static int IsBlockFormatted( yaffs_dev *dev, int blockNumber )
  {
    Chimera::insert_debug_breakpoint();
    size_t chunkNumber = Block2FormatChunk( blockNumber );
    size_t offset      = Block2FormatOffset( blockNumber );

    u32 formatVal = 0;

    sNORFlash.read( chunkNumber, offset, &formatVal, sizeof( formatVal ) );
    return ( formatVal == FORMAT_VALUE );
  }


  static int EraseBlock( yaffs_dev *dev, int blockNumber )
  {
    if ( blockNumber < 0 || blockNumber >= BlocksInDevice )
    {
      return YAFFS_FAIL;
    }
    else
    {
      UnformatBlock( dev, blockNumber );
      FormatBlock( dev, blockNumber );
    }
  }


  static int MarkBad( yaffs_dev *dev, int block_no )
  {
    return YAFFS_OK;
  }

  static int CheckBad( yaffs_dev *dev, int block_no )
  {
    return YAFFS_OK;
  }

  static int Initialise( yaffs_dev *dev )
  {
    for ( auto i = dev->param.start_block; i <= dev->param.end_block; i++ )
    {
      if ( !IsBlockFormatted( dev, i ) )
      {
        FormatBlock( dev, i );
      }
    }

    return YAFFS_OK;
  }


  static int Deinitialise( yaffs_dev *dev )
  {
    return YAFFS_OK;
  }


  static int mount()
  {
    using namespace Aurora::Logging;

    /*-------------------------------------------------
    Input Protection
    -------------------------------------------------*/
    RT_HARD_ASSERT( sNORProps );

    /*-------------------------------------------------
    Try mounting. It's possible to get a clean chip,
    which will need some formatting before mounting.
    -------------------------------------------------*/
    int err = yaffs_mount( "root" );
    // if ( err )
    // {
    //   /*-------------------------------------------------
    //   Attempt to quick format
    //   -------------------------------------------------*/
    //   LOG_DEBUG( "Initial mount failed with code %d. Reformatting.\r\n", err );
    //   err = lfs_format( s_fs, s_fs_cfg );
    //   if ( err )
    //   {
    //     LOG_DEBUG( "Reformatting failed with code %d\r\n", err );
    //   }

    //   /*-------------------------------------------------
    //   Retry the mount
    //   -------------------------------------------------*/
    //   err = lfs_mount( s_fs, s_fs_cfg );
    // }

    /*-------------------------------------------------
    Log the mount status for posterity
    -------------------------------------------------*/
    if ( err )
    {
      LOG_DEBUG( "Mount failed with code %d\r\n", yaffs_get_error() );
    }
    else
    {
      LOG_DEBUG( "File system mounted\r\n" );
    }

    return err == 0;
  }


  static int unmount()
  {
    return yaffs_unmount( "root" );
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
      flag = O_WRONLY | O_CREAT;
    }
    else if ( ( strcmp( mode, "w+" ) == 0 ) || ( strcmp( mode, "wb+" ) == 0 ) )
    {
      flag = O_WRONLY | O_CREAT | O_APPEND;
    }
    else if ( ( strcmp( mode, "r" ) == 0 ) || ( strcmp( mode, "rb" ) == 0 ) )
    {
      flag = O_RDONLY;
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

    iter->second = yaffs_open( filename, flag, S_IFMT );
    if ( iter->second < 0 )
    {
      LOG_DEBUG( "Failed to open %s with code %d\r\n", filename, iter->second );
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
        err = yaffs_close( iter->second );
        if ( err )
        {
          LOG_DEBUG( "Failed to close %s with code %d\r\n", iter->first, err );
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
    return yaffs_flush( stream );
  }


  static size_t fread( void *ptr, size_t size, size_t count, FileHandle stream )
  {
    return yaffs_read( stream, ptr, size * count );
  }


  static size_t fwrite( const void *ptr, size_t size, size_t count, FileHandle stream )
  {
    return yaffs_write( stream, ptr, size * count );
  }


  static int fseek( FileHandle stream, size_t offset, size_t origin )
  {
    return yaffs_lseek( stream, offset, origin );
  }


  static size_t ftell( FileHandle stream )
  {
    // Currently doesn't appear to be implemented?
    RT_HARD_ASSERT( false );
  }


  static void frewind( FileHandle stream )
  {
    yaffs_lseek( stream, 0, SEEK_SET );
  }

}  // namespace Aurora::FileSystem::YAFFS
