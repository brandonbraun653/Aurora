/********************************************************************************
 *  File Name:
 *    heap.hpp
 *
 *  Description:
 *    Implements a dynamic memory allocation heap from a buffer
 *
 *  2019-2021 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#pragma once
#ifndef AURORA_MEMORY_HEAP_HPP
#define AURORA_MEMORY_HEAP_HPP

/* C++ Includes */
#include <cstdint>
#include <cstdlib>
#include <memory>

/* Chimera Includes */
#include <Chimera/thread>

namespace Aurora::Memory
{
  /**
   *  A heap implementation that is mostly just encapsulation of the FreeRTOS V10.0.0
   *  heap4.c allocation algorithm. The only addition is to allow the user to specify
   *  their own buffer (static or dynamic) to be used as the source memory for the heap.
   */
  class Heap
  {
  public:
    Heap();
    Heap( Heap &&other );
    ~Heap();

    /**
     *  Resets the entire heap memory to zero
     *  @return void
     */
    void staticReset();

    /**
     *  Attaches a pre-existing buffer as the source memory for the heap.
     *
     *  @warning  The lifetime of this buffer must not expire while an instance
     *            of this class exists in memory.
     *
     *  @param[in]  buffer    The memory pool to use as a heap
     *  @param[in]  size      How many bytes are in the buffer
     *  @return bool
     */
    bool assignPool( void *buffer, const size_t size );

    /**
     *  Standard malloc implementation
     *
     *  @param[in]  size      The number of bytes to be allocated
     *  @return void *
     */
    void *malloc( size_t size );

    /**
     *  Standard free implementation
     *
     *  @param[in]  pv        The memory to be freed
     *  @return void
     */
    void free( void *pv );

    /**
     *  Gets the remaining bytes available in the heap
     *  @return size_t
     */
    size_t available() const;

  private:
    /*------------------------------------------------
    FreeRTOS variables for managing the heap allocations. For descriptions
    of what each actually does, please look at the heap4.c source code.
    ------------------------------------------------*/
    struct BlockLink_t
    {
      BlockLink_t *next;
      size_t size;
    };

    std::shared_ptr<Chimera::Thread::RecursiveMutex> mLock;

    uint8_t *heapBuffer;
    size_t heapSize;
    BlockLink_t blockStart;
    BlockLink_t *blockEnd;
    size_t freeBytesRemaining;
    size_t minimumEverFreeBytesRemaining;
    size_t blockAllocatedBit;
    size_t blockStructSize;
    size_t minBlockSize;

    /*------------------------------------------------
    Internal FreeRTOS functions to manage the heap
    ------------------------------------------------*/
    void init();
    void prvInsertBlockIntoFreeList( BlockLink_t *pxBlockToInsert );
    size_t xPortGetFreeHeapSize();
    size_t xPortGetMinimumEverFreeHeapSize();
  };
}  // namespace Aurora::Memory

#endif /* !AURORA_MEMORY_HEAP_HPP */