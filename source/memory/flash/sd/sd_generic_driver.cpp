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
  Driver::Driver()
  {
  }


  Driver::~Driver()
  {
  }


  Aurora::Memory::Status Driver::open( const DeviceAttr *const attributes )
  {
    return Aurora::Memory::Status::ERR_FAIL;
  }


  Aurora::Memory::Status Driver::close()
  {
    return Aurora::Memory::Status::ERR_FAIL;
  }


  Aurora::Memory::Status Driver::write( const size_t chunk, const size_t offset, const void *const data, const size_t length )
  {
    return Aurora::Memory::Status::ERR_FAIL;
  }


  Aurora::Memory::Status Driver::write( const size_t address, const void *const data, const size_t length )
  {
    return Aurora::Memory::Status::ERR_FAIL;
  }


  Aurora::Memory::Status Driver::read( const size_t chunk, const size_t offset, void *const data, const size_t length )
  {
    return Aurora::Memory::Status::ERR_FAIL;
  }


  Aurora::Memory::Status Driver::read( const size_t address, void *const data, const size_t length )
  {
    return Aurora::Memory::Status::ERR_FAIL;
  }


  Aurora::Memory::Status Driver::erase( const size_t chunk )
  {
    return Aurora::Memory::Status::ERR_FAIL;
  }


  Aurora::Memory::Status Driver::erase( const size_t address, const size_t length )
  {
    return Aurora::Memory::Status::ERR_FAIL;
  }


  Aurora::Memory::Status Driver::erase()
  {
    return Aurora::Memory::Status::ERR_FAIL;
  }


  Aurora::Memory::Status Driver::flush()
  {
    return Aurora::Memory::Status::ERR_FAIL;
  }


  Aurora::Memory::Status Driver::pendEvent( const Aurora::Memory::Event event, const size_t timeout )
  {
    return Aurora::Memory::Status::ERR_FAIL;
  }
}  // namespace Aurora::Memory::Flash::SD
