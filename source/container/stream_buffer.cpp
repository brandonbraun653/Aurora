/********************************************************************************
 *  File Name:
 *    stream_buffer.cpp
 *
 *  Description:
 *    Stream buffer implementation
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/container>
#include <Chimera/utility>

namespace Aurora::Container
{
  /*---------------------------------------------------------------------------
  Stream Attributes Class Implementations
  ---------------------------------------------------------------------------*/
  StreamAttr::StreamAttr() : mQueue( nullptr ), mMutex( nullptr ), mISRSignal( Chimera::Interrupt::SIGNAL_INVALID )
  {
  }


  StreamAttr::~StreamAttr()
  {
    this->destroy();
  }


  bool StreamAttr::init( const size_t size )
  {
    /*-------------------------------------------------------------------------
    Allocate the raw memory buffer
    -------------------------------------------------------------------------*/
    uint8_t *raw_buffer = new uint8_t( size );
    if ( !raw_buffer )
    {
      return false;
    }

    /*-------------------------------------------------------------------------
    Allocate the circular buffer
    -------------------------------------------------------------------------*/
    *mQueue = new CircularBuffer<uint8_t>( raw_buffer, size );
    if( !(*mQueue ) )
    {
      delete raw_buffer;
      return false;
    }

    /*-------------------------------------------------------------------------
    Allocate the mutex
    -------------------------------------------------------------------------*/
    *mMutex = new Chimera::Thread::RecursiveMutex();
    if( !(*mMutex ))
    {
      delete raw_buffer;
      delete *mQueue;
      return false;
    }

    mDynamicMem = true;
    return true;
  }


  bool StreamAttr::init( CircularBuffer<uint8_t> *const queue, Chimera::Thread::RecursiveMutex *const mtx,
                         const Chimera::Interrupt::Signal_t signal )
  {
    /*-------------------------------------------------------------------------
    Input Protection
    -------------------------------------------------------------------------*/
    if( !queue || !mtx )
    {
      return false;
    }

    /*-------------------------------------------------------------------------
    Assign the data
    -------------------------------------------------------------------------*/
    *mQueue    = queue;
    *mMutex    = mtx;
    mISRSignal = signal;
    return true;
  }


  void StreamAttr::destroy()
  {
    /*-------------------------------------------------------------------------
    Free memory if dynamically allocated
    -------------------------------------------------------------------------*/
    if ( mDynamicMem )
    {
      delete *mMutex;
      delete ( *mQueue )->data();
      delete *mQueue;
    }

    /*-------------------------------------------------------------------------
    Reset the data members
    -------------------------------------------------------------------------*/
    mQueue     = nullptr;
    mMutex     = nullptr;
    mISRSignal = Chimera::Interrupt::SIGNAL_INVALID;
  }


  bool StreamAttr::lock()
  {
    /*-------------------------------------------------------------------------
    Input Protections
    -------------------------------------------------------------------------*/
    if( !DPTR_EXISTS( mMutex ) )
    {
      return false;
    }

    /*-------------------------------------------------------------------------
    Perform the thread lock first, then the ISR lock if necessary
    -------------------------------------------------------------------------*/
    bool retval = true;
    ( *mMutex )->lock();
    if( mISRSignal != Chimera::Interrupt::SIGNAL_INVALID )
    {
      retval = ( Chimera::Status::OK ==  Chimera::Interrupt::disableISR( mISRSignal ) );
    }

    return retval;
  }


  void StreamAttr::unlock()
  {
    /*-------------------------------------------------------------------------
    Input Protections
    -------------------------------------------------------------------------*/
    if( !DPTR_EXISTS( mMutex ) )
    {
      return;
    }

    /*-------------------------------------------------------------------------
    Do the inverse of lock call. Enable the ISR, then release the lock.
    -------------------------------------------------------------------------*/
    if( mISRSignal != Chimera::Interrupt::SIGNAL_INVALID )
    {
      Chimera::Interrupt::enableISR( mISRSignal );
    }

    ( *mMutex )->unlock();
  }


  /*---------------------------------------------------------------------------
  Stream Buffer Class Implementations
  ---------------------------------------------------------------------------*/
  StreamBuffer::StreamBuffer()
  {
  }

  StreamBuffer::~StreamBuffer()
  {
    // Only destroy memory if marked as dynamic
  }

}  // namespace Aurora::Container
