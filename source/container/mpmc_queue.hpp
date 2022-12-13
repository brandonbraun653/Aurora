/******************************************************************************
 *  File Name:
 *    stream_buffer.hpp
 *
 *  Description:
 *    Implementation of a thread/isr safe stream buffer
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *****************************************************************************/

#pragma once
#ifndef AURORA_STREAM_BUFFER_HPP
#define AURORA_STREAM_BUFFER_HPP

/* C++ Includes */
#include <cstddef>

/* Aurora Includes */
#include <Aurora/container>

/* Chimera Includes */
#include <Chimera/interrupt>
#include <Chimera/thread>
#include <Chimera/utility>


namespace Aurora::Container
{
  /*---------------------------------------------------------------------------
  Forward Declarations
  ---------------------------------------------------------------------------*/
  template<typename T>
  class MPMCQueue;


  /*---------------------------------------------------------------------------
  Classes
  ---------------------------------------------------------------------------*/
  /**
   * Helper class for managing the lifetime and access permissions of
   * the underlying memory of a MPMCQueue class.
   */
  template<typename T>
  class MPMCAttr
  {
  public:
    MPMCAttr() : mQueue( nullptr ), mMutex( nullptr ), mISRSignal( Chimera::Interrupt::SIGNAL_INVALID ), mDynamicMem( false )
    {
    }

    ~MPMCAttr()
    {
    }

    /**
     * @brief Dynamically allocate stream memory
     *
     * @param num_elements    Number of elements to allocate
     * @return bool
     */
    bool init( const size_t num_elements )
    {
      /*-----------------------------------------------------------------------
      Allocate the raw memory buffer
      -----------------------------------------------------------------------*/
      if ( num_elements == 0 )
      {
        return false;
      }

      T *raw_buffer = new T[ num_elements ];
      RT_HARD_ASSERT( raw_buffer );

      /*-----------------------------------------------------------------------
      Allocate the circular buffer
      -----------------------------------------------------------------------*/
      mQueue = new CircularBuffer<T>();
      RT_HARD_ASSERT( mQueue );
      RT_HARD_ASSERT( mQueue->init( raw_buffer, num_elements ) );

      /*-----------------------------------------------------------------------
      Allocate the mutex
      -----------------------------------------------------------------------*/
      mMutex = new Chimera::Thread::RecursiveMutex();
      RT_HARD_ASSERT( mMutex );

      mDynamicMem = true;
      return true;
    }

    /**
     * @brief Statically initialize stream memory
     *
     * @param queue   Circular buffer to use
     * @param mtx     Thread lock to use
     * @param signal  ISR signal to protect against
     * @return bool
     */
    bool init( CircularBuffer<T> *const queue, Chimera::Thread::RecursiveMutex *const mtx,
               const Chimera::Interrupt::Signal_t signal )
    {
      /*-----------------------------------------------------------------------
      Input Protection
      -----------------------------------------------------------------------*/
      if ( !queue || !mtx )
      {
        return false;
      }

      /*-----------------------------------------------------------------------
      Assign the data
      -----------------------------------------------------------------------*/
      mQueue      = queue;
      mMutex      = mtx;
      mISRSignal  = signal;
      mDynamicMem = false;
      return true;
    }

    /**
     * @brief De-initializes the stream attributes
     */
    void destroy()
    {
      /*-----------------------------------------------------------------------
      Free memory if dynamically allocated
      -----------------------------------------------------------------------*/
      if ( mDynamicMem )
      {
        delete mMutex;
        delete[] mQueue->data();
        delete mQueue;
      }

      /*-----------------------------------------------------------------------
      Reset the data members
      -----------------------------------------------------------------------*/
      mQueue     = nullptr;
      mMutex     = nullptr;
      mISRSignal = Chimera::Interrupt::SIGNAL_INVALID;
    }

    /**
     * @brief Checks the validity of the attributes
     *
     * @return true
     * @return false
     */
    bool valid()
    {
      return ( mQueue && mMutex );
    }

  protected:
    friend class MPMCQueue<T>;

    CircularBuffer<T>               *mQueue;     /**< Memory manager */
    Chimera::Thread::RecursiveMutex *mMutex;     /**< Thread safety lock */
    Chimera::Interrupt::Signal_t     mISRSignal; /**< Which ISR signal to protect against, if any */

    /**
     * @brief Protects memory access from multithreaded and ISR access
     *
     * @return true   Memory was locked
     * @return false  Memory was not locked
     */
    void lock()
    {
      RT_HARD_ASSERT( mMutex );

      /*-----------------------------------------------------------------------
      Perform the thread lock first, then the ISR lock if necessary
      -----------------------------------------------------------------------*/
      mMutex->lock();
      Chimera::Interrupt::disableISR( mISRSignal );
    }

    /**
     * @brief Unlocks previous lock call
     */
    void unlock()
    {
      RT_HARD_ASSERT( mMutex );

      /*-----------------------------------------------------------------------
      Do the inverse of lock call. Enable the ISR, then release the lock.
      -----------------------------------------------------------------------*/
      Chimera::Interrupt::enableISR( mISRSignal );
      mMutex->unlock();
    }

