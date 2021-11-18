/********************************************************************************
 *  File Name:
 *    circular_buffer.hpp
 *
 *  Description:
 *    Implemenation of a circular buffer
 *
 *  2020-2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_CIRCULAR_BUFFER_HPP
#define AURORA_CIRCULAR_BUFFER_HPP

/* STL Includes */
#include <cstddef>
#include <cstdint>

/* Chimera Includes */
#include <Chimera/assert>
#include <Chimera/common>
#include <Chimera/thread>

namespace Aurora::Container
{
  /**
   * Generic circular buffer that does not assume anything about memory allocation
   * or thread/interrupt safety of memory access. Essentially this class is a manager
   * of memory given by another context.
   */
  template<typename T>
  class CircularBuffer
  {
  public:
    CircularBuffer() : mFull( false ), mBuffer( nullptr ), mSize( 0 ), mMaxSize( 0 ), mHead( 0 ), mTail( 0 )
    {
    }

    ~CircularBuffer()
    {
    }

    /**
     *  @brief Initialize a circular buffer object
     *
     *  @param buffer   Pointer to static allocated memory
     *  @param size     Number of elements that the buffer can hold (not byte size)
     *  @return bool
     */
    bool init( T *const buffer, const size_t size )
    {
      /*-----------------------------------------------------------------------
      Input protection
      -----------------------------------------------------------------------*/
      if ( !buffer || !size )
      {
        mBuffer = nullptr;
        mSize   = 0;
        return false;
      }

      /*-----------------------------------------------------------------------
      Reset the buffer to the default state
      -----------------------------------------------------------------------*/
      mBuffer  = buffer;
      mMaxSize = size;
      reset();
      return true;
    }

    /**
     *  @brief Resets the buffer to the empty state
     *  @return void
     */
    void reset()
    {
      mHead = 0;
      mSize = 0;
      mTail = 0;
    }

    /**
     *  @brief Checks if the buffer contains zero elements
     *  @return bool
     */
    bool empty()
    {
      return ( !mFull && ( mHead == mTail ) );
    }

    /**
     *  @brief Checks if the buffer is filled to capacity
     *  @return bool
     */
    bool full() const
    {
      return mFull;
    }

    /**
     *  @brief Returns the max number of elements the buffer can hold
     *  @return size_t
     */
    size_t capacity() const
    {
      return mMaxSize;
    }

    /**
     *  @brief Returns the current number of elements in the buffer
     *  @return size_t
     */
    size_t size() const
    {
      return mSize;
    }

    /**
     *  @brief Pushes data into the buffer, overwriting the oldest element.
     *
     *  @param data   The element to write
     *  @return void
     */
    void pushOverwrite( const T &data )
    {
      mBuffer[ mHead ] = data;

      if ( mFull )
      {
        mTail = ( mTail + 1 ) % mMaxSize;
      }
      else
      {
        mSize++;
      }

      mHead = ( mHead + 1 ) % mMaxSize;
      mFull = ( mHead == mTail );
    }

    /**
     *  @brief Pushes data into the buffer, but first checks if there is
     *  enough room to contain it.
     *
     *  @param data   The element to write
     *  @return bool  True if the write succeeds, else False
     */
    bool push( const T &data )
    {
      if ( !mFull )
      {
        mBuffer[ mHead ] = data;

        mSize++;
        mHead = ( mHead + 1 ) % mMaxSize;
        mFull = ( mHead == mTail );
        return true;
      }

      return false;
    }

    /**
     * @brief Pops the data off the buffer
     *
     * If empty, return an object that has been default constructed.
     *
     * @return T
     */
    T pop()
    {
      if ( empty() )
      {
        return T();
      }

      T tmp = mBuffer[ mTail ];

      mTail = ( mTail + 1 ) % mMaxSize;
      mSize--;
      mFull = false;
      return tmp;
    }

    /**
     * @brief Gets the raw memory backing the circular buffer
     * @return T*
     */
    T *data()
    {
      return mBuffer;
    }

    /**
     * @brief Gets a pointer to the last written element in the buffer
     *
     * @return T*
     */
    T* back()
    {
      /*-----------------------------------------------------------------------
      Nothing to return if empty
      -----------------------------------------------------------------------*/
      if( this->empty() )
      {
        return nullptr;
      }

      /*-----------------------------------------------------------------------
      Get the index for the last valid write location
      -----------------------------------------------------------------------*/
      int    tmp_head  = static_cast<int>( mHead );
      size_t last_head = static_cast<size_t>( ( ( tmp_head - 1 ) + mMaxSize ) % mMaxSize );
      RT_HARD_ASSERT( last_head < mMaxSize );

      return &mBuffer[ last_head ];
    }

    /**
     * @brief Gets a pointer to the oldest element in the buffer
     *
     * @return T*
     */
    T* front()
    {
      if( !this->empty() )
      {
        return &mBuffer[ mTail ];
      }
      else
      {
        return nullptr;
      }
    }

  private:
    bool   mFull;     /**< Tracks if the buffer is full of elements */
    T *    mBuffer;   /**< Raw memory buffer storing data */
    size_t mSize;     /**< Number of elements in the buffer */
    size_t mMaxSize;  /**< Max number of elements the buffer may hold */
    size_t mHead;     /**< Write location */
    size_t mTail;     /**< Read location */
  };
}  // namespace Aurora::Container

#endif /* !AURORA_CIRCULAR_BUFFER_HPP */
