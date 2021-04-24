/********************************************************************************
 *  File Name:
 *    nor_generic_driver.cpp
 *
 *  Description:
 *    NOR flash generic driver implementation
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* Aurora Includes */
#include <Aurora/logging>
#include <Aurora/utility>

/* Chimera Includes */
#include <Chimera/assert>
#include <Chimera/common>
#include <Chimera/event>
#include <Chimera/spi>
#include <Chimera/thread>

/* Aurora Includes */
#include <Aurora/memory>
#include <Aurora/source/memory/flash/jedec/jedec_cfi_cmds.hpp>
#include <Aurora/source/memory/flash/nor/manufacturer/nor_adesto.hpp>

namespace Aurora::Flash::NOR
{
  /*-------------------------------------------------------------------------------
  Static Functions
  -------------------------------------------------------------------------------*/
  bool unitChunk2Address( const size_t unitSize, const size_t unitId, const size_t maxAddress, size_t *address )
  {
    size_t physicalAddress = unitSize * unitId;
    bool isValid           = physicalAddress < maxAddress;

    if ( isValid && address )
    {
      *address = physicalAddress;
    }

    return isValid;
  }

  /*-------------------------------------------------------------------------------
  Public Methods
  -------------------------------------------------------------------------------*/
  const Aurora::Memory::Properties *getProperties( const Chip_t device )
  {
    /*-------------------------------------------------
    Input Protection
    -------------------------------------------------*/
    if ( device >= Chip::NUM_OPTIONS )
    {
      return nullptr;
    }

    /*-------------------------------------------------
    Look up the device in the appropriate table
    -------------------------------------------------*/
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
    /*-------------------------------------------------
    Input Protection
    -------------------------------------------------*/
    auto props = getProperties( device );
    if ( !props )
    {
      return false;
    }

    /*-------------------------------------------------
    Calculate the address
    -------------------------------------------------*/
    return unitChunk2Address( props->pageSize, page, props->endAddress, address );
  }


  bool block2Address( const Chip_t device, const size_t block, size_t *const address )
  {
    /*-------------------------------------------------
    Input Protection
    -------------------------------------------------*/
    auto props = getProperties( device );
    if ( !props )
    {
      return false;
    }

    /*-------------------------------------------------
    Calculate the address
    -------------------------------------------------*/
    return unitChunk2Address( props->blockSize, block, props->endAddress, address );
  }


  bool sector2Address( const Chip_t device, const size_t sector, size_t *const address )
  {
    /*-------------------------------------------------
    Input Protection
    -------------------------------------------------*/
    auto props = getProperties( device );
    if ( !props )
    {
      return false;
    }

    /*-------------------------------------------------
    Calculate the address
    -------------------------------------------------*/
    return unitChunk2Address( props->sectorSize, sector, props->endAddress, address );
  }


  bool address2WriteChunkOffset( const Chip_t device, const size_t address, size_t *const chunk, size_t *const offset )
  {
    using namespace Aurora::Memory;

    /*-------------------------------------------------
    Input Protection
    -------------------------------------------------*/
    const Properties *props = nullptr;

    if ( !chunk || !offset )
    {
      return false;
    }
    else if ( props = getProperties( device ); !props )
    {
      return false;
    }

    /*-------------------------------------------------
    Figure out the chunk ID in use
    -------------------------------------------------*/
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

    /*-------------------------------------------------
    Figure out the byte offset inside the chunk
    -------------------------------------------------*/
    size_t byte_offset = 0;
    if ( address < chunk_base_address )
    {
      return false;
    }

    byte_offset = address - chunk_base_address;

    /*-------------------------------------------------
    Give the data back to the user
    -------------------------------------------------*/
    *chunk  = chunk_id;
    *offset = byte_offset;
    return true;
  }


  /*-------------------------------------------------------------------------------
  Device Driver Implementation
  -------------------------------------------------------------------------------*/
  Driver::Driver() : mChip( Chip::UNKNOWN ), mSPI( nullptr )
  {
  }


  Driver::~Driver()
  {
  }


