/********************************************************************************
 *  File Name:
 *    database_types.hpp
 *
 *  Description:
 *    Types used in the database
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_DATABASE_TYPES_HPP
#define AURORA_DATABASE_TYPES_HPP

/* STL Includes */
#include <cstdint>

/* ETL Includes */
#include <etl/string.h>
#include <etl/list.h>
#include <etl/pool.h>
#include <etl/delegate.h>
#include <etl/delegate_service.h>

/*Aurora Includes */
#include <Aurora/source/database/database_config.hpp>


namespace Aurora::Database
{
  /*-------------------------------------------------------------------------------
  Forward Declarations
  -------------------------------------------------------------------------------*/
  struct Entry;

  /*-------------------------------------------------------------------------------
  Aliases
  -------------------------------------------------------------------------------*/
  using Key = size_t;

  /*-------------------------------------------------------------------------------
  Enumerations
  -------------------------------------------------------------------------------*/
  enum CallbackId : uint8_t
  {
    CB_UNHANDLED,
    CB_CRC_ERROR,
    CB_INVALID_KEY,
    CB_MAX_ENTRY_ERROR,
    CB_MEM_ALLOC_ERROR,
    CB_PERMISSION,

    CB_NUM_OPTIONS
  };

  /**
   *  Available storage types for data. Note that RAM and NVM are very
   *  generic on purpose so that the physical memory technology can be
   *  a wide variety of types. For example, RAM could be internal SRAM
   *  or perhaps DDR3. NVM might be NOR flash, NAND, FRAM, etc.
   */
  enum class Storage : uint8_t
  {
    RAM0, /**< Random access memory */
    RAM1,
    RAM2,
    NVM0, /**< Non-volatile memory */
    NVM1,
    NVM2,

    NUM_OPTIONS,
    INVALID,

    /*-------------------------------------------------
    Aliases
    -------------------------------------------------*/
    RAM_DEVICE_START = RAM0,
    RAM_DEVICE_END   = RAM2,
    NVM_DEVICE_START = NVM0,
    NVM_DEVICE_END   = NVM2,
  };

  enum MemAccess : uint8_t
  {
    MEM_INVALID    = 0,
    MEM_READ       = ( 1u << 0 ),                              /**< Read access */
    MEM_WRITE      = ( 1u << 1 ),                              /**< Write access */
    MEM_WRITE_BACK = ( 1u << 2 ),                              /**< Data can be written back to NVM */
    MEM_RW         = ( MEM_READ | MEM_WRITE ),                 /**< Read/write access */
    MEM_RWWB       = ( MEM_READ | MEM_WRITE | MEM_WRITE_BACK ) /**< Full NVM access */
  };

  /*-------------------------------------------------------------------------------
  Structures
  -------------------------------------------------------------------------------*/
  /**
   *  Describes data that lives in novolatile memory. Assumes that
   *  there is an underlying filesystem in use or data can be retrieved
   *  via a string based key.
   */
  struct NVMEntry
  {
    Storage device;    /**< Which NVM device the data is stored on */
    size_t fileOffset; /**< Offset into the file to start reading from */
    size_t dataSize;   /**< How much data this entry contains */
    MemAccess access;  /**< Data access availability */
    etl::string<MAX_FILE_NAME_LENGTH> filename;

    void clear()
    {
      device     = Storage::INVALID;
      fileOffset = 0;
      dataSize   = 0;
      access     = MemAccess::MEM_INVALID;
      filename.clear();
    }
  };

  /**
   *  Describes an entry into the database from the user's perspective.
   *  Can be optionally initialized from data contained in a file.
   */
  struct UserEntry
  {
    Storage ramDevice; /**< Device to store entry data into */
    MemAccess access;  /**< Memory access permissions for the user */
    void *data;        /**< Optional data to initialize entry with */
    size_t dataSize;   /**< Size of the entry in bytes */
    NVMEntry nvm;      /**< Optional field to specify an NVM data source */

    void clear()
    {
      ramDevice = Storage::INVALID;
      access    = MemAccess::MEM_INVALID;
      data      = nullptr;
      dataSize  = 0;
      nvm.clear();
    }
  };

  /**
   *  Tracks the location of stored data in RAM
   */
  struct RawData
  {
    void *data;  /**< Entry data allocated by the database */
    size_t size; /**< Number of bytes in the entry */
  };

  /**
   *  Describes an entry into the database from the perspective of the
   *  database software. Efectively an entry control block structure.
   */
  struct Entry
  {
    Key key;          /**< Key associated with the entry */
    MemAccess access; /**< Access permissions */
    Storage device;   /**< Where this data is located */
    RawData entry;    /**< Records size and location of data */
    uint32_t crc32;   /**< CRC of the entry field */

    void clear()
    {
      key        = 0;
      access     = MemAccess::MEM_INVALID;
      device     = Storage::INVALID;
      entry.data = nullptr;
      entry.size = 0;
      crc32      = 0;
    }
  };

  /*-------------------------------------------------------------------------------
  Aliases
  -------------------------------------------------------------------------------*/
  using EntryList = etl::list_ext<Entry>;

  template<const size_t SIZE>
  using EntryStore = etl::pool<EntryList::pool_type, SIZE>;

}  // namespace Aurora::Database

#endif /* !AURORA_DATABASE_TYPES_HPP */
