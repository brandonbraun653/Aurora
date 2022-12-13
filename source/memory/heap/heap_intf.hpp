/******************************************************************************
 *  File Name:
 *    heap_intf.hpp
 *
 *  Description:
 *    Interface class for various heap controllers
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *****************************************************************************/

#pragma once
#ifndef RIPPLE_NET_MANAGER_HPP
#define RIPPLE_NET_MANAGER_HPP

/* STL Includes */
#include <cstddef>

/* Chimera Includes */
#include <Chimera/thread>

namespace Aurora::Memory
{
  /*---------------------------------------------------------------------------
  Classes
  ---------------------------------------------------------------------------*/
  /**
   * @brief Memory allocator interface that is based on some heap algorithm
   *
   * High level API to describe memory allocators that operate on some sort
   * of pool. Where this pool comes from doesn't matter, but the idea is to
   * allow statically allocated memory be used as a heap to dynamically create
   * and destroy objects at runtime without fear of corrupting the global heap.
   */
  class IHeapAllocator : public Chimera::Thread::Lockable<IHeapAllocator>
  {
  public:
    /**
     * @brief Attaches a pre-existing buffer as the source memory for the heap.
     *
     * @warning  The lifetime of this buffer must not expire while an instance
     *           of this class exists in memory.
     *
     * @param buffer The memory pool to use as a heap
     * @param size   How many bytes are in the buffer
     * @return true
     * @return false
     */
    virtual void assignMemoryPool( void *const buffer, const size_t size ) = 0;

    /**
     * @brief Allocates memory from the heap
     *
     * @param[in]  size      The number of bytes to be allocated
     * @return void *
     */
    virtual void *malloc( const size_t size ) = 0;

    /**
     * @brief Frees previously allocated memory
     *
     * @param[in]  pv        The memory to be freed
     * @return void
     */
    virtual void free( void *const pv ) = 0;

    /**
     * @brief Gets the remaining bytes available in the heap
     *
     * @return size_t
     */
    virtual size_t available() const = 0;

    /**
     * @brief Returns the total number of bytes allocated over all time
     *
     * @return size_t
     */
    virtual size_t allocated() const = 0;

    /**
     * @brief Returns the total number of bytes freed over all time
     *
     * @return size_t
     */
    virtual size_t freed() const = 0;

  private:
    friend Chimera::Thread::Lockable<IHeapAllocator>;
  };

}  // namespace Ripple

#endif  /* !RIPPLE_NET_MANAGER_HPP */