  /*-------------------------------------------------------------------------------
  Driver: Generic Memory Interface
  -------------------------------------------------------------------------------*/
  Aurora::Memory::Status Driver::open()
  {
    return Aurora::Memory::Status::ERR_OK;
  }


  Aurora::Memory::Status Driver::close()
  {
    return Aurora::Memory::Status::ERR_OK;
  }


  Aurora::Memory::Status Driver::write( const size_t chunk, const size_t offset, const void *const data, const size_t length )
  {
    using namespace Aurora::Logging;
    using namespace Aurora::Memory;
    using namespace Chimera::Thread;

    /*-------------------------------------------------
    Input Protection
    -------------------------------------------------*/
    const Properties *props = nullptr;

    if ( !data || !length )
    {
      return Aurora::Memory::Status::ERR_BAD_ARG;
    }
    else if ( props = getProperties( mChip ); !props )
    {
      return Aurora::Memory::Status::ERR_UNSUPPORTED;
    }

    /*-------------------------------------------------
    Figure out the chunk size in use
    -------------------------------------------------*/
    size_t address = 0;
    bool validity  = false;

    switch ( props->writeChunk )
    {
      case Chunk::PAGE:
        validity = page2Address( mChip, chunk, &address );
        break;

      case Chunk::BLOCK:
        validity = block2Address( mChip, chunk, &address );
        break;

      case Chunk::SECTOR:
        validity = sector2Address( mChip, chunk, &address );
        break;

      default:
        return Status::ERR_BAD_ARG;
        break;
    }

    if ( !validity )
    {
      return Status::ERR_BAD_ARG;
    }

    address += offset;
    if( ( address + length ) >= props->endAddress )
    {
      Chimera::insert_debug_breakpoint();
      return Status::ERR_OVERRUN;
    }

    /*-------------------------------------------------
    Acquire access to this driver
    -------------------------------------------------*/
    this->lock();

    /*-------------------------------------------------
    Per datasheet specs, the write enable command must
    be sent before issuing the actual data.
    -------------------------------------------------*/
    issueWriteEnable();

    /*-------------------------------------------------
    Initialize the command sequence
    -------------------------------------------------*/
    cmdBuffer[ 0 ] = CFI::PAGE_PROGRAM;
    cmdBuffer[ 1 ] = ( address & ADDRESS_BYTE_3_MSK ) >> ADDRESS_BYTE_3_POS;
    cmdBuffer[ 2 ] = ( address & ADDRESS_BYTE_2_MSK ) >> ADDRESS_BYTE_2_POS;
    cmdBuffer[ 3 ] = ( address & ADDRESS_BYTE_1_MSK ) >> ADDRESS_BYTE_1_POS;

    /*-------------------------------------------------
    Perform the SPI transaction
    -------------------------------------------------*/
    // Acquire the SPI and enable the memory chip
    mSPI->lock();
    mSPI->setChipSelect( Chimera::GPIO::State::LOW );

    // Tell the hardware which address to write into
    mSPI->writeBytes( cmdBuffer.data(), CFI::PAGE_PROGRAM_OPS_LEN );
    mSPI->await( Chimera::Event::Trigger::TRIGGER_TRANSFER_COMPLETE, TIMEOUT_BLOCK );

    // Dump the data
    mSPI->writeBytes( data, length );
    mSPI->await( Chimera::Event::Trigger::TRIGGER_TRANSFER_COMPLETE, TIMEOUT_BLOCK );

    // Release the SPI and disable the memory chip
    mSPI->setChipSelect( Chimera::GPIO::State::HIGH );
    mSPI->unlock();

    /*-------------------------------------------------
    Wait for the hardware to finish the operation
    -------------------------------------------------*/
    pendEvent( Event::MEM_WRITE_COMPLETE, TIMEOUT_BLOCK );

    /*-------------------------------------------------
    Release access to this driver and exit
    -------------------------------------------------*/
    LOG_DEBUG( "Write %d bytes to address 0x%.8X\r\n", length, address );
    this->unlock();
    return Aurora::Memory::Status::ERR_OK;
  }


