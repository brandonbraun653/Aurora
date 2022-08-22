/******************************************************************************
 *  File Name:
 *    db_mem_api.hpp
 *
 *  Description:
 *    Memory access layer interface
 *
 *  2022 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef AURORA_DATABASE_MEMORY_INTERFACE_HPP
#define AURORA_DATABASE_MEMORY_INTERFACE_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/


namespace Aurora::Database::Persistent
{
  /*---------------------------------------------------------------------------
  Classes
  ---------------------------------------------------------------------------*/
  class IMemoryController
  {
  public:
    virtual ~IMemoryController() = default;

    /**
     * @brief Opens the memory controller and readies it for operation
     *
     * @return Zero if ready, another number if not
     */
    virtual int open()
    {
      return -1;
    }

    /**
     * @brief Reads data from a fixed address
     *
     * @param data      Buffer to read into
     * @param size      How many bytes to read
     * @param address   Where to read from (absolute)
     * @return Zero if ok, another number if not
     */
    virtual int read( void *const data, const size_t size, const size_t address )
    {
      return -1;
    }

    /**
     * @brief Writes data to a fixed address
     *
     * @param data      Buffer to write from
     * @param size      How many bytes to write
     * @param address   Where to write to (absolute)
     * @return Zero if ok, another number if not
     */
    virtual int write( const void *const data, const size_t size, const size_t address )
    {
      return -1;
    }

    /**
     * @brief Erase a range of memory
     *
     * @param address   Starting address to erase at
     * @param size      Number of bytes to erase
     * @return Zero if ok, another number if not
     */
    virtual int erase( const size_t address, const size_t size )
    {
      return -1;
    }

    /**
     * @brief Closes the memory controller
     *
     * @return Zero if closed, another number if not
     */
    virtual int close()
    {
      return -1;
    }
  };
}  // namespace Aurora::Database::Persistent

#endif /* !AURORA_DATABASE_MEMORY_INTERFACE_HPP */
