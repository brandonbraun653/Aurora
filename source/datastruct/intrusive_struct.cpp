/******************************************************************************
 *  File Name:
 *    intrusive_struct.cpp
 *
 *  Description:
 *    Implementation of intrusive struct details
 *
 *  2022 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/datastruct>
#include <Chimera/assert>
#include <etl/crc.h>

namespace Aurora::DS
{
  namespace SH
  {
    static constexpr size_t SH16_MAX_BYTE_SIZE = 16383;

    bool initHeader( SecureHeader16_t *const header, const uint16_t size, const uint8_t version, const uint8_t tag )
    {
      /*-----------------------------------------------------------------------
      Input Protection
      -----------------------------------------------------------------------*/
      RT_DBG_ASSERT( header );

      if( size > SH16_MAX_BYTE_SIZE )
      {
        return false;
      }

      /*-----------------------------------------------------------------------
      Initialize the header. Leave the CRC alone to be figured out later.
      -----------------------------------------------------------------------*/
      header->clear();
      header->size       = size;
      header->version    = version;
      header->_magicTag0 = tag;
      header->_magicTag1 = ~tag;

      return true;
    }


    bool isValid( SecureHeader16_t *const header, const size_t size )
    {
      /*-----------------------------------------------------------------------
      CRC must match and the tag complements should cancel out
      -----------------------------------------------------------------------*/
      RT_DBG_ASSERT( header );
      return ( calcCRC( header, size ) == header->crc16 ) && ( 0x00 == ( header->_magicTag0 & header->_magicTag1 ) );
    }


    uint16_t addCRC( SecureHeader16_t *const header, const size_t size )
    {
      RT_DBG_ASSERT( header );
      header->crc16 = calcCRC( header, size );
      return header->crc16;
    }


    uint16_t calcCRC( SecureHeader16_t *const header, const size_t size )
    {
      /*-----------------------------------------------------------------------
      Input Protection
      -----------------------------------------------------------------------*/
      RT_DBG_ASSERT( header );
      if( size < sizeof( SecureHeader16_t::crc16 ) )
      {
        return 0;
      }

      /*-----------------------------------------------------------------------
      Calculate the CRC and return its value
      -----------------------------------------------------------------------*/
      etl::crc16 crc_calculator;
      uint8_t   *data       = reinterpret_cast<uint8_t *>( header ) + sizeof( SecureHeader16_t::crc16 );
      size_t     bytes_left = size - sizeof( SecureHeader16_t::crc16 );

      while ( bytes_left )
      {
        crc_calculator.add( *data );
        data++;
        bytes_left--;
      }

      return crc_calculator.value();
    }

  }  // namespace SH
}  // namespace Aurora::DS
