/********************************************************************************
 *  File Name:
 *    database_intf.hpp
 *
 *  Description:
 *    User interface to the database
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_DATABASE_INTF_HPP
#define AURORA_DATABASE_INTF_HPP

/* STL Includes */
#include <cstddef>
#include <cstdint>

/* Aurora Includes */
#include <Aurora/source/memory/generic/generic_intf.hpp>
#include <Aurora/source/memory/heap/heap.hpp>
#include <Aurora/source/database/database_types.hpp>

/* Chimera Includes */
#include <Chimera/assert>
#include <Chimera/thread>

namespace Aurora::Database
{
  /*-------------------------------------------------------------------------------
  Forward Declarations
  -------------------------------------------------------------------------------*/
  class RAM;

  /*-------------------------------------------------------------------------------
  Classes
  -------------------------------------------------------------------------------*/
  class RAM : public Chimera::Thread::Lockable<RAM>
  {
  public:
    RAM();
    ~RAM();

    /**
     *  Assigns the memory used in the RAM database. Both the entry store and heap
     *  memory **must** be statically allocated to guarantee it's existence for the
     *  duration of the program. Dynamically allocated memory is supported technically,
     *  but this class assumes the memory will always exist, so the lifetime management
     *  aspects of the memory are left up to the user.
     *
     *  @param[in]  store     Memory pool for storing database entry control blocks
     *  @param[in]  heap      Raw memory buffer to allocate entry data from
     *  @param[in]  size      Number of bytes contained in the heap
     *  @return void
     */
    template<const size_t NUM_ELEM>
    void assignCoreMemory( EntryStore<NUM_ELEM> &store, void *const heap, const size_t size )
    {
      mEntryList.set_pool( store );
      if ( !mAllocPool.assignPool( heap, size ) )
      {
        RT_HARD_ASSERT( false );
      }
    }

    /**
     *  Empties the entire database to contain no entries
     *  @return void
     */
    void reset();

    /**
     *  Reads the database entry for the given key.
     *
     *  @warning The data field must be able to contain the size registered with the key
     *
     *  @param[in]  key       Key to look up
     *  @param[out] data      Output buffer to write data into
     *  @return bool          Validity of the read. If false, CRC failed or key doesn't exist.
     */
    bool read( const Key &key, void *const data );

    /**
     *  Writes the database with a new value for the given key
     *
     *  @warning The data field must be the same size as registered with the key
     *
     *  @param[in]  key       Key to write into
     *  @param[in]  data      Data used to update the database
     *  @return bool          Validity. If false, the key doesn't exist or an internal error occurred.
     */
    bool write( const Key &key, const void *const data );

    /**
     *  Inserts a new entry into the database with default settings
     *
     *  @param[in]  key       Key that is being created
     *  @param[in]  size      Number of bytes to allocate for the data
     *  @return Chimera::Status_t
     */
    Chimera::Status_t insert( const Key &key, const size_t size );


    /**
     *  Inserts a new entry into the database using the default store
     *
     *  @param[in]  key       Key that is being created
     *  @param[in]  data      Data to initialize the entry with. May be left null.
     *  @param[in]  size      Number of bytes to allocate for the data
     *  @param[in]  access    User access permissions
     *  @return Chimera::Status_t
     */
    Chimera::Status_t insert( const Key &key, const void *const data, const size_t size, const MemAccess access );

    /**
     *  Removes an entry from the database and frees all allocated memory
     *
     *  @warning Beware of fragmentation if create/free is performed often enough
     *
     *  @param[in]  key       Key to be removed
     *  @return Chimera::Status_t
     */
    Chimera::Status_t remove( const Key &key );

    /**
     *  Gets the size allocated for a particular key
     *
     *  @param[in]  key       Key to look up
     *  @return size_t        Zero if not found
     */
    size_t size( const Key &key );

    /**
     *  Register a callback to be invoked upon some event that occurs during
     *  the service processing.
     *
     *  @param[in]  id          Which event to register against
     *  @param[in]  func        The function to register
     *  @return Chimera::Status_t
     */
    Chimera::Status_t registerCallback( const CallbackId id, etl::delegate<void( size_t )> func );

  private:
    friend Chimera::Thread::Lockable<RAM>;

    /*-------------------------------------------------
    Core database memory
    -------------------------------------------------*/
    EntryList mEntryList;            /**< Stores control blocks for all registered entries */
    Memory::Heap mAllocPool;         /**< Raw memory pool to allocate new entries from */

    /*-------------------------------------------------
    Helper for tracking/invoking callbacks
    -------------------------------------------------*/
    etl::delegate_service<CallbackId::CB_NUM_OPTIONS> mCBService_registry;

    /*-------------------------------------------------
    Helper Functions
    -------------------------------------------------*/
    EntryList::iterator findKey( const Key &key );
  };

}  // namespace Aurora::Database

#endif /* !AURORA_DATABASE_INTF_HPP */
