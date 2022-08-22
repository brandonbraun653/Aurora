/********************************************************************************
 *  File Name:
 *    datastore_driver.cpp
 *
 *  Description:
 *    Implements the datastore manager
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* Aurora Includes */
#include <Aurora/database>
#include <Aurora/datastore>

/* Chimera Includes */
#include <Chimera/assert>

namespace Aurora::Datastore
{
  /*-------------------------------------------------------------------------------
  Manager Implementation
  -------------------------------------------------------------------------------*/
  Manager::Manager() : mObservableMap( nullptr )
  {
  }


  Manager::~Manager()
  {
  }


  void Manager::assignCoreMemory( ObservableMap *const map )
  {
    RT_HARD_ASSERT( map );
    mObservableMap = map;
  }


  void Manager::resetRegistry()
  {
    this->lock();
    mObservableMap->clear();
    this->unlock();
  }


  bool Manager::registerObservable( IObservableAttr *const observable, Database::Volatile::RAM *const database )
  {
    this->lock();

    /*-------------------------------------------------
    Room enough to contain the new observer? Does the
    key already exist in the registry?
    -------------------------------------------------*/
    bool registered = false;
    if ( !observable || !mObservableMap->available() || ( mObservableMap->find( observable->key() ) != mObservableMap->end() ) )
    {
      mCBService_registry.call<CB_REGISTER_FAIL>();
    }
    else  // Register the observer
    {
      /*-------------------------------------------------
      Register the observable into the map
      -------------------------------------------------*/
      ObservableMap::value_type tmp = { observable->key(), observable };
      mObservableMap->insert( std::move( tmp ) );

      /*-------------------------------------------------
      Let the observable know where its core data storage
      is located.
      -------------------------------------------------*/
      observable->assignDatabase( database );

      /*-------------------------------------------------
      Allocate memory in the database for the observable
      -------------------------------------------------*/
      database->insert( observable->key(), observable->size() );
      registered = true;
    }

    this->unlock();
    return registered;
  }


  void Manager::process()
  {
    this->lock();
    for ( auto iterator = mObservableMap->begin(); iterator != mObservableMap->end(); iterator++ )
    {

      // TODO: Need to process timeouts

      if( iterator->second )
      {
        iterator->second->update();
      }
      else
      {
        /* Hitting this means a nullptr dereference was about to occur */
        Chimera::insert_debug_breakpoint();
      }
    }
    this->unlock();
  }


  bool Manager::readDataSafe( const Database::Key key, void *const data, const size_t size )
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
    Does the key exist?
    -------------------------------------------------*/
    auto iterator = mObservableMap->find( key );
    if ( iterator == mObservableMap->end() )
    {
      mCBService_registry.call<CB_INVALID_KEY>();
      goto exit;
    }

    /*-------------------------------------------------
    Read out the data
    -------------------------------------------------*/
    result = iterator->second->read( data, size );

  /*-------------------------------------------------
  Common exit sequence
  -------------------------------------------------*/
  exit:
    this->unlock();
    return result;
  }


  bool Manager::requestUpdate( const Database::Key key )
  {
    bool result = false;
    this->lock();

    /*-------------------------------------------------
    Does the key exist?
    -------------------------------------------------*/
    auto iterator = mObservableMap->find( key );
    if ( iterator == mObservableMap->end() )
    {
      mCBService_registry.call<CB_INVALID_KEY>();
      goto exit;
    }

    /*-------------------------------------------------
    Read out the data
    -------------------------------------------------*/
    iterator->second->update();
    result = true;

  /*-------------------------------------------------
  Common exit sequence
  -------------------------------------------------*/
  exit:
    this->unlock();
    return result;
  }
}  // namespace Aurora::Datastore
