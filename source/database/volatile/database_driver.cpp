/********************************************************************************
 *  File Name:
 *    database_driver.cpp
 *
 *  Description:
 *    Database driver implementation
 *
 *  2021-2022 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* Aurora Includes */
#include <Aurora/database>

/* ETL Includes */
#include <etl/crc32.h>

namespace Aurora::Database::Volatile
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
  RAM::RAM()
  {
  }


  RAM::~RAM()
  {
  }


  void RAM::reset()
  {
    this->lock();
    mEntryList.clear();
    this->unlock();
  }


  bool RAM::read( const Key &key, void *const data )
  {
    bool result  = false;
    uint32_t crc = 0;

    this->lock();

    /*-------------------------------------------------
    Check to see if the key exists
    -------------------------------------------------*/
    EntryList::iterator it = findKey( key );
    if ( it == mEntryList.end() || !it->entry.data || !it->entry.size )
    {
      mCBService_registry.call<CB_INVALID_KEY>();
      goto exit;
    }

    /*-------------------------------------------------
    Does the CRC match?
    -------------------------------------------------*/
    crc = getEntryCRC32( it->entry );
    if ( crc != it->crc32 )
    {
      mCBService_registry.call<CB_CRC_ERROR>();
      goto exit;
    }

    /*-------------------------------------------------
    Does the user want to copy data out?
    -------------------------------------------------*/
    if ( data )
    {
      memcpy( data, it->entry.data, it->entry.size );
    }

    result = true;

    /*-------------------------------------------------
    Common exit routine
    -------------------------------------------------*/
  exit:
    this->unlock();
    return result;
  }


  bool RAM::write( const Key &key, const void *const data )
  {
    /*-------------------------------------------------
    Input protection
    -------------------------------------------------*/
    if ( !data )
    {
      return false;
    }

    bool result = false;
    this->lock();

    /*-------------------------------------------------
    Check to see if the key exists
    -------------------------------------------------*/
    EntryList::iterator it = findKey( key );
    if ( it == mEntryList.end() || !it->entry.data || !it->entry.size )
    {
      mCBService_registry.call<CB_INVALID_KEY>();
      goto exit;
    }

    /*-------------------------------------------------
    Does this data have write access permissions?
    -------------------------------------------------*/
    if ( !( it->access & MemAccess::MEM_WRITE ) )
    {
      mCBService_registry.call<CB_PERMISSION>();
      goto exit;
    }

    /*-------------------------------------------------
    Copy in the data and update the CRC
    -------------------------------------------------*/
    memcpy( it->entry.data, data, it->entry.size );
    it->crc32 = getEntryCRC32( it->entry );
    result    = true;

    /*-------------------------------------------------
    Common exit routine
    -------------------------------------------------*/
  exit:
    this->unlock();
    return result;
  }


  Chimera::Status_t RAM::insert( const Key &key, const size_t size )
  {
    return insert( key, nullptr, size, MemAccess::MEM_RW );
  }


  Chimera::Status_t RAM::insert( const Key &key, const void *const data, const size_t size, const MemAccess access )
  {
    /*-------------------------------------------------
    Input Protection
    -------------------------------------------------*/
    if ( !size )
    {
      return Chimera::Status::INVAL_FUNC_PARAM;
    }

    auto result = Chimera::Status::OK;
    this->lock();

    /*-------------------------------------------------
    Is there room for a new entry?
    -------------------------------------------------*/
    if ( !mEntryList.available() )
    {
      mCBService_registry.call<CB_MAX_ENTRY_ERROR>();
      result = Chimera::Status::FULL;
      goto exit;
    }

    /*-------------------------------------------------
    Ensure the key doesn't already exist
    -------------------------------------------------*/
    if ( findKey( key ) != mEntryList.end() )
    {
      mCBService_registry.call<CB_INVALID_KEY>();
      result = Chimera::Status::FAIL;
      goto exit;
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
      mCBService_registry.call<CB_MEM_ALLOC_ERROR>();
      result = Chimera::Status::MEMORY;
      goto exit;
    }
    else if ( data )
    {
      memcpy( tmp.entry.data, data, size );
    }
    else
    {
      memset( tmp.entry.data, 0, size );
    }

    tmp.crc32 = getEntryCRC32( tmp.entry );

    /*-------------------------------------------------
    Copy the entry into the registry list
    -------------------------------------------------*/
    mEntryList.push_back( std::move( tmp ) );
    mEntryList.sort( EntryKeySortCompare );

    /*-------------------------------------------------
    Common exit routine
    -------------------------------------------------*/
  exit:
    this->unlock();
    return result;
  }


  Chimera::Status_t RAM::remove( const Key &key )
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


  size_t RAM::size( const Key &key )
  {
    this->lock();

    size_t size = 0;
    EntryList::iterator it = findKey( key );
    if ( it != mEntryList.end() )
    {
      size = it->entry.size;
    }

    this->unlock();
    return size;
  }


  Chimera::Status_t RAM::registerCallback( const CallbackId id, etl::delegate<void( size_t )> func )
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
      mCBService_registry.register_unhandled_delegate( func );
    }
    else
    {
      mCBService_registry.register_delegate( id, func );
    }

    this->unlock();
    return Chimera::Status::OK;
  }


  /*-------------------------------------------------------------------------------
  RAM: Private Functions
  -------------------------------------------------------------------------------*/
  EntryList::iterator RAM::findKey( const Key &key )
  {
    return std::find_if( mEntryList.begin(), mEntryList.end(),
                         [ &key ]( const EntryList::value_type &entry ) { return entry.key == key; } );
  }

}  // namespace Aurora::Database
