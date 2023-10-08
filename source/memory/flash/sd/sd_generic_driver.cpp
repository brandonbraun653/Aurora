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


  bool Driver::init( const Chimera::SDIO::Channel channel )
  {
    mSDIO = Chimera::SDIO::getDriver( channel );
    return mSDIO != nullptr;
  }


  DeviceAttr Driver::getAttributes()
  {
    Chimera::SDIO::CardInfo info;
    mSDIO->getCardInfo( info );

    DeviceAttr attr;
    attr.readSize   = info.BlockSize;
    attr.writeSize  = info.BlockSize;
    attr.eraseSize  = info.BlockSize;
    attr.blockCount = info.BlockNbr;

    return attr;
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
    /*-------------------------------------------------------------------------
    Input Protection
    -------------------------------------------------------------------------*/
    if ( ( offset != 0 ) || ( data == nullptr ) || ( length == 0 ) )
    {
      return Status::ERR_BAD_ARG;
    }

    /*-------------------------------------------------------------------------
    Compute the block count
    -------------------------------------------------------------------------*/
    const DeviceAttr attr = getAttributes();

    if ( ( length % attr.writeSize ) != 0 )
    {
      return Status::ERR_BAD_ARG;
    }

    const size_t blockCount = length / attr.writeSize;

    /*-------------------------------------------------------------------------
    Write the data
    -------------------------------------------------------------------------*/
    const auto result = mSDIO->writeBlock( chunk, blockCount, data );
    return ( result == Chimera::Status::OK ) ? Status::ERR_OK : Status::ERR_FAIL;
  }


  Status Driver::write( const size_t address, const void *const data, const size_t length )
  {
    /*-------------------------------------------------------------------------
    Compute the block index
    -------------------------------------------------------------------------*/
    const DeviceAttr attr = getAttributes();
    RT_DBG_ASSERT( attr.writeSize != 0 );
    const size_t block = address / attr.writeSize;

    /*-------------------------------------------------------------------------
    Write the data
    -------------------------------------------------------------------------*/
    return write( block, 0, data, length );
  }


  Status Driver::read( const size_t chunk, const size_t offset, void *const data, const size_t length )
  {
    /*-------------------------------------------------------------------------
    Input Protection
    -------------------------------------------------------------------------*/
    if ( ( offset != 0 ) || ( data == nullptr ) || ( length == 0 ) )
    {
      return Status::ERR_BAD_ARG;
    }

    /*-------------------------------------------------------------------------
    Compute the block count
    -------------------------------------------------------------------------*/
    const DeviceAttr attr = getAttributes();

    if ( ( length % attr.readSize ) != 0 )
    {
      return Status::ERR_BAD_ARG;
    }

    const size_t blockCount = length / attr.readSize;

    /*-------------------------------------------------------------------------
    Read the data
    -------------------------------------------------------------------------*/
    const auto result = mSDIO->readBlock( chunk, blockCount, data );
    return ( result == Chimera::Status::OK ) ? Status::ERR_OK : Status::ERR_FAIL;
  }


  Status Driver::read( const size_t address, void *const data, const size_t length )
  {
    /*-------------------------------------------------------------------------
    Compute the block index
    -------------------------------------------------------------------------*/
    const DeviceAttr attr = getAttributes();
    RT_DBG_ASSERT( attr.readSize != 0 );
    const size_t block = address / attr.readSize;

    /*-------------------------------------------------------------------------
    Read the data
    -------------------------------------------------------------------------*/
    return read( block, 0, data, length );
  }


  Status Driver::erase( const size_t chunk )
  {
    return ( mSDIO->eraseBlock( chunk, 1 ) == Chimera::Status::OK ) ? Status::ERR_OK : Status::ERR_FAIL;
  }


  Status Driver::erase( const size_t address, const size_t length )
  {
    /*-------------------------------------------------------------------------
    Compute the block index
    -------------------------------------------------------------------------*/
    const DeviceAttr attr = getAttributes();
    RT_DBG_ASSERT( attr.eraseSize != 0 );
    const size_t block = address / attr.eraseSize;

    /*-------------------------------------------------------------------------
    Compute the block count
    -------------------------------------------------------------------------*/
    if ( ( length % attr.eraseSize ) != 0 )
    {
      return Status::ERR_BAD_ARG;
    }

    const size_t blockCount = length / attr.eraseSize;

    /*-------------------------------------------------------------------------
    Erase the data
    -------------------------------------------------------------------------*/
    return ( mSDIO->eraseBlock( block, blockCount ) == Chimera::Status::OK ) ? Status::ERR_OK : Status::ERR_FAIL;
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