    CircularBuffer<T> *queue()
    {
      return mQueue;
    }

  private:
    bool mDynamicMem; /**< Memory was dynamically allocated */
  };


  /**
   * Provides a solution for multi-producer, multi-consumer FIFO queues that
   * must function along side ISR handlers and threads. Usually the callee
   * doesn't want to manage these kinds of details, so this class helps take
   * care of it. Essentially this is a wrapper around a circular buffer.
   */
  template<typename T>
  class MPMCQueue
  {
  public:
    MPMCQueue()
    {
    }

    ~MPMCQueue()
    {
    }

    /**
     * @brief Initializes the class
     *
     * @param attr    Attributes to configure with
     * @return bool   Init success
     */
    bool init( MPMCAttr<T> &attr )
    {
      if ( attr.valid() )
      {
        mAttr = attr;
        return true;
      }

      return false;
    }

    /**
     * @brief Write data into the FIFO stream
     *
     * @param data      Input buffer of data to write
     * @param elements  Number of elements to write into the FIFO stream
     * @param safe      If true, protect against ISR & thread access
     * @return size_t   Number of elements actually written
     */
    size_t push( const T *const data, size_t elements, const bool safe = true )
    {
      /*-----------------------------------------------------------------------
      Input Protections
      -----------------------------------------------------------------------*/
      if ( !data || !elements )
      {
        return 0;
      }

      /*-----------------------------------------------------------------------
      Write the elements
      -----------------------------------------------------------------------*/
      size_t read_idx = 0;
      bool   push_ok  = true;

      enter_critical( safe );
      while ( push_ok && elements && !mAttr.queue()->full() )
      {
        push_ok = mAttr.queue()->push( data[ read_idx ] );
        read_idx++;
        elements--;
      }
      exit_critical( safe );

      return read_idx;
    }

    /**
     * @brief Read data from the FIFO stream
     *
     * @param data      Output buffer to write into
     * @param elements  Number of elements to read from the FIFO stream into the output buffer
     * @param safe      If true, protect against ISR & thread access
     * @return size_t   Number of elements actually read
     */
    size_t pop( T *const data, size_t elements, const bool safe = true )
    {
      /*-----------------------------------------------------------------------
      Input Protections
      -----------------------------------------------------------------------*/
      if ( !data || !elements )
      {
        return 0;
      }

      /*-----------------------------------------------------------------------
      Grab as many elements as possible
      -----------------------------------------------------------------------*/
      size_t write_idx = 0;

      enter_critical( safe );
      while ( elements && !mAttr.queue()->empty() )
      {
        data[ write_idx ] = mAttr.queue()->pop();
        write_idx++;
        elements--;
      }
      exit_critical( safe );

      return write_idx;
    }

    /**
     * @brief Checks if the FIFO is empty
     *
     * @param safe      If true, protect against ISR & thread access
     * @return true   FIFO is empty
     * @return false  FIFO is not empty
     */
    bool empty( const bool safe = true )
    {
      enter_critical( safe );
      bool result = mAttr.queue()->empty();
      exit_critical( safe );

      return result;
    }

    /**
     * @brief Checks if the FIFO is full
     *
     * @param safe    If true, protect against ISR & thread access
     * @return true
     * @return false
     */
    bool full( const bool safe = true )
    {
      enter_critical( safe );
      bool result = mAttr.queue()->full();
      exit_critical( safe );

      return result;
    }

    /**
     * @brief Returns the total number of elements the FIFO may hold
     *
     * @param safe      If true, protect against ISR & thread access
     * @return size_t
     */
    size_t capacity( const bool safe = true )
    {
      enter_critical( safe );
      size_t result = mAttr.queue()->capacity();
      exit_critical( safe );

      return result;
    }

    /**
     * @brief Returns the remaining number of elements in the FIFO
     *
     * @param safe      If true, protect against ISR & thread access
     * @return size_t
     */
    size_t available( const bool safe = true )
    {
      enter_critical( safe );
      size_t result = mAttr.queue()->capacity() - mAttr.queue()->size();
      exit_critical( safe );

      return result;
    }

    /**
     * @brief Returns the total used elements in the FIFO
     *
     * @param safe      If true, protect against ISR & thread access
     * @return size_t
     */
    size_t size( const bool safe = true )
    {
      enter_critical( safe );
      size_t result = mAttr.queue()->size();
      exit_critical( safe );

      return result;
    }

  private:
    MPMCAttr<T> mAttr; /**< Configuration attributes */

    inline void enter_critical( bool should_enter )
    {
      if ( should_enter )
      {
        mAttr.lock();
      }
    }

    inline void exit_critical( bool should_exit )
    {
      if ( should_exit )
      {
        mAttr.unlock();
      }
    }
  };
}  // namespace Aurora::Container

#endif /* !AURORA_STREAM_BUFFER_HPP */
