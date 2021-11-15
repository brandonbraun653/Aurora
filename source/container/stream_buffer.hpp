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


namespace Aurora::Container
{
  /*---------------------------------------------------------------------------
  Forward Declarations
  ---------------------------------------------------------------------------*/
  class StreamBuffer;


  /*---------------------------------------------------------------------------
  Classes
  ---------------------------------------------------------------------------*/
  /**
   * @brief Helper class for managing the lifetime and access permissions of
   * the underlying memory of a StreamBuffer class.
   */
  class StreamAttr
  {
  public:
    StreamAttr();
    ~StreamAttr();

    /**
     * @brief Dynamically allocate stream memory
     *
     * @param size    Number of bytes to allocate
     * @return bool
     */
    bool init( const size_t size );

    /**
     * @brief Statically initialize stream memory
     *
     * @param queue   Circular buffer to use
     * @param mtx     Thread lock to use
     * @param signal  ISR signal to protect against
     * @return bool
     */
    bool init( CircularBuffer<uint8_t> *const queue, Chimera::Thread::RecursiveMutex *const mtx,
               const Chimera::Interrupt::Signal_t signal );

    /**
     * @brief De-initializes the stream attributes
     */
    void destroy();

  protected:
    friend class StreamBuffer;

    CircularBuffer<uint8_t> **        mQueue;     /**< Memory manager */
    Chimera::Thread::RecursiveMutex **mMutex;     /**< Thread safety lock */
    Chimera::Interrupt::Signal_t      mISRSignal; /**< Which ISR signal to protect against, if any */

    /**
     * @brief Protects memory access from multithreaded and ISR access
     *
     * @return true   Memory was locked
     * @return false  Memory was not locked
     */
    bool lock();

    /**
     * @brief Unlocks previous lock call
     */
    void unlock();

  private:
    bool mDynamicMem; /**< Memory was dynamically allocated */
  };


  class StreamBuffer
  {
  public:
    StreamBuffer();
    ~StreamBuffer();

    /**
     * @brief Initializes the class
     *
     * @param attr    Attributes to configure with
     * @return true   Successfully initialized
     * @return false  Failed to initialize
     */
    bool init( StreamAttr &attr );

    /**
     * @brief Write data into the FIFO stream
     *
     * @param data      Input buffer of data to write
     * @param size      Bytes to write into the FIFO stream
     * @param safe      If true, protect against ISR & thread access
     * @return size_t   Number of bytes actually written
     */
    size_t write( const void *const data, const size_t size, const bool safe = false );

    /**
     * @brief Read data from the FIFO stream
     *
     * @param data      Output buffer to write into
     * @param size      Number of bytes to read from the FIFO stream into the output buffer
     * @param safe      If true, protect against ISR & thread access
     * @return size_t   Number of bytes actually read
     */
    size_t read( void *const data, const size_t size, const bool safe = false );

    /**
     * @brief Checks if the FIFO is empty
     *
     * @return true   FIFO is empty
     * @return false  FIFO is not empty
     */
    bool empty();

    /**
     * @brief Returns the total number of bytes the FIFO may hold
     * @return size_t
     */
    size_t capacity();

    /**
     * @brief Returns the remaining number of bytes in the FIFO
     * @return size_t
     */
    size_t available();

    /**
     * @brief Returns the total used bytes in the FIFO
     * @return size_t
     */
    size_t size();

  protected:
    bool assign_memory();

  private:
    StreamAttr mAttr; /**< Configuration attributes */
  };
}  // namespace Aurora::Container

#endif /* !AURORA_STREAM_BUFFER_HPP */
