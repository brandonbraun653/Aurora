/******************************************************************************
 *  File Name:
 *    db_sto_types.hpp
 *
 *  Description:
 *    Storage types for NVM
 *
 *  2022 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef AURORA_DATABASE_STORAGE_TYPES_HPP
#define AURORA_DATABASE_STORAGE_TYPES_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <etl/crc.h>
#include <Aurora/source/database/db_shared_types.hpp>
#include <Chimera/source/drivers/common/compiler_intf.hpp>

namespace Aurora::Database::Persistent
{
  /*---------------------------------------------------------------------------
  Constants
  ---------------------------------------------------------------------------*/
  static constexpr uint8_t MAGIC_HEADER     = 0xAA;
  static constexpr uint8_t MAGIC_HEADER_INV = ~MAGIC_HEADER;

  /*---------------------------------------------------------------------------
  Enumerations
  ---------------------------------------------------------------------------*/
  enum ValueType : uint8_t
  {
    VALUE_TYPE_UINT8,
    VALUE_TYPE_UINT16,
    VALUE_TYPE_UINT32,
    VALUE_TYPE_UINT64,
    VALUE_TYPE_FLOAT,
    VALUE_TYPE_DOUBLE,
    VALUE_TYPE_BYTE_ARRAY,

    VALUE_TYPE_NUM_OPTIONS
  };
  // Ensure this data still fits the bitfield in the NVMHeader
  static_assert( VALUE_TYPE_NUM_OPTIONS <= 7 );

  /*---------------------------------------------------------------------------
  Classes
  ---------------------------------------------------------------------------*/
  /**
   * @brief Header that begins each data entry in memory
   * @note Expects to only hold data up to 64kB.
   */
  __packed_struct NVMHeader
  {
    const uint8_t magic_header_start = MAGIC_HEADER;
    const uint8_t magic_header_end   = MAGIC_HEADER_INV;
    uint16_t      crc;         /**< CRC-16 of the header + data */
    uint16_t      size;        /**< Byte size of the header + data */
    Key           key;         /**< Unique key associated with the data value */
    uint8_t       version : 3; /**< Structure version */
    uint8_t       type : 3;    /**< Data type following the header */
    uint8_t       _pad0 : 2;   /**< Padding for alignment */
    uint8_t       access : 3;  /**< Access permission flags */
    uint8_t       _pad1 : 5;   /**< Padding for future growth */
    uint16_t      _pad2;       /**< Padding for future growth */
  };
  static_assert( sizeof( NVMHeader ) % sizeof( uint32_t ) == 0 );

}  // namespace Aurora::Database::Persistent

#endif  /* !AURORA_DATABASE_STORAGE_TYPES_HPP */
