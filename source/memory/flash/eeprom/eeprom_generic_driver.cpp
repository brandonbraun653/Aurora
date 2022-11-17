/******************************************************************************
 *  File Name:
 *    eeprom_generic_driver.cpp
 *
 *  Description:
 *    EEPROM Generic Driver
 *
 *  2022 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/logging>
#include <Aurora/memory>
#include <Aurora/utility>
#include <Chimera/assert>
#include <Chimera/common>
#include <Chimera/event>
#include <Chimera/i2c>
#include <Chimera/thread>


namespace Aurora::Memory::Flash::EEPROM
{
  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/
  const Aurora::Memory::Properties *getProperties( const Chip_t device )
  {
    /*-------------------------------------------------------------------------
    Input Protection
    -------------------------------------------------------------------------*/
    if ( device >= Chip::EEPROM_CHIP_OPTIONS )
    {
      return nullptr;
    }

    /*-------------------------------------------------------------------------
    Look up the device in the properties table
    -------------------------------------------------------------------------*/
    if ( ( device >= Chip::EEPROM_CHIP_START ) && ( device < Chip::EEPROM_CHIP_END ) )
    {
      size_t idx = static_cast<size_t>( device - Chip::EEPROM_CHIP_START );
      RT_HARD_ASSERT( idx < ARRAY_COUNT( ChipProperties ) );
      return &ChipProperties[ idx ];
    }
    else
    {
      return nullptr;
    }
  }

  /*---------------------------------------------------------------------------
  Classes
  ---------------------------------------------------------------------------*/
  Driver::Driver() : mConfig( {} ), mDriver( nullptr ), mProps( nullptr )
  {
  }


  Driver::~Driver()
  {
  }


  Aurora::Memory::Status Driver::open( const DeviceAttr *const attributes )
  {
    this->initAIO();
    return Aurora::Memory::Status::ERR_OK;
  }


  Aurora::Memory::Status Driver::close()
  {
    return Aurora::Memory::Status::ERR_OK;
  }


  Aurora::Memory::Status Driver::write( const size_t chunk, const size_t offset, const void *const data, const size_t length )
  {
    /*-------------------------------------------------------------------------
    EEPROM have no concept of a chunk. Byte level access.
    -------------------------------------------------------------------------*/
    return Aurora::Memory::Status::ERR_UNSUPPORTED;
  }


  Aurora::Memory::Status Driver::write( const size_t address, const void *const data, const size_t length )
  {
    using namespace Aurora::Memory;
    using namespace Chimera::Event;
    using namespace Chimera::Thread;

    /*-------------------------------------------------------------------------
    Input Protection
    -------------------------------------------------------------------------*/
    if ( !data || !length || ( ( address + length ) > mProps->endAddress ) )
    {
      return Aurora::Memory::Status::ERR_BAD_ARG;
    }

    /*-------------------------------------------------------------------------
    Do the write. This is a very naive approach that simply delays a fixed
    amount after each write. EEPROM aren't exactly known for performance...
    -------------------------------------------------------------------------*/
    Chimera::Status_t result = Chimera::Status::OK;

#if defined( EMBEDDED )
    uint8_t write_size = 0;
    uint8_t write_data[ 3 ];

    for ( size_t idx = 0; idx < length; idx++ )
    {
      if ( mProps->endAddress <= 256 )
      {
        write_size      = 2;
        write_data[ 0 ] = address + idx;
        write_data[ 1 ] = static_cast<const uint8_t *const>( data )[ idx ];
      }
      else if ( mProps->endAddress <= ( 65 * 1024 ) )
      {
        write_size      = 3;
        write_data[ 0 ] = ( ( ( address + idx ) >> 8 ) & 0xFF );  // High byte
        write_data[ 1 ] = ( ( ( address + idx ) >> 0 ) & 0xFF );  // Low byte
        write_data[ 2 ] = static_cast<const uint8_t *const>( data )[ idx ];
      }
      else
      {
        // Need to implement larger address access scheme
        RT_HARD_ASSERT( false );
      }

      result |= mDriver->write( mConfig.deviceAddress, write_data, write_size );
      Chimera::delayMilliseconds( mProps->pagePgmDelay );
    }
#else /* SIMULATOR */
    result |= mDriver->write( address, data, length );
    result |= mDriver->await( Trigger::TRIGGER_TRANSFER_COMPLETE, ( length * TIMEOUT_10MS ) );
#endif

    return ( result == Chimera::Status::OK ) ? Status::ERR_OK : Status::ERR_FAIL;
  }


  Aurora::Memory::Status Driver::read( const size_t chunk, const size_t offset, void *const data, const size_t length )
  {
    /*-------------------------------------------------------------------------
    EEPROM have no concept of a chunk. Byte level access.
    -------------------------------------------------------------------------*/
    return Aurora::Memory::Status::ERR_UNSUPPORTED;
  }


  Aurora::Memory::Status Driver::read( const size_t address, void *const data, const size_t length )
  {
    using namespace Aurora::Memory;
    using namespace Chimera::Event;
    using namespace Chimera::Thread;

    /*-------------------------------------------------------------------------
    Input Protection
    -------------------------------------------------------------------------*/
    if ( !data || !length || ( ( address + length ) > mProps->endAddress ) )
    {
      return Aurora::Memory::Status::ERR_BAD_ARG;
    }

    /*-------------------------------------------------------------------------
    Do a random word read. Assumes the EEPROM supports continuous reads with
    wraparound, which is pretty common.
    -------------------------------------------------------------------------*/
    Chimera::Status_t result = Chimera::Status::OK;

#if defined( EMBEDDED )
    /* Setup the read address in-chip */
    if ( mProps->endAddress <= 256 )
    {
      result |= mDriver->write( mConfig.deviceAddress, &address, 1 );
    }
    else if ( mProps->endAddress <= ( 65 * 1024 ) )
    {
      result |= mDriver->write( mConfig.deviceAddress, &address, 2 );
    }
    else
    {
      // Need to implement larger address access scheme
      RT_HARD_ASSERT( false );
    }

    result |= mDriver->await( Trigger::TRIGGER_TRANSFER_COMPLETE, TIMEOUT_10MS );

    /* Do the continuous read */
    result |= mDriver->read( mConfig.deviceAddress, data, length );
    result |= mDriver->await( Trigger::TRIGGER_TRANSFER_COMPLETE, ( length * TIMEOUT_10MS ) );

#else /* SIMULATOR */
    result |= mDriver->read( address, data, length );
    result |= mDriver->await( Trigger::TRIGGER_TRANSFER_COMPLETE, ( length * TIMEOUT_10MS ) );
#endif

    return ( result == Chimera::Status::OK ) ? Status::ERR_OK : Status::ERR_FAIL;
  }


  Aurora::Memory::Status Driver::erase( const size_t chunk )
  {
    /*-------------------------------------------------------------------------
    EEPROM have no concept of a chunk. Byte level access.
    -------------------------------------------------------------------------*/
    return Aurora::Memory::Status::ERR_UNSUPPORTED;
  }


  Aurora::Memory::Status Driver::erase( const size_t address, const size_t length )
  {
    using namespace Aurora::Memory;
    using namespace Chimera::Event;
    using namespace Chimera::Thread;

    /*-------------------------------------------------------------------------
    Input Protection
    -------------------------------------------------------------------------*/
    if ( !length || ( ( address + length ) > mProps->endAddress ) )
    {
      return Aurora::Memory::Status::ERR_BAD_ARG;
    }

    /*-------------------------------------------------------------------------
    Do erase
    -------------------------------------------------------------------------*/
    Status  error     = Status::ERR_OK;
    uint8_t erase_val = 0xFF;

    for ( size_t idx = 0; idx < length; idx++ )
    {
      error = error | this->write( address + idx, &erase_val, sizeof( erase_val ) );
    }

    return error;
  }


  Aurora::Memory::Status Driver::erase()
  {
    /*-------------------------------------------------------------------------
    Erase the whole chip
    -------------------------------------------------------------------------*/
    return this->erase( mProps->startAddress, mProps->endAddress );
  }


  Aurora::Memory::Status Driver::flush()
  {
    return Aurora::Memory::Status::ERR_OK;
  }


  Aurora::Memory::Status Driver::pendEvent( const Aurora::Memory::Event event, const size_t timeout )
  {
    return Aurora::Memory::Status::ERR_UNSUPPORTED;
  }


  bool Driver::configure( const DeviceConfig &config )
  {
    mDriver = Chimera::I2C::getDriver( config.i2cChannel );
    mConfig = config;
    mProps  = getProperties( config.whichChip );

    return static_cast<bool>( mDriver && mProps );
  }

}  // namespace Aurora::Memory::Flash::EEPROM
