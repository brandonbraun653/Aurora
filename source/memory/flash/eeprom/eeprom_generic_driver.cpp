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


namespace Aurora::Flash::EEPROM
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
  Driver::Driver()
  {
  }


  Driver::~Driver()
  {
  }


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
    const Properties *attr = nullptr;
    if ( attr = getProperties( mConfig.whichChip ); !attr  )
    {
      return Aurora::Memory::Status::ERR_UNSUPPORTED;
    }

    if( ( address + length ) > attr->endAddress )
    {
      return Aurora::Memory::Status::ERR_BAD_ARG;
    }

    /*-------------------------------------------------------------------------
    Do the write. This is a very naive approach that simply delays a fixed
    amount after each write. Some chips also allow for larger page write blocks
    but that's too specialized for this generic driver.
    -------------------------------------------------------------------------*/
    Chimera::Status_t result = Chimera::Status::OK;
    uint8_t write_data[ 2 ];

    if ( data && length )
    {
      for ( size_t idx = 0; idx < length; idx++ )
      {
        write_data[ 0 ] = address + idx;
        write_data[ 1 ] = static_cast<const uint8_t *const>( data )[ idx ];

        result |= mDriver->write( mConfig.deviceAddress, write_data, sizeof( write_data ) );
        Chimera::delayMilliseconds( attr->pagePgmDelay );
      }
    }
    else
    {
      result |= mDriver->write( mConfig.deviceAddress, &address, 1 );
    }

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
    const Properties *attr = nullptr;
    if ( attr = getProperties( mConfig.whichChip ); !attr  )
    {
      return Aurora::Memory::Status::ERR_UNSUPPORTED;
    }

    if( !data || !length || ( ( address + length ) > attr->endAddress ) )
    {
      return Aurora::Memory::Status::ERR_BAD_ARG;
    }

    /*-------------------------------------------------------------------------
    Do a random word read. Assumes the EEPROM supports continuous reads with
    wraparound, which is pretty common.
    -------------------------------------------------------------------------*/
    /* Setup the read address in-chip */
    this->write( address, nullptr, 0 );
    Chimera::delayMilliseconds( 1 );

    /* Do the continuous read */
    Chimera::Status_t result = mDriver->read( mConfig.deviceAddress, data, length );

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
    const Properties *attr = nullptr;
    if ( attr = getProperties( mConfig.whichChip ); !attr  )
    {
      return Aurora::Memory::Status::ERR_UNSUPPORTED;
    }

    if( !length || ( ( address + length ) > attr->endAddress ) )
    {
      return Aurora::Memory::Status::ERR_BAD_ARG;
    }

    /*-------------------------------------------------------------------------
    Do erase
    -------------------------------------------------------------------------*/
    Status  error     = Status::ERR_OK;
    uint8_t erase_val = 0xFF;

    for( size_t idx = 0; idx < length; idx++ )
    {
      error = error | this->write( address + idx, &erase_val, sizeof( erase_val ) );
    }

    return error;
  }


  Aurora::Memory::Status Driver::erase()
  {
    using namespace Aurora::Memory;
    using namespace Chimera::Event;
    using namespace Chimera::Thread;

    /*-------------------------------------------------------------------------
    Input Protection
    -------------------------------------------------------------------------*/
    const Properties *attr = nullptr;
    if ( attr = getProperties( mConfig.whichChip ); !attr  )
    {
      return Aurora::Memory::Status::ERR_UNSUPPORTED;
    }

    /*-------------------------------------------------------------------------
    Erase the whole chip
    -------------------------------------------------------------------------*/
    return this->erase( attr->startAddress, attr->endAddress );
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

    return static_cast<bool>( mDriver );
  }

}  // namespace Aurora::Flash::EEPROM
