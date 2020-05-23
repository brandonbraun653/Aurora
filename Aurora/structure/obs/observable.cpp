/********************************************************************************
 *  File Name:
 *    observable.cpp
 *
 *  Description:
 *    Observable implementation
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* STL Includes */
#include <algorithm>
#include <cstring>

/* Aurora Includes  */
#include <Aurora/structure/observer>

namespace Aurora::Structure::Observer
{
  Observable::Observable( ControlBlock *cb ) : mControlBlock( cb ), mUserType( EMPTY_TYPE ), mNextEmptySlot( 0 ), mRegisteredObservers( 0 )
  {
  }


  Observable::Observable() : mControlBlock( nullptr ), mUserType( EMPTY_TYPE ), mNextEmptySlot( 0 ), mRegisteredObservers( 0 )
  {
  }


  Observable::~Observable()
  {
    mControlBlock->clear();
  }


  bool Observable::initialize()
  {
    bool result = false;

    /*-------------------------------------------------
    Ensure we have exclusive access
    -------------------------------------------------*/
    mLock.lock();

    /*-------------------------------------------------
    Reset the object's state
    -------------------------------------------------*/
    if ( mControlBlock && !mControlBlock->inUse )
    {
      mControlBlock->clear();
      mControlBlock->inUse = true;
      mUserType            = EMPTY_TYPE;
      mNextEmptySlot       = 0;
      mRegisteredObservers = 0;
    }

    /*-------------------------------------------------
    Let other threads loose on the object
    -------------------------------------------------*/
    mLock.unlock();
    return result;
  }


  Chimera::Status_t Observable::registerControlBlock( ControlBlock *cb, const size_t timeout )
  {
    /*-------------------------------------------------
    Entry validation
    -------------------------------------------------*/
    if ( !cb )
    {
      return Chimera::CommonStatusCodes::INVAL_FUNC_PARAM;
    }
    else if ( cb->inUse )
    {
      return Chimera::CommonStatusCodes::FAIL;
    }
    else if ( !mLock.try_lock_for( timeout ) )
    {
      return Chimera::CommonStatusCodes::LOCKED;
    }

    /*-------------------------------------------------
    Register the control block and set it to defaults
    -------------------------------------------------*/
    mControlBlock = cb;
    initialize();

    mLock.unlock();
    return Chimera::CommonStatusCodes::OK;
  }


  Chimera::Status_t Observable::attach( IObserver *const observer, const size_t timeout )
  {
    /*-------------------------------------------------
    Entry validation
    -------------------------------------------------*/
    if ( !mLock.try_lock_for( timeout ) )
    {
      return Chimera::CommonStatusCodes::LOCKED;
    }
    else if ( mNextEmptySlot >= mControlBlock->elements )
    {
      return Chimera::CommonStatusCodes::FULL;
    }

    /*-------------------------------------------------
    Register the observer
    -------------------------------------------------*/
    mControlBlock->list[ mNextEmptySlot ] = observer;
    mNextEmptySlot++;
    mRegisteredObservers++;

    mLock.unlock();
    return Chimera::CommonStatusCodes::OK;
  }


  Chimera::Status_t Observable::detach( IObserver *const observer, const size_t timeout )
  {
    /*-------------------------------------------------
    Entry validation
    -------------------------------------------------*/
    if ( !mLock.try_lock_for( timeout ) )
    {
      return Chimera::CommonStatusCodes::LOCKED;
    }

    /*-------------------------------------------------
    Find the observer in the list and remove it
    -------------------------------------------------*/
    size_t iObserver = 0;
    for ( iObserver = 0; iObserver < mRegisteredObservers; iObserver++ )
    {
      if ( mControlBlock->list[ iObserver ] == observer )
      {
        mControlBlock->list[ iObserver ] = nullptr;
        mRegisteredObservers--;
        break;
      }
    }

    /*-------------------------------------------------
    If the observer wasn't in the list just exit now
    -------------------------------------------------*/
    if ( iObserver == mRegisteredObservers )
    {
      mLock.unlock();
      return Chimera::CommonStatusCodes::NOT_FOUND;
    }

    /*-------------------------------------------------
    The observer was found. Re-sort the list such that 
    there are no nullptr objects fragmenting the data.
    -------------------------------------------------*/
    // Sorts in descending order such that all nullptr locations are at the end of the list
    std::sort( mControlBlock->list, mControlBlock->list + mControlBlock->elements,
               []( IObserver *a, IObserver *b ) { return static_cast<bool>( a ); } );

    // Given zero indexing, this statement is correct
    mNextEmptySlot = mRegisteredObservers;

    mLock.unlock();
    return Chimera::CommonStatusCodes::OK;
  }


  Chimera::Status_t Observable::update( UpdateArgs *event, const size_t timeout )
  {
    /*-------------------------------------------------
    Entry validation
    -------------------------------------------------*/
    if ( !mLock.try_lock_for( timeout ) )
    {
      return Chimera::CommonStatusCodes::LOCKED;
    }

    /*-------------------------------------------------
    Inform all the observers of the event
    -------------------------------------------------*/
    for ( size_t x = 0; x < mRegisteredObservers; x++ )
    {
      mControlBlock->list[ x ]->update( event );
    }

    mLock.unlock();
    return Chimera::CommonStatusCodes::OK;
  }


  void Observable::setType( const size_t type )
  {
    mUserType = type;   // Atomic operation, don't need lock.
  }


  size_t Observable::getType()
  {
    return mUserType;   // Atomic operation, don't need lock.
  }

}  // namespace Aurora::Structure::Observer
