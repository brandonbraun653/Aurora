/******************************************************************************
 *  File Name:
 *    sd_generic_driver.cpp
 *
 *  Description:
 *    SD Card memory interface
 *
 *  2023 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/memory>

namespace Aurora::Memory::Flash::SD
{
  /*---------------------------------------------------------------------------
  Driver Class Implementation
  ---------------------------------------------------------------------------*/

  Driver::Driver() : mSDIO( nullptr )
  {
  }


  Driver::~Driver()
  {
  }


  bool Driver::init( const Chimera::SDIO::HWConfig &cfg )
  {
    /*-------------------------------------------------------------------------
    Grab a reference to the SDIO driver
    -------------------------------------------------------------------------*/
    mSDIO = Chimera::SDIO::getDriver( cfg.channel );
    if ( !mSDIO )
    {
      return false;
    }

    /*-------------------------------------------------------------------------
    Initialize the SDIO driver
    -------------------------------------------------------------------------*/
    if( mSDIO->open( cfg ) != Chimera::Status::OK )
    {
      return false;
    }

    return true;
  }


  Status Driver::open( const DeviceAttr *const attributes )
  {
    ( void )attributes;
    return ( mSDIO->connect() == Chimera::Status::OK ) ? Status::ERR_OK : Status::ERR_FAIL;
  }


  Status Driver::close()
  {
    mSDIO->close();
    return Status::ERR_OK;
  }


  Status Driver::write( const size_t chunk, const size_t offset, const void *const data, const size_t length )
  {
    // Offset should be zero. Can't write partial blocks.
    // Length will need to be converted into the number of blocks to write
    // Chunk should == block index
    return Status::ERR_FAIL;
  }


  Status Driver::write( const size_t address, const void *const data, const size_t length )
  {
    // Address should be the block index, not the phsyical byte address
    // Length will need to be converted into the number of blocks to write
    return Status::ERR_FAIL;
  }


  Status Driver::read( const size_t chunk, const size_t offset, void *const data, const size_t length )
  {
    // Same notes as write
    return Status::ERR_FAIL;
  }


  Status Driver::read( const size_t address, void *const data, const size_t length )
  {
    // Same notes as write
    return Status::ERR_FAIL;
  }


  Status Driver::erase( const size_t chunk )
  {
    return ( mSDIO->eraseBlock( chunk, 1 ) == Chimera::Status::OK ) ? Status::ERR_OK : Status::ERR_FAIL;
  }


  Status Driver::erase( const size_t address, const size_t length )
  {
    // Must be aligned to a block boundary. Length must be a multiple of the block size.
    return Status::ERR_FAIL;
  }


  Status Driver::erase()
  {
    /*-------------------------------------------------------------------------
    SDIO driver doesn't support mass erase
    -------------------------------------------------------------------------*/
    return Status::ERR_UNSUPPORTED;
  }


  Status Driver::flush()
  {
    /*-------------------------------------------------------------------------
    SDIO driver doesn't cache anything, so this is a no-op
    -------------------------------------------------------------------------*/
    return Status::ERR_OK;
  }


  Status Driver::pendEvent( const Aurora::Memory::Event event, const size_t timeout )
  {
    return Status::ERR_UNSUPPORTED;
  }
}  // namespace Aurora::Memory::Flash::SD
