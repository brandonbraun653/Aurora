/******************************************************************************
 *  File Name:
 *    fs_eeprom_types.cpp
 *
 *  Description:
 *    Implementation of shared EEPROM filesystem types
 *
 *  2022 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/filesystem>
#include <etl/crc32.h>

namespace Aurora::FileSystem::EEPROM
{
  /*---------------------------------------------------------------------------
  Classes
  ---------------------------------------------------------------------------*/
  size_t iMBR::entryCount() const
  {
    size_t count = 0;
    for ( size_t i = 0; i < entryLimit(); i++ )
    {
      if ( !getEntry( i )->isReset() )
      {
        count++;
      }
    }

    return count;
  }


  bool iMBR::isValid() const
  {
    return getHeader()->crc == calculateCRC();
  }


  void iMBR::reset()
  {
    /*-------------------------------------------------------------------------
    Clear out the header and entries back to defaults
    -------------------------------------------------------------------------*/
    getHeader()->reset();
    for ( size_t i = 0; i < entryLimit(); i++ )
    {
      getEntry( i )->reset();
    }

    /*-------------------------------------------------------------------------
    Update the CRC to reflect the new state
    -------------------------------------------------------------------------*/
    getHeader()->crc = calculateCRC();
  }


  uint32_t iMBR::calculateCRC() const
  {
    etl::crc32 crc_calculator;
    crc_calculator.reset();

    for ( size_t byteIdx = sizeof( uint32_t ); byteIdx < cacheSize(); byteIdx++ )
    {
      auto ptr = reinterpret_cast<const uint8_t *const>( cacheData() ) + byteIdx;
      crc_calculator.add( *ptr );
    }

    return crc_calculator.value();
  }

}  // namespace Aurora::FileSystem::EEPROM
