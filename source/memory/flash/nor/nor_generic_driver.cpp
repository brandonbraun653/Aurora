/******************************************************************************
 *  File Name:
 *    nor_generic_driver.cpp
 *
 *  Description:
 *    NOR flash generic driver implementation
 *
 *  2021-2022 | Brandon Braun | brandonbraun653@gmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/logging>
#include <Aurora/memory>
#include <Aurora/source/memory/flash/jedec/jedec_cfi_cmds.hpp>
#include <Aurora/source/memory/flash/nor/manufacturer/nor_adesto.hpp>
#include <Aurora/utility>
#include <Chimera/assert>
#include <Chimera/common>
#include <Chimera/event>
#include <Chimera/spi>
#include <Chimera/thread>

namespace Aurora::Memory::Flash::NOR
{
  /*---------------------------------------------------------------------------
  Constants
  ---------------------------------------------------------------------------*/
  static constexpr bool LOG_ENABLE = false;
#define NOR_LOG( x )          \
  if constexpr ( LOG_ENABLE ) \
  {                           \
    ( x );                    \
  }

  /*---------------------------------------------------------------------------
  Static Functions
  ---------------------------------------------------------------------------*/
  bool unitChunk2Address( const size_t unitSize, const size_t unitId, const size_t maxAddress, size_t *address )
  {
    size_t physicalAddress = unitSize * unitId;
    bool   isValid         = physicalAddress < maxAddress;

    if ( isValid && address )
    {
      *address = physicalAddress;
    }

    return isValid;
  }

  /*---------------------------------------------------------------------------
  Public Methods
  ---------------------------------------------------------------------------*/
  const Aurora::Memory::Properties *getProperties( const Chip_t device )
  {
    /*-------------------------------------------------------------------------
    Input Protection
    -------------------------------------------------------------------------*/
    if ( device >= Chip::NUM_OPTIONS )
    {
      return nullptr;
    }

    /*-------------------------------------------------------------------------
    Look up the device in the appropriate table
    -------------------------------------------------------------------------*/
    if ( ( device >= Chip::ADESTO_START ) && ( device < Chip::ADESTO_END ) )
    {
      size_t idx = static_cast<size_t>( device - Chip::ADESTO_START );
      RT_HARD_ASSERT( idx < ARRAY_COUNT( Adesto::ChipProperties ) );
      return &Adesto::ChipProperties[ idx ];
    }
    // else if( <some other device> )
    else
    {
      return nullptr;
    }
  }


  bool page2Address( const Chip_t device, const size_t page, size_t *const address )
  {
    /*-------------------------------------------------------------------------
    Input Protection
    -------------------------------------------------------------------------*/
    auto props = getProperties( device );
    if ( !props )
    {
      return false;
    }

    /*-------------------------------------------------------------------------
    Calculate the address
    -------------------------------------------------------------------------*/
    return unitChunk2Address( props->pageSize, page, props->endAddress, address );
  }


  bool block2Address( const Chip_t device, const size_t block, size_t *const address )
  {
    /*-------------------------------------------------------------------------
    Input Protection
    -------------------------------------------------------------------------*/
    auto props = getProperties( device );
    if ( !props )
    {
      return false;
    }

    /*-------------------------------------------------------------------------
    Calculate the address
    -------------------------------------------------------------------------*/
    return unitChunk2Address( props->blockSize, block, props->endAddress, address );
  }


  bool sector2Address( const Chip_t device, const size_t sector, size_t *const address )
  {
    /*-------------------------------------------------------------------------
    Input Protection
    -------------------------------------------------------------------------*/
    auto props = getProperties( device );
    if ( !props )
    {
      return false;
    }

    /*-------------------------------------------------------------------------
    Calculate the address
    -------------------------------------------------------------------------*/
    return unitChunk2Address( props->sectorSize, sector, props->endAddress, address );
  }


  bool address2WriteChunkOffset( const Chip_t device, const size_t address, size_t *const chunk, size_t *const offset )
  {
    using namespace Aurora::Memory;

    /*-------------------------------------------------------------------------
    Input Protection
    -------------------------------------------------------------------------*/
    const Properties *props = nullptr;

    if ( !chunk || !offset )
    {
      return false;
    }
    else if ( props = getProperties( device ); !props )
    {
      return false;
    }

    /*-------------------------------------------------------------------------
    Figure out the chunk ID in use
    -------------------------------------------------------------------------*/
    size_t write_size         = 0;
    size_t chunk_id           = 0;
    size_t chunk_base_address = 0;

    switch ( props->writeChunk )
    {
      case Chunk::PAGE:
        write_size = props->pageSize;
        break;

      case Chunk::BLOCK:
        write_size = props->blockSize;
        break;

      case Chunk::SECTOR:
        write_size = props->sectorSize;
        break;

      default:
        return false;
        break;
    }

    if ( write_size == 0 )
    {
      return false;
    }

    chunk_id           = address / write_size;
    chunk_base_address = chunk_id * write_size;

    /*-------------------------------------------------------------------------
    Figure out the byte offset inside the chunk
    -------------------------------------------------------------------------*/
    size_t byte_offset = 0;
    if ( address < chunk_base_address )
    {
      return false;
    }

    byte_offset = address - chunk_base_address;

    /*-------------------------------------------------------------------------
    Give the data back to the user
    -------------------------------------------------------------------------*/
    *chunk  = chunk_id;
    *offset = byte_offset;
    return true;
  }


  /*---------------------------------------------------------------------------
  Device Driver Implementation
  ---------------------------------------------------------------------------*/
  Driver::Driver() :
      mChip( Chip::UNKNOWN ), mAttr( {} ), mProps( nullptr ), mSPIChannel( Chimera::SPI::Channel::NOT_SUPPORTED ),
      mSPI( nullptr ), mCS( nullptr ), cmdBuffer( {} )
  {
  }


  Driver::~Driver()
  {
  }


  /*---------------------------------------------------------------------------
  Driver: Generic Memory Interface
  ---------------------------------------------------------------------------*/
  Aurora::Memory::Status Driver::open( const DeviceAttr *const attributes )
  {
    RT_DBG_ASSERT( attributes );
    RT_DBG_ASSERT( attributes->eraseSize );
    RT_DBG_ASSERT( attributes->readSize );
    RT_DBG_ASSERT( attributes->writeSize );

    mAttr = *attributes;
    return Aurora::Memory::Status::ERR_OK;
  }


  Aurora::Memory::Status Driver::close()
  {
    return Aurora::Memory::Status::ERR_OK;
  }


  Aurora::Memory::Status Driver::write( const size_t chunk, const size_t offset, const void *const data, const size_t length )
  {
    return this->write( ( ( mAttr.writeSize * chunk ) + offset ), data, length );
  }


  Aurora::Memory::Status Driver::write( const size_t address, const void *const data, const size_t length )
  {
    using namespace Aurora::Logging;
    using namespace Aurora::Memory;
    using namespace Chimera::Thread;

    /*-------------------------------------------------------------------------
    Input Protection
    -------------------------------------------------------------------------*/
    LockGuard _driverLock( *this );
    if ( !data || !length || ( ( address + length ) > mProps->endAddress ) )
    {
      NOR_LOG( LOG_ERROR( "Bad argument\r\n" ) );
      return Aurora::Memory::Status::ERR_BAD_ARG;
    }

    /*-------------------------------------------------------------------------
    Write enable command must be sent before sending write command
    -------------------------------------------------------------------------*/
    NOR_LOG( LOG_DEBUG( "Write %d bytes to address 0x%.8X\r\n", length, address ) );
    issueWriteEnable();

    /*-----------------------------------------------------------------------------
    Initialize the command sequence
    -----------------------------------------------------------------------------*/
    cmdBuffer[ 0 ] = CFI::PAGE_PROGRAM;
    cmdBuffer[ 1 ] = ( address & ADDRESS_BYTE_3_MSK ) >> ADDRESS_BYTE_3_POS;
    cmdBuffer[ 2 ] = ( address & ADDRESS_BYTE_2_MSK ) >> ADDRESS_BYTE_2_POS;
    cmdBuffer[ 3 ] = ( address & ADDRESS_BYTE_1_MSK ) >> ADDRESS_BYTE_1_POS;

    /*-------------------------------------------------------------------------
    Perform the SPI transaction
    -------------------------------------------------------------------------*/
    Chimera::Thread::LockGuard _spilock( *mSPI );
    auto                       result = Chimera::Status::OK;

    // Enable the NOR chip
    result |= mSPI->assignChipSelect( mCS );
    result |= mSPI->setChipSelectControlMode( Chimera::SPI::CSMode::MANUAL );
    result |= mSPI->setChipSelect( Chimera::GPIO::State::LOW );

    // Tell NOR controller which address to write into
    result |= mSPI->writeBytes( cmdBuffer.data(), CFI::PAGE_PROGRAM_OPS_LEN );
    result |= mSPI->await( Chimera::Event::Trigger::TRIGGER_TRANSFER_COMPLETE, TIMEOUT_BLOCK );

    // Dump the data
    result |= mSPI->writeBytes( data, length );
    result |= mSPI->await( Chimera::Event::Trigger::TRIGGER_TRANSFER_COMPLETE, TIMEOUT_BLOCK );

    // Release the SPI and disable the NOR chip
    result |= mSPI->setChipSelect( Chimera::GPIO::State::HIGH );

    /*-------------------------------------------------------------------------
    Wait for the hardware to finish the operation
    -------------------------------------------------------------------------*/
    auto status = pendEvent( Event::MEM_WRITE_COMPLETE, TIMEOUT_BLOCK );

    if ( ( result == Chimera::Status::OK ) && ( status == Status::ERR_OK ) )
    {
      return Status::ERR_OK;
    }
    else
    {
      return Status::ERR_DRIVER_ERR;
    }
  }


  Aurora::Memory::Status Driver::read( const size_t chunk, const size_t offset, void *const data, const size_t length )
  {
    return this->read( ( ( mAttr.readSize * chunk ) + offset ), data, length );
  }


  Aurora::Memory::Status Driver::read( const size_t address, void *const data, const size_t length )
  {
    using namespace Aurora::Logging;
    using namespace Aurora::Memory;
    using namespace Chimera::Thread;

    /*-------------------------------------------------------------------------
    Input Protection
    -------------------------------------------------------------------------*/
    LockGuard _driverLock( *this );
    if ( !data || !length || ( ( address + length ) > mProps->endAddress ) )
    {
      NOR_LOG( LOG_ERROR( "Bad argument\r\n" ) );
      return Aurora::Memory::Status::ERR_BAD_ARG;
    }

    /*-------------------------------------------------------------------------
    Init the cmd sequence. The high speed cmd works for all frequency ranges.
    -------------------------------------------------------------------------*/
    NOR_LOG( LOG_DEBUG( "Read %d bytes from address 0x%.8X\r\n", length, address ) );

    cmdBuffer[ 0 ] = CFI::READ_ARRAY_HS;
    cmdBuffer[ 1 ] = ( address & ADDRESS_BYTE_3_MSK ) >> ADDRESS_BYTE_3_POS;
    cmdBuffer[ 2 ] = ( address & ADDRESS_BYTE_2_MSK ) >> ADDRESS_BYTE_2_POS;
    cmdBuffer[ 3 ] = ( address & ADDRESS_BYTE_1_MSK ) >> ADDRESS_BYTE_1_POS;
    cmdBuffer[ 4 ] = 0;  // Dummy byte

    /*-------------------------------------------------------------------------
    Perform the SPI transaction
    -------------------------------------------------------------------------*/
    Chimera::Thread::LockGuard _spilock( *mSPI );
    auto                       result = Chimera::Status::OK;

    // Enable the NOR chip
    result |= mSPI->assignChipSelect( mCS );
    result |= mSPI->setChipSelectControlMode( Chimera::SPI::CSMode::MANUAL );
    result |= mSPI->setChipSelect( Chimera::GPIO::State::LOW );

    // Tell the hardware which address to read from
    result |= mSPI->writeBytes( cmdBuffer.data(), CFI::READ_ARRAY_HS_OPS_LEN );
    result |= mSPI->await( Chimera::Event::Trigger::TRIGGER_TRANSFER_COMPLETE, Chimera::Thread::TIMEOUT_BLOCK );

    // Pull out all the data
    result |= mSPI->readBytes( data, length );
    result |= mSPI->await( Chimera::Event::Trigger::TRIGGER_TRANSFER_COMPLETE, Chimera::Thread::TIMEOUT_BLOCK );

    // Release the SPI and disable the NOR chip
    result |= mSPI->setChipSelect( Chimera::GPIO::State::HIGH );
    result |= mSPI->assignChipSelect( nullptr );

    return ( result == Chimera::Status::OK ) ? Status::ERR_OK : Status::ERR_DRIVER_ERR;
  }


  Aurora::Memory::Status Driver::erase( const size_t chunk )
  {
    return this->erase( ( mAttr.eraseSize * chunk ), mAttr.eraseSize );
  }


  Aurora::Memory::Status Driver::erase( const size_t address, const size_t length )
  {
    using namespace Aurora::Logging;
    using namespace Aurora::Memory;
    using namespace Chimera::Thread;

    /*-------------------------------------------------------------------------
    Input Protection
    -------------------------------------------------------------------------*/
    LockGuard _driverLock( *this );
    RT_DBG_ASSERT( ( address + length ) <= mProps->endAddress );

    /*-------------------------------------------------------------------------
    Determine the op-code to use based on the requested chunk size to erase.
    -------------------------------------------------------------------------*/
    size_t eraseOpsLen = CFI::BLOCK_ERASE_OPS_LEN;
    switch ( length )
    {
      case CHUNK_SIZE_4K:
        cmdBuffer[ 0 ] = CFI::BLOCK_ERASE_4K;
        break;

      case CHUNK_SIZE_32K:
        cmdBuffer[ 0 ] = CFI::BLOCK_ERASE_32K;
        break;

      case CHUNK_SIZE_64K:
        cmdBuffer[ 0 ] = CFI::BLOCK_ERASE_64K;
        break;

      default:  // Weird erase size
        RT_HARD_ASSERT( false );
        break;
    }

    /*-------------------------------------------------------------------------
    Write enable command must be sent before sending erase command
    -------------------------------------------------------------------------*/
    NOR_LOG( LOG_DEBUG( "Erase %d kB at address 0x%.8X\r\n", ( length / 1024 ), address ) );
    issueWriteEnable();

    /*-------------------------------------------------------------------------
    Initialize the command sequence. If whole chip erase command, these bytes
    will be ignored anyways.
    -------------------------------------------------------------------------*/
    cmdBuffer[ 1 ] = ( address & ADDRESS_BYTE_3_MSK ) >> ADDRESS_BYTE_3_POS;
    cmdBuffer[ 2 ] = ( address & ADDRESS_BYTE_2_MSK ) >> ADDRESS_BYTE_2_POS;
    cmdBuffer[ 3 ] = ( address & ADDRESS_BYTE_1_MSK ) >> ADDRESS_BYTE_1_POS;

    /*-------------------------------------------------------------------------
    Perform the SPI transaction
    -------------------------------------------------------------------------*/
    Chimera::Thread::LockGuard _spilock( *mSPI );
    auto                       result = Chimera::Status::OK;

    result |= mSPI->assignChipSelect( mCS );
    result |= mSPI->setChipSelect( Chimera::GPIO::State::LOW );
    result |= mSPI->readWriteBytes( cmdBuffer.data(), cmdBuffer.data(), eraseOpsLen );
    result |= mSPI->await( Chimera::Event::Trigger::TRIGGER_TRANSFER_COMPLETE, TIMEOUT_BLOCK );
    result |= mSPI->setChipSelect( Chimera::GPIO::State::HIGH );

    /*-------------------------------------------------------------------------
    Wait for the hardware to finish the operation
    -------------------------------------------------------------------------*/
    auto status = pendEvent( Event::MEM_ERASE_COMPLETE, TIMEOUT_BLOCK );
    NOR_LOG( LOG_DEBUG( "Erase complete\r\n" ) );

    if ( ( result == Chimera::Status::OK ) && ( status == Status::ERR_OK ) )
    {
      return Status::ERR_OK;
    }
    else
    {
      return Status::ERR_DRIVER_ERR;
    }
  }


  Aurora::Memory::Status Driver::erase()
  {
    using namespace Aurora::Memory;
    using namespace Chimera::Thread;

    /*-------------------------------------------------------------------------
    Input Protection
    -------------------------------------------------------------------------*/
    RT_DBG_ASSERT( mProps );
    LockGuard _driverLock( *this );

    /*-------------------------------------------------------------------------
    Write enable command must be sent before sending erase command
    -------------------------------------------------------------------------*/
    NOR_LOG( LOG_DEBUG( "Erase entire chip\r\n" ) );
    issueWriteEnable();

    /*-------------------------------------------------------------------------
    Perform the SPI transaction
    -------------------------------------------------------------------------*/
    Chimera::Thread::LockGuard _spilock( *mSPI );
    auto                       result = Chimera::Status::OK;

    result |= mSPI->assignChipSelect( mCS );
    result |= mSPI->setChipSelectControlMode( Chimera::SPI::CSMode::MANUAL );
    result |= mSPI->setChipSelect( Chimera::GPIO::State::LOW );
    result |= mSPI->writeBytes( &CFI::CHIP_ERASE, CFI::CHIP_ERASE_OPS_LEN );
    result |= mSPI->await( Chimera::Event::Trigger::TRIGGER_TRANSFER_COMPLETE, Chimera::Thread::TIMEOUT_BLOCK );
    result |= mSPI->setChipSelect( Chimera::GPIO::State::HIGH );

    /*-------------------------------------------------------------------------
    Wait for the hardware to finish the operation
    -------------------------------------------------------------------------*/
    auto status = pendEvent( Event::MEM_ERASE_COMPLETE, TIMEOUT_BLOCK );
    NOR_LOG( LOG_DEBUG( "Erase complete\r\n" ) );

    if ( ( result == Chimera::Status::OK ) && ( status == Status::ERR_OK ) )
    {
      return Status::ERR_OK;
    }
    else
    {
      return Status::ERR_DRIVER_ERR;
    }
  }


  Aurora::Memory::Status Driver::flush()
  {
    return Aurora::Memory::Status::ERR_OK;
  }


  Aurora::Memory::Status Driver::pendEvent( const Aurora::Memory::Event event, const size_t timeout )
  {
    /*-------------------------------------------------------------------------
    Input Protection
    -------------------------------------------------------------------------*/
    RT_DBG_ASSERT( mProps );
    RT_DBG_ASSERT( mProps->eventPoll );

    /*-------------------------------------------------------------------------
    Invoke the driver's poll func
    -------------------------------------------------------------------------*/
    return mProps->eventPoll( mSPI, static_cast<uint8_t>( mChip ), event, timeout );
  }


  /*---------------------------------------------------------------------------
  Driver: NOR Specific Interface
  ---------------------------------------------------------------------------*/
  bool Driver::configure( const Chip_t device, const Chimera::SPI::Channel channel )
  {
    mChip       = device;
    mSPIChannel = channel;
    mSPI        = Chimera::SPI::getDriver( channel );
    mProps      = getProperties( device );

    return static_cast<bool>( mSPI && mProps );
  }


  void Driver::transfer( const void *const cmd, void *const output, const size_t size )
  {
    RT_DBG_ASSERT( mSPI );
    Chimera::Thread::LockGuard _lck( *mSPI );
    Chimera::Status_t          result = Chimera::Status::OK;

    result |= mSPI->assignChipSelect( mCS );
    result |= mSPI->setChipSelectControlMode( Chimera::SPI::CSMode::MANUAL );
    result |= mSPI->setChipSelect( Chimera::GPIO::State::LOW );
    result |= mSPI->readWriteBytes( cmd, output, size );
    result |= mSPI->await( Chimera::Event::Trigger::TRIGGER_TRANSFER_COMPLETE, Chimera::Thread::TIMEOUT_BLOCK );
    result |= mSPI->setChipSelect( Chimera::GPIO::State::HIGH );
    result |= mSPI->assignChipSelect( nullptr );

    RT_DBG_ASSERT( result == Chimera::Status::OK );
  }

  bool Driver::assignChipSelect( const Chimera::GPIO::Port port, const Chimera::GPIO::Pin pin )
  {
    mCS = Chimera::GPIO::getDriver( port, pin );
    return static_cast<bool>( mCS );
  }


  Chip_t Driver::deviceType() const
  {
    return mChip;
  }


  DeviceAttr Driver::getAttr() const
  {
    return mAttr;
  }

  /*---------------------------------------------------------------------------
  Driver: Private Interface
  ---------------------------------------------------------------------------*/
  void Driver::issueWriteEnable()
  {
    RT_DBG_ASSERT( mSPI );
    Chimera::Thread::LockGuard _lck( *mSPI );
    Chimera::Status_t          result = Chimera::Status::OK;

    result |= mSPI->assignChipSelect( mCS );
    result |= mSPI->setChipSelect( Chimera::GPIO::State::LOW );
    result |= mSPI->writeBytes( &CFI::WRITE_ENABLE, CFI::WRITE_ENABLE_OPS_LEN );
    result |= mSPI->await( Chimera::Event::Trigger::TRIGGER_TRANSFER_COMPLETE, Chimera::Thread::TIMEOUT_BLOCK );
    result |= mSPI->setChipSelect( Chimera::GPIO::State::HIGH );

    RT_DBG_ASSERT( result == Chimera::Status::OK );
  }

}  // namespace Aurora::Memory::Flash::NOR
