/********************************************************************************
 *  File Name:
 *    stream_buffer.hpp
 *
 *  Description:
 *    Implementation of a thread/isr safe stream buffer
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

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
  class StreamBuffer;


  /*---------------------------------------------------------------------------
  Classes
  ---------------------------------------------------------------------------*/
  /**
   * Helper class for managing the lifetime and access permissions of
   * the underlying memory of a StreamBuffer class.
   */
  template<typename T>
  class StreamAttr
  {
  public:
    StreamAttr() : mQueue( nullptr ), mMutex( nullptr ), mISRSignal( Chimera::Interrupt::SIGNAL_INVALID ), mDynamicMem( false )
    {
    }

    ~StreamAttr()
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
      /*-------------------------------------------------------------------------
      Allocate the raw memory buffer
      -------------------------------------------------------------------------*/
      T *raw_buffer = new T( num_elements );
      if ( !raw_buffer )
      {
        return false;
      }

      /*-------------------------------------------------------------------------
      Allocate the circular buffer
      -------------------------------------------------------------------------*/
      *mQueue = new CircularBuffer<T>( raw_buffer, num_elements );
      if ( !( *mQueue ) )
      {
        delete raw_buffer;
        return false;
      }

      /*-------------------------------------------------------------------------
      Allocate the mutex
      -------------------------------------------------------------------------*/
      *mMutex = new Chimera::Thread::RecursiveMutex();
      if ( !( *mMutex ) )
      {
        delete raw_buffer;
        delete *mQueue;
        return false;
      }

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
      /*-------------------------------------------------------------------------
      Input Protection
      -------------------------------------------------------------------------*/
      if ( !queue || !mtx )
      {
        return false;
      }

      /*-------------------------------------------------------------------------
      Assign the data
      -------------------------------------------------------------------------*/
      *mQueue     = queue;
      *mMutex     = mtx;
      mISRSignal  = signal;
      mDynamicMem = false;
      return true;
    }

    /**
     * @brief De-initializes the stream attributes
     */
    void destroy()
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

  protected:
    friend class StreamBuffer<T>;

    CircularBuffer<T> **              mQueue;     /**< Memory manager */
    Chimera::Thread::RecursiveMutex **mMutex;     /**< Thread safety lock */
    Chimera::Interrupt::Signal_t      mISRSignal; /**< Which ISR signal to protect against, if any */

    /**
     * @brief Protects memory access from multithreaded and ISR access
     *
     * @return true   Memory was locked
     * @return false  Memory was not locked
     */
    bool lock()
    {
      /*-------------------------------------------------------------------------
      Input Protections
      -------------------------------------------------------------------------*/
      if ( !DPTR_EXISTS( mMutex ) )
      {
        return false;
      }

      /*-------------------------------------------------------------------------
      Perform the thread lock first, then the ISR lock if necessary
      -------------------------------------------------------------------------*/
      bool retval = true;
      ( *mMutex )->lock();
      if ( mISRSignal != Chimera::Interrupt::SIGNAL_INVALID )
      {
        retval = ( Chimera::Status::OK == Chimera::Interrupt::disableISR( mISRSignal ) );
      }

      return retval;
    }

    /**
     * @brief Unlocks previous lock call
     */
    void unlock()
    {
      /*-------------------------------------------------------------------------
      Input Protections
      -------------------------------------------------------------------------*/
      if ( !DPTR_EXISTS( mMutex ) )
      {
        return;
      }

      /*-------------------------------------------------------------------------
      Do the inverse of lock call. Enable the ISR, then release the lock.
      -------------------------------------------------------------------------*/
      if ( mISRSignal != Chimera::Interrupt::SIGNAL_INVALID )
      {
        Chimera::Interrupt::enableISR( mISRSignal );
      }

      ( *mMutex )->unlock();
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
  class StreamBuffer
  {
  public:
    StreamBuffer()
    {
    }

    ~StreamBuffer()
    {
    }

    /**
     * @brief Initializes the class
     *
     * @param attr    Attributes to configure with
     */
    void init( StreamAttr<T> &attr )
    {
    }

    /**
     * @brief Write data into the FIFO stream
     *
     * @param data      Input buffer of data to write
     * @param elements  Number of elements to write into the FIFO stream
     * @param safe      If true, protect against ISR & thread access
     * @return size_t   Number of bytes actually written
     */
    size_t write( const T *const data, const size_t elements, const bool safe = false )
    {
    }

    /**
     * @brief Read data from the FIFO stream
     *
     * @param data      Output buffer to write into
     * @param elements  Number of elements to read from the FIFO stream into the output buffer
     * @param safe      If true, protect against ISR & thread access
     * @return size_t   Number of bytes actually read
     */
    size_t read( T *const data, const size_t elements, const bool safe = false )
    {
    }

    /**
     * @brief Checks if the FIFO is empty
     *
     * @param safe      If true, protect against ISR & thread access
     * @return true   FIFO is empty
     * @return false  FIFO is not empty
     */
    bool empty( const bool safe = false )
    {
    }

    /**
     * @brief Returns the total number of bytes the FIFO may hold
     *
     * @param safe      If true, protect against ISR & thread access
     * @return size_t
     */
    size_t capacity( const bool safe = false )
    {
    }

    /**
     * @brief Returns the remaining number of bytes in the FIFO
     *
     * @param safe      If true, protect against ISR & thread access
     * @return size_t
     */
    size_t available( const bool safe = false )
    {
    }

    /**
     * @brief Returns the total used bytes in the FIFO
     *
     * @param safe      If true, protect against ISR & thread access
     * @return size_t
     */
    size_t size( const bool safe = false )
    {
    }

  private:
    StreamAttr<T> mAttr; /**< Configuration attributes */
  };
}  // namespace Aurora::Container

#endif /* !AURORA_STREAM_BUFFER_HPP */
