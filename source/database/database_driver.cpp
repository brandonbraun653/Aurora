/********************************************************************************
 *  File Name:
 *    database_driver.cpp
 *
 *  Description:
 *    Database driver implementation
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* Aurora Includes */
#include <Aurora/database>

namespace Aurora::Database
{
  /*-------------------------------------------------------------------------------
  Static Functions
  -------------------------------------------------------------------------------*/
  static bool EntryKeySortCompare( EntryList::const_reference rhs, EntryList::const_reference lhs )
  {
    return rhs.key < lhs.key;
  }

  /*-------------------------------------------------------------------------------
  RAM Database Implementation
  -------------------------------------------------------------------------------*/
  RAMDB::RAMDB()
  {
  }


  RAMDB::~RAMDB()
  {
  }

  void RAMDB::reset()
  {
    this->lock();
    mEntryList.clear();
    mAllocPool.staticReset();
    this->unlock();
  }


  bool RAMDB::read( const Key &key, void *const data )
  {
  }


  bool RAMDB::write( const Key &key, const void *const data )
  {
  }


  Chimera::Status_t RAMDB::insert( const Key &key, const UserEntry &entry )
  {
  }


  Chimera::Status_t RAMDB::insert( const Key &key, const void *const data, const size_t size, const MemAccess access )
  {
  }


  Chimera::Status_t RAMDB::remove( const Key &key )
  {
    this->lock();

    /*-------------------------------------------------
    Remove the key if it exists
    -------------------------------------------------*/
    EntryList::iterator it = findKey( key );
    if( it != mEntryList.end() )
    {
      mEntryList.erase( it );
      mEntryList.sort( EntryKeySortCompare );
    }


    this->unlock();
  }


  Chimera::Status_t RAMDB::registerCallback( const CallbackId id, etl::delegate<void( size_t )> func )
  {
    /*-------------------------------------------------
    Input protection
    -------------------------------------------------*/
    if ( !( id < CallbackId::CB_NUM_OPTIONS ) )
    {
      return Chimera::Status::INVAL_FUNC_PARAM;
    }

    /*-------------------------------------------------
    Register the callback
    -------------------------------------------------*/
    this->lock();
    if ( id == CallbackId::CB_UNHANDLED )
    {
      mDelegateRegistry.register_unhandled_delegate( func );
    }
    else
    {
      mDelegateRegistry.register_delegate( id, func );
    }
    this->unlock();
    return Chimera::Status::OK;
  }


  /*-------------------------------------------------------------------------------
  RAMDB: Private Functions
  -------------------------------------------------------------------------------*/
  EntryList::iterator RAMDB::findKey( const Key &key )
  {
    return std::find_if( mEntryList.begin(), mEntryList.end(),
                         [ &key ]( const EntryList::value_type &entry ) { return entry.key == key; } );
  }


}  // namespace Aurora::Database
