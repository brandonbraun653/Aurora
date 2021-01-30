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

/* ETL Includes */
#include <etl/crc32.h>

namespace Aurora::Database
{
  /*-------------------------------------------------------------------------------
  Static Functions
  -------------------------------------------------------------------------------*/
  static bool EntryKeySortCompare( EntryList::const_reference rhs, EntryList::const_reference lhs )
  {
    return rhs.key < lhs.key;
  }

  static uint32_t getEntryCRC32( const RawData &entry )
  {
    etl::crc32 crc_calculator;
    crc_calculator.reset();

    /* Calculate the CRC over the data field */
    for ( size_t x = 0; x < entry.size; x++ )
    {
      auto ptr = reinterpret_cast<const uint8_t *const>( entry.data ) + x;
      crc_calculator.add( *ptr );
    }

    /* Calculate the CRC over the size field */
    for ( size_t x = 0; x < sizeof( entry.size ); x++ )
    {
      auto ptr = reinterpret_cast<const uint8_t *const>( &entry.size ) + x;
      crc_calculator.add( *ptr );
    }

    return crc_calculator.value();
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
    this->lock();

    /*-------------------------------------------------
    Check to see if the key exists
    -------------------------------------------------*/
    EntryList::iterator it = findKey( key );
    if ( it == mEntryList.end() || !it->entry.data || !it->entry.size )
    {
      this->unlock();
      mDelegateRegistry.call<CB_INVALID_KEY>();
      return false;
    }

    /*-------------------------------------------------
    Does the CRC match?
    -------------------------------------------------*/
    uint32_t crc = getEntryCRC32( it->entry );
    if ( crc != it->crc32 )
    {
      this->unlock();
      mDelegateRegistry.call<CB_CRC_ERROR>();
      return false;
    }

    /*-------------------------------------------------
    Does the user want to copy data out?
    -------------------------------------------------*/
    if ( data )
    {
      memcpy( data, it->entry.data, it->entry.size );
    }

    this->unlock();
    return true;
  }


  bool RAMDB::write( const Key &key, const void *const data )
  {
    /*-------------------------------------------------
    Input protection
    -------------------------------------------------*/
    if ( !data )
    {
      return false;
    }

    this->lock();

    /*-------------------------------------------------
    Check to see if the key exists
    -------------------------------------------------*/
    EntryList::iterator it = findKey( key );
    if ( it == mEntryList.end() || !it->entry.data || !it->entry.size )
    {
      this->unlock();
      mDelegateRegistry.call<CB_INVALID_KEY>();
      return false;
    }

    /*-------------------------------------------------
    Does this data have write access permissions?
    -------------------------------------------------*/
    if ( !( it->access & MemAccess::MEM_WRITE ) )
    {
      this->unlock();
      mDelegateRegistry.call<CB_PERMISSION>();
      return false;
    }

    /*-------------------------------------------------
    Copy in the data and update the CRC
    -------------------------------------------------*/
    memcpy( it->entry.data, data, it->entry.size );
    it->crc32 = getEntryCRC32( it->entry );

    this->unlock();
    return true;
  }


  Chimera::Status_t RAMDB::insert( const Key &key, const void *const data, const size_t size, const MemAccess access )
  {
    /*-------------------------------------------------
    Input Protection
    -------------------------------------------------*/
    if ( !data || !size )
    {
      return Chimera::Status::INVAL_FUNC_PARAM;
    }

    this->lock();

    /*-------------------------------------------------
    Is there room for a new entry?
    -------------------------------------------------*/
    if( !mEntryList.available() )
    {
      this->unlock();
      mDelegateRegistry.call<CB_MAX_ENTRY_ERROR>();
      return Chimera::Status::FULL;
    }

    /*-------------------------------------------------
    Ensure the key doesn't already exist
    -------------------------------------------------*/
    if ( findKey( key ) != mEntryList.end() )
    {
      this->unlock();
      mDelegateRegistry.call<CB_INVALID_KEY>();
      return Chimera::Status::FAIL;
    }

    /*-------------------------------------------------
    Allocate storage memory and copy in the entry data
    -------------------------------------------------*/
    Entry tmp;
    tmp.clear();

    tmp.access     = access;
    tmp.key        = key;
    tmp.device     = Storage::RAM0;
    tmp.entry.size = size;
    tmp.entry.data = mAllocPool.malloc( size );

    if ( tmp.entry.data == nullptr )
    {
      this->unlock();
      mDelegateRegistry.call<CB_MEM_ALLOC_ERROR>();
      return Chimera::Status::MEMORY;
    }
    else if( data )
    {
      memcpy( tmp.entry.data, data, size );
      tmp.crc32 = getEntryCRC32( tmp.entry );
    }

    /*-------------------------------------------------
    Copy the entry into the registry list
    -------------------------------------------------*/
    mEntryList.push_back( std::move( tmp ) );
    mEntryList.sort( EntryKeySortCompare );
    this->unlock();
    return Chimera::Status::OK;
  }


  Chimera::Status_t RAMDB::remove( const Key &key )
  {
    this->lock();
    auto result = Chimera::Status::NOT_FOUND;

    /*-------------------------------------------------
    Remove the key if it exists
    -------------------------------------------------*/
    EntryList::iterator it = findKey( key );
    if ( it != mEntryList.end() )
    {
      mEntryList.erase( it );
      mEntryList.sort( EntryKeySortCompare );
      result = Chimera::Status::OK;
    }

    this->unlock();
    return result;
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