  Aurora::Memory::Status Driver::read( const size_t chunk, const size_t offset, void *const data, const size_t length )
  {
    using namespace Aurora::Memory;

    /*-------------------------------------------------
    Input Protection
    -------------------------------------------------*/
    const Properties *props = nullptr;

    if ( !data || !length )
    {
      return Aurora::Memory::Status::ERR_BAD_ARG;
    }
    else if ( props = getProperties( mChip ); !props )
    {
      return Aurora::Memory::Status::ERR_UNSUPPORTED;
    }

    /*-------------------------------------------------
    Figure out the chunk size in use
    -------------------------------------------------*/
    size_t address = 0;
    bool validity  = false;

    switch ( props->readChunk )
    {
      case Chunk::PAGE:
        validity = page2Address( mChip, chunk, &address );
        break;

      case Chunk::BLOCK:
        validity = block2Address( mChip, chunk, &address );
        break;

      case Chunk::SECTOR:
        validity = sector2Address( mChip, chunk, &address );
        break;

      default:
        return Status::ERR_BAD_ARG;
        break;
    }

    if ( !validity )
    {
      return Status::ERR_BAD_ARG;
    }

    address += offset;

    if( ( address + length ) >= props->endAddress )
    {
      Chimera::insert_debug_breakpoint();
      return Status::ERR_OVERRUN;
    }

    /*-------------------------------------------------
    Acquire access to this driver
    -------------------------------------------------*/
    this->lock();

    /*-------------------------------------------------
    Initialize the command sequence. The high speed
    command works for all frequency ranges.
    -------------------------------------------------*/
    cmdBuffer[ 0 ] = CFI::READ_ARRAY_HS;
    cmdBuffer[ 1 ] = ( address & ADDRESS_BYTE_3_MSK ) >> ADDRESS_BYTE_3_POS;
    cmdBuffer[ 2 ] = ( address & ADDRESS_BYTE_2_MSK ) >> ADDRESS_BYTE_2_POS;
    cmdBuffer[ 3 ] = ( address & ADDRESS_BYTE_1_MSK ) >> ADDRESS_BYTE_1_POS;
    cmdBuffer[ 4 ] = 0;  // Dummy byte

    /*-------------------------------------------------
    Perform the SPI transaction
    -------------------------------------------------*/
    mSPI->lock();
    mSPI->setChipSelect( Chimera::GPIO::State::LOW );

    // Tell the hardware which address to read from
    mSPI->writeBytes( cmdBuffer.data(), CFI::READ_ARRAY_HS_OPS_LEN );
    mSPI->await( Chimera::Event::Trigger::TRIGGER_TRANSFER_COMPLETE, Chimera::Thread::TIMEOUT_BLOCK );

    // Pull out all the data
    mSPI->readBytes( data, length );
    mSPI->await( Chimera::Event::Trigger::TRIGGER_TRANSFER_COMPLETE, Chimera::Thread::TIMEOUT_BLOCK );

    mSPI->setChipSelect( Chimera::GPIO::State::HIGH );
    mSPI->unlock();

    /*-------------------------------------------------
    Release access to this driver and exit
    -------------------------------------------------*/
    LOG_DEBUG( "Read %d bytes from address 0x%.8X\r\n", length, address );
    this->unlock();
    return Aurora::Memory::Status::ERR_OK;
  }


