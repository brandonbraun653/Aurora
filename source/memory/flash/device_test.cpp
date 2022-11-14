/******************************************************************************
 *  File Name:
 *    device_test.cpp
 *
 *  Description:
 *    Generic flash device testing module
 *
 *  2022 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/memory>
#include <Chimera/assert>
#include <etl/crc.h>
#include <etl/random.h>

namespace Aurora::Memory::Flash
{
  /*---------------------------------------------------------------------------
  Structures
  ---------------------------------------------------------------------------*/
#pragma pack( push, 1 )
  struct NVMHeader
  {
    uint32_t crc32;
  };
#pragma pack( pop )

  /*---------------------------------------------------------------------------
  Static Functions
  ---------------------------------------------------------------------------*/
  /**
   * @brief Generates random data in the device write buffer
   *
   * @param buffer  Buffer to fill
   * @param bytes   How many bytes to generate
   */
  static void genRandomData( void *const buffer, const size_t bytes )
  {
    etl::random_xorshift rng( Chimera::millis() );

    size_t   bytes_left = bytes;
    uint8_t *bPtr       = reinterpret_cast<uint8_t *const>( buffer );

    while( bytes_left )
    {
      *bPtr = static_cast<uint8_t>( rng.range( 0, 255 ) );
      bytes_left--;
      bPtr++;
    }
  }


  /**
   * @brief Generates a CRC of the data managed by the header
   *
   * @param hdr     Header of data
   * @param bytes   How many bytes, including the header
   * @return uint32_t
   */
  static uint32_t genCRC( const NVMHeader *const hdr, const size_t bytes )
  {
    /*-----------------------------------------------------------------------
    Input Protection
    -----------------------------------------------------------------------*/
    RT_DBG_ASSERT( hdr );
    if ( bytes < sizeof( NVMHeader ) )
    {
      return 0;
    }

    /*-----------------------------------------------------------------------
    Calculate the CRC and return its value
    -----------------------------------------------------------------------*/
    etl::crc32     crc_calculator;
    const uint8_t *data       = reinterpret_cast<const uint8_t *const>( hdr ) + sizeof( NVMHeader::crc32 );
    size_t         bytes_left = bytes - sizeof( NVMHeader::crc32 );

    while ( bytes_left )
    {
      crc_calculator.add( *data );
      data++;
      bytes_left--;
    }

    return crc_calculator.value();
  }


  /**
   * @brief Checks if the data managed by the header is valid
   *
   * @param hdr     Header of data
   * @param bytes   How many bytes, including the header
   * @return bool
   */
  static bool isValid( const NVMHeader *const hdr, const size_t bytes )
  {
    RT_DBG_ASSERT( hdr );
    return ( genCRC( hdr, bytes ) == hdr->crc32 );
  }


  /*---------------------------------------------------------------------------
  Device Test Implementation
  ---------------------------------------------------------------------------*/
  DeviceTest::DeviceTest() : mCfg( {} )
  {
  }


  DeviceTest::~DeviceTest()
  {
  }


  void DeviceTest::initialize( Config &cfg )
  {
    RT_DBG_ASSERT( cfg.dut );
    RT_DBG_ASSERT( cfg.writeBuffer );
    RT_DBG_ASSERT( cfg.readBuffer );
    RT_DBG_ASSERT( cfg.bufferSize );

    mCfg = cfg;
    memset( mCfg.writeBuffer, 0, mCfg.bufferSize );
    memset( mCfg.readBuffer, 0, mCfg.bufferSize );
  }


  Aurora::Memory::Status DeviceTest::pageAccess( const size_t page, const bool erase )
  {
    size_t address     = page * mCfg.pageSize;
    size_t erase_chunk = address / mCfg.eraseSize;
    auto   result      = Status::ERR_OK;

    if( erase )
    {
      result = result | this->erase( erase_chunk );
    }

    result = result | dutAccess( address, mCfg.pageSize );

    return result;
  }


  Aurora::Memory::Status DeviceTest::blockAccess( const size_t block, const bool erase )
  {
    return dutAccess( ( block * mCfg.blockSize ), mCfg.blockSize );
  }


  Aurora::Memory::Status DeviceTest::sectorAccess( const size_t sector, const bool erase )
  {
    return dutAccess( ( sector * mCfg.sectorSize ), mCfg.sectorSize );
  }


  Aurora::Memory::Status DeviceTest::randomAccess( const size_t limit, const bool erase )
  {
  }


  Aurora::Memory::Status DeviceTest::erase( const size_t chunk )
  {
    auto result = Aurora::Memory::Status::ERR_OK;

    result = result | mCfg.dut->erase( ( chunk * mCfg.eraseSize ), mCfg.eraseSize );
    result = result | mCfg.dut->pendEvent( Event::MEM_ERASE_COMPLETE, Chimera::Thread::TIMEOUT_BLOCK );

    return result;
  }


  Aurora::Memory::Status DeviceTest::dutAccess( const size_t address, const size_t size )
  {
    /*-------------------------------------------------------------------------
    Input Validation
    -------------------------------------------------------------------------*/
    if ( ( ( address + size ) < mCfg.maxAddress ) && ( size > mCfg.bufferSize ) )
    {
      return Status::ERR_BAD_ARG;
    }

    /*-------------------------------------------------------------------------
    Generate the data to write
    -------------------------------------------------------------------------*/
    uint8_t *data = reinterpret_cast<uint8_t *>( mCfg.writeBuffer );
    genRandomData( data + sizeof( NVMHeader ), size - sizeof( NVMHeader ) );

    NVMHeader *writeHeader = reinterpret_cast<NVMHeader *>( mCfg.writeBuffer );
    writeHeader->crc32     = genCRC( writeHeader, size );

    /*-------------------------------------------------------------------------
    Write/Read the data back
    -------------------------------------------------------------------------*/
    auto result = mCfg.dut->write( address, mCfg.writeBuffer, size );
    result      = result | mCfg.dut->read( address, mCfg.readBuffer, size );

    /*-------------------------------------------------------------------------
    Validate
    -------------------------------------------------------------------------*/
    NVMHeader *readHeader = reinterpret_cast<NVMHeader *>( mCfg.readBuffer );

    if ( ( result == Status::ERR_OK ) && isValid( readHeader, size ) )
    {
      return Aurora::Memory::Status::ERR_OK;
    }
    else
    {
      return Aurora::Memory::Status::ERR_FAIL;
    }
  }

}  // namespace Aurora::Memory::Flash
