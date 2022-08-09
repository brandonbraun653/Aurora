/******************************************************************************
 *  File Name:
 *    fs_eeprom_driver.cpp
 *
 *  Description:
 *    Interface for an EEPROM based file system
 *
 *  2022 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef AURORA_FILESYSTEM_EEPROM_HPP
#define AURORA_FILESYSTEM_EEPROM_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <cstdint>
#include <Aurora/source/filesystem/file_types.hpp>
#include <Chimera/common>


namespace Aurora::FileSystem::EEPROM
{
  /*---------------------------------------------------------------------------
  Constants
  ---------------------------------------------------------------------------*/

  /*---------------------------------------------------------------------------
  Public Data
  ---------------------------------------------------------------------------*/
  extern const Interface implementation; /**< Function pointers to the EEPROM file system driver */

  /*---------------------------------------------------------------------------
  Structures
  ---------------------------------------------------------------------------*/
  /**
   * @brief A single entry in the EEPROM master boot record to represent a file
   */
  __packed_struct MBREntry
  {
    /*-------------------------------------------------------------------
    Public Data
    -------------------------------------------------------------------*/
    uint32_t file_hash; /**< 32-bit hash of the file name */
    uint32_t offset;    /**< Starting offset of the file in NVM */

    /*-------------------------------------------------------------------
    Configuration Data
    -------------------------------------------------------------------*/
    enum
    {
      HASH_RESET_VALUE   = 0x2b7628db, /**< Default invalid hash value */
      OFFSET_RESET_VALUE = 0x58980644  /**< Default invalid offset value */
    };

    /*-------------------------------------------------------------------
    Public Functions
    -------------------------------------------------------------------*/
    void reset()
    {
      file_hash = HASH_RESET_VALUE;
      offset    = OFFSET_RESET_VALUE;
    }

    inline bool isReset() const
    {
      return ( file_hash == HASH_RESET_VALUE ) && ( offset == OFFSET_RESET_VALUE );
    }
  };
  static_assert( sizeof( MBREntry ) == 8, "MBREntry size is not correct" );


  /**
   * @brief Header that begins every EEPROM file system file
   * @note File data is stored immediately after this header
   */
  __packed_struct FileHeader
  {
    uint32_t crc;       /**< CRC of the file data and this header */
    uint16_t size;      /**< Size of the file in bytes */
  };


  /**
   * @brief Interface for the master boot record of the EEPROM file system
   */
  class iMBR
  {
  public:
    virtual ~iMBR() = default;

    /**
     * @brief Retrieves maximum supported files in this file system
     *
     * @return size_t
     */
    virtual size_t getEntryLimit() const = 0;

    /**
     * @brief Determines the total number of files currently present
     *
     * @return size_t
     */
    virtual size_t getEntryCount() const = 0;
  };


  /**
   * @brief Master Boot Record for the EEPROM file system
   *
   * This structure is stored at the beginning of the EEPROM. It is  used to determine if the file
   * system is initialized and if it is, where the available "files" are stored.
   *
   * @tparam _NUM_FILES    The number of files that can be stored
   */
  template<size_t _NUM_FILES>
  class MBR : public iMBR
  {
  public:
    enum
    {
      NUM_FILES = _NUM_FILES
    };

    size_t getEntryLimit() const override
    {
      return NUM_FILES;
    }

    size_t getEntryCount() const override
    {
      size_t count = 0;
      for ( size_t i = 0; i < NUM_FILES; i++ )
      {
        if ( !mbr.files[ i ].isReset() )
        {
          count++;
        }
      }
      return count;
    }

  private:
    /**
     * @brief Core data stored in NVM as the master boot record
     */
    __packed_struct NVMData
    {
      uint32_t crc;                 /**< CRC of the entire MBR structure */
      uint8_t  max_files;           /**< The maximum number of files that can be stored */
      uint8_t  _pad[ 3 ];           /**< Padding to align the data to a 4-byte boundary */
      MBREntry files[ _NUM_FILES ]; /**< Array of file entries */
    }
    mbr; /**< The actual data stored in the EEPROM */
  };

}  // namespace Aurora::FileSystem::EEPROM

#endif /* !AURORA_FILESYSTEM_EEPROM_HPP */