  Aurora::Memory::Status Driver::erase( const size_t chunk )
  {
    using namespace Aurora::Memory;
    using namespace Chimera::Thread;

    /*-------------------------------------------------
    Input Protection
    -------------------------------------------------*/
    const Properties *props = nullptr;
    if ( props = getProperties( mChip ); !props )
    {
      return Aurora::Memory::Status::ERR_UNSUPPORTED;
    }

    /*-------------------------------------------------
    Figure out the chunk size in use
    -------------------------------------------------*/
    size_t address   = 0;
    size_t eraseSize = 0;
    bool validity    = false;

    switch ( props->eraseChunk )
    {
      case Chunk::PAGE:
        eraseSize = props->pageSize;
        validity  = page2Address( mChip, chunk, &address );
        break;

      case Chunk::BLOCK:
        eraseSize = props->blockSize;
        validity  = block2Address( mChip, chunk, &address );
        break;

      case Chunk::SECTOR:
        eraseSize = props->sectorSize;
        validity  = sector2Address( mChip, chunk, &address );
        break;

      default:
        return Status::ERR_BAD_ARG;
        break;
    }

    if ( !validity )
    {
      return Status::ERR_BAD_ARG;
    }

    /*-------------------------------------------------
    Determine the op-code to use based on the requested
    chunk size to erase.
    -------------------------------------------------*/
    size_t eraseOpsLen = CFI::BLOCK_ERASE_OPS_LEN;
    size_t traceEraseLenKilo = 0;
    switch ( eraseSize )
    {
      case CHUNK_SIZE_4K:
        cmdBuffer[ 0 ] = CFI::BLOCK_ERASE_4K;
        traceEraseLenKilo = 4;
        break;

      case CHUNK_SIZE_32K:
        cmdBuffer[ 0 ] = CFI::BLOCK_ERASE_32K;
        traceEraseLenKilo = 32;
        break;

      case CHUNK_SIZE_64K:
        cmdBuffer[ 0 ] = CFI::BLOCK_ERASE_64K;
        traceEraseLenKilo = 64;
        break;

      default:  // Weird erase size
        Chimera::insert_debug_breakpoint();
        return Status::ERR_UNSUPPORTED;
        break;
    }

    /*-------------------------------------------------
    Acquire access to this driver
    -------------------------------------------------*/
    this->lock();

    /*-------------------------------------------------
    Per datasheet specs, the write enable command must
    be sent before issuing the actual data.
    -------------------------------------------------*/
    issueWriteEnable();

    /*-------------------------------------------------
    Initialize the command sequence. If whole chip
    erase command, these bytes will be ignored anyways.
    -------------------------------------------------*/
    cmdBuffer[ 1 ] = ( address & ADDRESS_BYTE_3_MSK ) >> ADDRESS_BYTE_3_POS;
    cmdBuffer[ 2 ] = ( address & ADDRESS_BYTE_2_MSK ) >> ADDRESS_BYTE_2_POS;
    cmdBuffer[ 3 ] = ( address & ADDRESS_BYTE_1_MSK ) >> ADDRESS_BYTE_1_POS;

    /*-------------------------------------------------
    Perform the SPI transaction
    -------------------------------------------------*/
    auto spiResult = Chimera::Status::OK;

    mSPI->lock();
    spiResult |= mSPI->setChipSelect( Chimera::GPIO::State::LOW );
    spiResult |= mSPI->readWriteBytes( cmdBuffer.data(), cmdBuffer.data(), eraseOpsLen );
    spiResult |= mSPI->await( Chimera::Event::Trigger::TRIGGER_TRANSFER_COMPLETE, TIMEOUT_BLOCK );
    spiResult |= mSPI->setChipSelect( Chimera::GPIO::State::HIGH );
    mSPI->unlock();

    /*-------------------------------------------------
    Wait for the hardware to finish the operation
    -------------------------------------------------*/
    pendEvent( Event::MEM_ERASE_COMPLETE, TIMEOUT_BLOCK );

    /*-------------------------------------------------
    Release access to this driver
    -------------------------------------------------*/
    LOG_DEBUG( "Erase %d kB at address 0x%.8X\r\n", traceEraseLenKilo, address );
    this->unlock();
    if ( spiResult == Chimera::Status::OK )
    {
      return Aurora::Memory::Status::ERR_OK;
    }
    else
    {
      return Aurora::Memory::Status::ERR_DRIVER_ERR;
    }
  }


