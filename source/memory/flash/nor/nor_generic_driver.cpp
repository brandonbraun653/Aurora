/********************************************************************************
 *  File Name:
 *    nor_generic_driver.cpp
 *
 *  Description:
 *    NOR flash generic driver implementation
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

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
    bool isValid = physicalAddress < maxAddress;

    if( isValid && address )
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
    if( !props )
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
    if( !props )
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
    if( !props )
    {
      return false;
    }

    /*-------------------------------------------------
    Calculate the address
    -------------------------------------------------*/
    return unitChunk2Address( props->sectorSize, sector, props->endAddress, address );
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
    using namespace Aurora::Memory;

    /*-------------------------------------------------
    Input Protection
    -------------------------------------------------*/
    const Properties *props = nullptr;

    if ( !data || !length )
    {
      return Aurora::Memory::Status::ERR_BAD_ARG;
    }
    else if( props = getProperties( mChip ); !props )
    {
      return Aurora::Memory::Status::ERR_UNSUPPORTED;
    }

    /*-------------------------------------------------
    Figure out the chunk size in use
    -------------------------------------------------*/
    size_t address = 0;
    bool validity = false;

    switch( props->writeChunk )
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

    if( !validity )
    {
      return Status::ERR_BAD_ARG;
    }

    address += offset;

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
    mSPI->await( Chimera::Event::Trigger::TRIGGER_TRANSFER_COMPLETE, Chimera::Threading::TIMEOUT_BLOCK );

    // Dump the data
    mSPI->writeBytes( data, length );
    mSPI->await( Chimera::Event::Trigger::TRIGGER_TRANSFER_COMPLETE, Chimera::Threading::TIMEOUT_BLOCK );

    // Release the SPI and disable the memory chip
    mSPI->setChipSelect( Chimera::GPIO::State::HIGH );
    mSPI->unlock();

    /*-------------------------------------------------
    Release access to this driver and exit
    -------------------------------------------------*/
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
    else if( props = getProperties( mChip ); !props )
    {
      return Aurora::Memory::Status::ERR_UNSUPPORTED;
    }

    /*-------------------------------------------------
    Figure out the chunk size in use
    -------------------------------------------------*/
    size_t address = 0;
    bool validity = false;

    switch( props->readChunk )
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

    if( !validity )
    {
      return Status::ERR_BAD_ARG;
    }

    address += offset;

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
    mSPI->await( Chimera::Event::Trigger::TRIGGER_TRANSFER_COMPLETE, Chimera::Threading::TIMEOUT_BLOCK );

    // Pull out all the data
    mSPI->readBytes( data, length );
    mSPI->await( Chimera::Event::Trigger::TRIGGER_TRANSFER_COMPLETE, Chimera::Threading::TIMEOUT_BLOCK );

    mSPI->setChipSelect( Chimera::GPIO::State::HIGH );
    mSPI->unlock();

    /*-------------------------------------------------
    Release access to this driver and exit
    -------------------------------------------------*/
    this->unlock();
    return Aurora::Memory::Status::ERR_OK;
  }


  Aurora::Memory::Status Driver::erase( const size_t chunk )
  {
    using namespace Aurora::Memory;

    /*-------------------------------------------------
    Input Protection
    -------------------------------------------------*/
    const Properties *props = nullptr;
    if( props = getProperties( mChip ); !props )
    {
      return Aurora::Memory::Status::ERR_UNSUPPORTED;
    }

    /*-------------------------------------------------
    Figure out the chunk size in use
    -------------------------------------------------*/
    size_t address = 0;
    size_t eraseSize = 0;
    bool validity = false;

    switch( props->eraseChunk )
    {
      case Chunk::PAGE:
        eraseSize = props->pageSize;
        validity = page2Address( mChip, chunk, &address );
        break;

      case Chunk::BLOCK:
        eraseSize = props->blockSize;
        validity = block2Address( mChip, chunk, &address );
        break;

      case Chunk::SECTOR:
        eraseSize = props->sectorSize;
        validity = sector2Address( mChip, chunk, &address );
        break;

      default:
        return Status::ERR_BAD_ARG;
        break;
    }

    if( !validity )
    {
      return Status::ERR_BAD_ARG;
    }

    /*-------------------------------------------------
    Determine the op-code to use based on the requested
    chunk size to erase.
    -------------------------------------------------*/
    size_t eraseOpsLen = CFI::BLOCK_ERASE_OPS_LEN;
    switch ( eraseSize )
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

      default: // Weird erase size
        Chimera::insert_debug_breakpoint();
        return Aurora::Memory::Status::ERR_UNSUPPORTED;
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
    spiResult |= mSPI->await( Chimera::Event::Trigger::TRIGGER_TRANSFER_COMPLETE, Chimera::Threading::TIMEOUT_BLOCK );
    spiResult |= mSPI->setChipSelect( Chimera::GPIO::State::HIGH );
    mSPI->unlock();

    /*-------------------------------------------------
    Release access to this driver
    -------------------------------------------------*/
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
    spiResult |= mSPI->await( Chimera::Event::Trigger::TRIGGER_TRANSFER_COMPLETE, Chimera::Threading::TIMEOUT_BLOCK );
    spiResult |= mSPI->setChipSelect( Chimera::GPIO::State::HIGH );
    mSPI->unlock();

    /*-------------------------------------------------
    Release access to this driver
    -------------------------------------------------*/
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


  uint16_t Driver::readStatusRegister()
  {
    /*-------------------------------------------------
    Acquire access to this driver
    -------------------------------------------------*/
    this->lock();

    /*-------------------------------------------------
    Initialize the command sequence
    -------------------------------------------------*/
    cmdBuffer.fill( 0 );
    uint16_t result = 0;

    /*-------------------------------------------------
    Perform the SPI transaction
    -------------------------------------------------*/
    mSPI->lock();

    // Read out byte 1
    cmdBuffer[ 0 ] = CFI::READ_SR_BYTE1;
    mSPI->setChipSelect( Chimera::GPIO::State::LOW );
    mSPI->readWriteBytes( cmdBuffer.data(), cmdBuffer.data(), CFI::READ_SR_BYTE1_OPS_LEN );
    mSPI->await( Chimera::Event::Trigger::TRIGGER_TRANSFER_COMPLETE, Chimera::Threading::TIMEOUT_BLOCK );
    mSPI->setChipSelect( Chimera::GPIO::State::HIGH );

    result |= cmdBuffer[ 1 ];

    // Read out byte 2
    cmdBuffer[ 0 ] = CFI::READ_SR_BYTE2;
    cmdBuffer[ 1 ] = 0;
    mSPI->setChipSelect( Chimera::GPIO::State::LOW );
    mSPI->readWriteBytes( cmdBuffer.data(), cmdBuffer.data(), CFI::READ_SR_BYTE2_OPS_LEN );
    mSPI->await( Chimera::Event::Trigger::TRIGGER_TRANSFER_COMPLETE, Chimera::Threading::TIMEOUT_BLOCK );
    mSPI->setChipSelect( Chimera::GPIO::State::HIGH );

    result |= ( cmdBuffer[ 1 ] << 8 );

    mSPI->unlock();

    /*-------------------------------------------------
    Release access to this driver
    -------------------------------------------------*/
    this->unlock();
    return result;
  }


  /*-------------------------------------------------------------------------------
  Driver: Private Interface
  -------------------------------------------------------------------------------*/
  void Driver::issueWriteEnable()
  {
    mSPI->lock();
    mSPI->setChipSelect( Chimera::GPIO::State::LOW );
    mSPI->writeBytes( &CFI::WRITE_ENABLE, CFI::WRITE_ENABLE_OPS_LEN );
    mSPI->await( Chimera::Event::Trigger::TRIGGER_TRANSFER_COMPLETE, Chimera::Threading::TIMEOUT_BLOCK );
    mSPI->setChipSelect( Chimera::GPIO::State::HIGH );
    mSPI->unlock();
  }

}  // namespace Aurora::Flash::NOR
