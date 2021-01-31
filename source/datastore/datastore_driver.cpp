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


  bool Manager::registerObservable( IObservableAttr &observable )
  {
    this->lock();

    /*-------------------------------------------------
    Room enough to contain the new observer? Does the
    key already exist in the registry?
    -------------------------------------------------*/
    bool registered = false;
    if ( !mObservableMap->available() || ( mObservableMap->find( observable.key() ) != mObservableMap->end() ) )
    {
      mDelegateRegistry.call<CB_REGISTER_FAIL>();
    }
    else  // Register the observer
    {
      ObservableMap::value_type tmp = { observable.key(), observable };
      mObservableMap->insert( tmp );
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
      iterator->second.update();
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
      mDelegateRegistry.call<CB_INVALID_KEY>();
      goto exit;
    }

    /*-------------------------------------------------
    Read out the data
    -------------------------------------------------*/
    result = iterator->second.read( data, size );

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
      mDelegateRegistry.call<CB_INVALID_KEY>();
      goto exit;
    }

    /*-------------------------------------------------
    Read out the data
    -------------------------------------------------*/
    iterator->second.update();
    result = true;

  /*-------------------------------------------------
  Common exit sequence
  -------------------------------------------------*/
  exit:
    this->unlock();
    return result;
  }
}  // namespace Aurora::Datastore