  Aurora::Memory::Status Driver::erase()
  {
    using namespace Aurora::Memory;
    using namespace Chimera::Thread;

    /*-------------------------------------------------
    Input Protection
    -------------------------------------------------*/
    const Properties *props = nullptr;
    if ( props = getProperties( mChip ); !props )
    {
      return Aurora::Memory::Status::ERR_UNSUPPORTED;
    }

    /*-------------------------------------------------
    Per datasheet specs, the write enable command must
    be sent before issuing the actual data.
    -------------------------------------------------*/
    issueWriteEnable();

    /*-------------------------------------------------
    Perform the SPI transaction
    -------------------------------------------------*/
    auto spiResult = Chimera::Status::OK;

    mSPI->lock();
    spiResult |= mSPI->setChipSelect( Chimera::GPIO::State::LOW );
    spiResult |= mSPI->writeBytes( &CFI::CHIP_ERASE, CFI::CHIP_ERASE_OPS_LEN );
    spiResult |= mSPI->await( Chimera::Event::Trigger::TRIGGER_TRANSFER_COMPLETE, Chimera::Thread::TIMEOUT_BLOCK );
    spiResult |= mSPI->setChipSelect( Chimera::GPIO::State::HIGH );
    mSPI->unlock();

    /*-------------------------------------------------
    Wait for the hardware to finish the operation
    -------------------------------------------------*/
    pendEvent( Event::MEM_ERASE_COMPLETE, TIMEOUT_BLOCK );

    /*-------------------------------------------------
    Release access to this driver
    -------------------------------------------------*/
    LOG_DEBUG( "Erase entire chip\r\n" );
    if ( spiResult == Chimera::Status::OK )
    {
      return Aurora::Memory::Status::ERR_OK;
    }
    else
    {
      return Aurora::Memory::Status::ERR_DRIVER_ERR;
    }
  }


  Aurora::Memory::Status Driver::flush()
  {
    return Aurora::Memory::Status::ERR_OK;
  }


  Aurora::Memory::Status Driver::pendEvent( const Aurora::Memory::Event event, const size_t timeout )
  {
    /*-------------------------------------------------
    Input Protection
    -------------------------------------------------*/
    const Aurora::Memory::Properties *props = nullptr;
    if ( props = getProperties( mChip ); !props )
    {
      return Aurora::Memory::Status::ERR_UNSUPPORTED;
    }
    else if ( !props->eventPoll )
    {
      return Aurora::Memory::Status::ERR_DRIVER_ERR;
    }

    /*-------------------------------------------------
    Invoke the driver's poll func
    -------------------------------------------------*/
    return props->eventPoll( static_cast<uint8_t>( mSPIChannel ), static_cast<uint8_t>( mChip ), event, timeout );
  }


  /*-------------------------------------------------------------------------------
  Driver: NOR Specific Interface
  -------------------------------------------------------------------------------*/
  bool Driver::configure( const Chip_t device, const Chimera::SPI::Channel channel )
  {
    mChip       = device;
    mSPIChannel = channel;
    mSPI        = Chimera::SPI::getDriver( channel );

    return static_cast<bool>( mSPI );
  }


  void Driver::transfer( const void *const cmd, void *const output, const size_t size )
  {
    mSPI->lock();
    mSPI->setChipSelect( Chimera::GPIO::State::LOW );
    mSPI->readWriteBytes( cmd, output, size );
    mSPI->await( Chimera::Event::Trigger::TRIGGER_TRANSFER_COMPLETE, Chimera::Thread::TIMEOUT_BLOCK );
    mSPI->setChipSelect( Chimera::GPIO::State::HIGH );
    mSPI->unlock();
  }


  /*-------------------------------------------------------------------------------
  Driver: Private Interface
  -------------------------------------------------------------------------------*/
  void Driver::issueWriteEnable()
  {
    mSPI->lock();
    mSPI->setChipSelect( Chimera::GPIO::State::LOW );
    mSPI->writeBytes( &CFI::WRITE_ENABLE, CFI::WRITE_ENABLE_OPS_LEN );
    mSPI->await( Chimera::Event::Trigger::TRIGGER_TRANSFER_COMPLETE, Chimera::Thread::TIMEOUT_BLOCK );
    mSPI->setChipSelect( Chimera::GPIO::State::HIGH );
    mSPI->unlock();
  }

}  // namespace Aurora::Flash::NOR
