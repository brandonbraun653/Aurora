/********************************************************************************
 *  File Name:
 *    circular_buffer.hpp
 *
 *  Description:
 *    Implemenation of a circular buffer
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_CIRCULAR_BUFFER_HPP
#define AURORA_CIRCULAR_BUFFER_HPP

/* STL Includes */
#include <cstddef>
#include <cstdint>

/* Chimera Includes */
#include <Chimera/common>
#include <Chimera/thread>

namespace Aurora::Container
{
  /**
   *  Generic circular buffer optimized for use in embedded systems where
   *  dynamically allocated memory is typically frowned upon and access
   *  inside of ISR contexts are common.
   *
   *  This implementation is thread safe with the assumption that bool and
   *  size_t access on the target system can be considered atomic. Careful
   *  system architecture will be needed for safe ISR access. To guarantee
   *  safe access, the ISR signal that could interrupt should be disabled
   *  while the non-ISR operation is occuring.
   */
  template<typename T>
  class CircularBuffer
  {
  public:
    /*-------------------------------------------------
    Initialization
    -------------------------------------------------*/
    /**
     *  Default constructor. Requires calling the init method later.
     */
    CircularBuffer() : mFull( false ), mBuffer( nullptr ), mSize( 0 ), mMaxSize( 0 ), mHead( 0 ), mTail( 0 )
    {
    }

    /**
     *  Default destructor
     */
    ~CircularBuffer()
    {
    }

    /**
     *  Create a circular buffer from statically allocated memory
     *
     *  @param[in]  buffer      Pointer to static allocated memory
     *  @param[in]  size        Number of elements that the buffer can hold (not byte size)
     */
    explicit CircularBuffer( T *const buffer, const size_t size ) :
        mFull( false ), mBuffer( buffer ), mSize( 0 ), mMaxSize( size ), mHead( 0 ), mTail( 0 )
    {
      if ( !buffer || !size )
      {
        mBuffer = nullptr;
        mSize   = 0;
      }
    }

    /**
     *  Initialize a circular buffer object. Typically only used
     *  when the object was created with the default constructor.
     *
     *  @param[in]  buffer      Pointer to static allocated memory
     *  @param[in]  size        Number of elements that the buffer can hold (not byte size)
     *  @return bool
     */
    bool init( T *const buffer, const size_t size )
    {
      /*-------------------------------------------------
      Input protection
      -------------------------------------------------*/
      if( !buffer || !size )
      {
        mBuffer = nullptr;
        mSize   = 0;
        return false;
      }

      /*-------------------------------------------------
      Reset the buffer to default state
      -------------------------------------------------*/
      mBuffer = buffer;
      reset();
      return true;
    }

    /**
     *  Resets the buffer to the empty state
     *  @return void
     */
    void reset()
    {
      mMutex.lock();
      mHead = 0;
      mTail = 0;
      mMutex.unlock();
    }

    /**
     *  Checks if the buffer contains zero elements
     *  @return bool
     */
    bool empty()
    {
      /*-------------------------------------------------
      Require lock as the empty check isn't atomic
      -------------------------------------------------*/
      mMutex.lock();
      bool result = ( !mFull && ( mHead == mTail ) );
      mMutex.unlock();

      return result;
    }

    /**
     *  Checks if the buffer is filled to capacity
     *  @return bool
     */
    bool full() const
    {
      return mFull;
    }

    /**
     *  Returns the max number of elements the buffer can hold
     *  @return size_t
     */
    size_t capacity() const
    {
      return mMaxSize;
    }

    /**
     *  Returns the current number of elements in the buffer
     *  @return size_t
     */
    size_t size() const
    {
      return mSize;
    }

    /**
     *  Pushes data into the buffer, overwriting the oldest element.
     *
     *  @param[in]  data        The element to write
     *  @return void
     */
    void pushOverwrite( const T& data )
    {
      mMutex.lock();
      pushOverwriteFromISR( data );
      mMutex.unlock();
    }

    /**
     *  Pushes data into the buffer, overwriting the oldest element.
     *  This function should only be called from an ISR due to the
     *  lack of mutex protection.
     *
     *  @param[in]  data        The element to write
     *  @return void
     */
    void pushOverwriteFromISR( const T& data)
    {
      mBuffer[ mHead ] = data;

      if( mFull )
      {
        mTail = ( mTail + 1 ) % mMaxSize;
      }
      else
      {
        mSize++;
      }

      mHead = ( mHead + 1 ) % mMaxSize ;
      mFull = ( mHead == mTail );
    }

    /**
     *  Pushes data into the buffer, but first checks if there is
     *  enough room to contain it.
     *
     *  @param[in] data         The element to write
     *  @return bool            True if the write succeeds, else False
     */
    bool push( const T& data)
    {
      bool result = false;

      mMutex.lock();
      result = pushFromISR( data );
      mMutex.unlock();
      return result;
    }

    /**
     *  Pushes data into the buffer, but first checks if there is
     *  enough room to contain it. This function should only be called
     *  from an ISR due to the lack of mutex protection.
     *
     *  @param[in] data         The element to write
     *  @return bool            True if the write succeeds, else False
     */
    bool pushFromISR( const T &data )
    {
      if( !mFull )
      {
        mBuffer[ mHead ] = data;

        mSize++;
        mHead  = ( mHead + 1 ) % mMaxSize;
        mFull  = ( mHead == mTail );
        return true;
      }

      return false;
    }

    /**
     *  Pops the data off the buffer. Should the buffer be empty, will
     *  return an object that has been default constructed.
     *
     *  @return T
     */
    T pop()
    {
      /*-------------------------------------------------
      Pop lock and drop it
      -------------------------------------------------*/
      mMutex.lock();
      T result = popFromISR();
      mMutex.unlock();

      return result;
    }

    /**
     *  Pops the data off the buffer. Should the buffer be empty, will
     *  return an object that has been default constructed. This function
     *  should only be called from an ISR due to the lack of mutex protection.
     *
     *  @return T
     */
    T popFromISR()
    {
      /*-------------------------------------------------
      Return a default contstructed object if empty
      -------------------------------------------------*/
      if( empty() )
      {
        return T();
      }

      /*-------------------------------------------------
      Pull out an object since it must exist
      -------------------------------------------------*/
      T tmp = mBuffer[ mTail ];

      mTail = ( mTail + 1 ) % mMaxSize;
      mSize--;
      mFull = false;
      return tmp;
    }

  private:
    bool mFull;
    T* mBuffer;
    size_t mSize;
    size_t mMaxSize;
    size_t mHead;
    size_t mTail;
    Chimera::Threading::RecursiveMutex mMutex;
  };
}  // namespace Aurora::Container

#endif  /* !AURORA_CIRCULAR_BUFFER_HPP */
