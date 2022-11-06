/******************************************************************************
 *  File Name:
 *    intrusive_struct.hpp
 *
 *  Description:
 *    Declarations/Utilities for intrusive data structures
 *
 *  2022 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef AURORA_DATA_STRUCTURES_HPP
#define AURORA_DATA_STRUCTURES_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace Aurora::DS
{
  /*---------------------------------------------------------------------------
  Intrusive NVM Packed Struct
  ---------------------------------------------------------------------------*/
  /**
   * @brief Minimal solution to tagging a structure with version and crc info
   * @warning This header must be the first item in the destination object!
   *
   * @see https://stackoverflow.com/a/2321761/8341975 for CRC size limits
   *
   * This structure is intended to intrusively add CRC and Versioning capabilities
   * to a generic data structure, especially one that is stored in non-volatile
   * memory or shipped across devices via some data link.
   *
   * The magic tags are defined by the user as a way to more easily identify
   * these headers in binary blobs of data.
   *
   * @code {C++}
   * struct ExampleUsage_t
   * {
   *   SecureHeader16_t header;
   *   uint32_t         some_data0;
   *   uint32_t         some_data1;
   *   uint8_t          some_buffer[ 32 ];
   * };
   * @endcode
   */
#pragma pack( push, 1 )
  struct SecureHeader16_t
  {
    uint16_t crc16;      /**< CRC is good for up to 16,383 bytes */
    uint16_t size;       /**< Size of the entire structure in bytes */
    uint8_t  version;    /**< Structure version */
    uint8_t  _pad0;      /**< Padding for future use && alignment */
    uint8_t  _magicTag0; /**< Inverse of magicTag1 */
    uint8_t  _magicTag1; /**< Inverse of magicTag0 */

    void clear()
    {
      memset( this, 0, sizeof( *this ) );
    }
  };
  static_assert( sizeof( SecureHeader16_t ) % sizeof( uint32_t ) == 0 );
#pragma pack( pop )


  namespace SH
  {
    /**
     * @brief Initializes a header with the appropriate data
     *
     * @param header      Header to modify
     * @param size        Total expected size of the entire structure containing the header
     * @param version     Current version to use
     * @param tag         Tag to use for structure identification
     * @return bool       True if init OK, false otherwise
     */
    bool initHeader( SecureHeader16_t *const header, const uint16_t size, const uint8_t version, const uint8_t tag );

    /**
     * @brief Checks to see if the structure is valid
     *
     * @param header      Pointer to the header inside the structure
     * @param size        Total size of the structure containing the header
     * @return bool       True if valid, false otherwise
     */
    bool isValid( SecureHeader16_t *const header, const size_t size );

    /**
     * @brief Adds a CRC to the given data structure
     *
     * @param header      Pointer to the header inside the structure
     * @param size        Total size of the structure containing the header
     * @return uint16_t   Calculated CRC value
     */
    uint16_t addCRC( SecureHeader16_t *const header, const size_t size );

    /**
     * @brief Calculates the current CRC value of the structure without modifying it
     *
     * @param header      Pointer to the header inside the structure
     * @param size        Total size of the structure containing the header
     * @return uint16_t   Calculated CRC value
     */
    uint16_t calcCRC( SecureHeader16_t *const header, const size_t size );
  }  // namespace SH
}  // namespace Aurora::DS

#endif /* !AURORA_DATA_STRUCTURES_HPP */
