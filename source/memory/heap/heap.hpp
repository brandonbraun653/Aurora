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

/* Aurora Includes */
#include <Aurora/source/memory/heap/heap_intf.hpp>

namespace Aurora::Memory
{
  /*-------------------------------------------------------------------------------
  Structures
  -------------------------------------------------------------------------------*/
  struct BlockLink_t
  {
    BlockLink_t *next;
    size_t size;
  };

  /*-------------------------------------------------------------------------------
  Classes
  -------------------------------------------------------------------------------*/
  /**
   *  A heap implementation that is mostly just encapsulation of the FreeRTOS V10.0.0
   *  heap4.c allocation algorithm. The only addition is to allow the user to specify
   *  their own buffer (static or dynamic) to be used as the source memory for the heap.
   */
  class Heap : public IHeapAllocator
  {
  public:
    Heap();
    Heap( Heap &&other );
    ~Heap();

    /*-------------------------------------------------
    IHeapAllocator Interface
    -------------------------------------------------*/
    void assignMemoryPool( void *const buffer, const size_t size ) final override;
    void *malloc( const size_t size ) final override;
    void free( void *const pv ) final override;
    size_t available() const final override;
    size_t allocated() const final override;
    size_t freed() const final override;

  private:
    /*------------------------------------------------
    FreeRTOS variables for managing the heap allocations. For descriptions
    of what each actually does, please look at the heap4.c source code.
    ------------------------------------------------*/
    Chimera::Thread::Mutex* mLock;

    uint8_t *heapBuffer;
    size_t heapSize;
    BlockLink_t blockStart;
    BlockLink_t *blockEnd;
    size_t freeBytesRemaining;
    size_t minimumEverFreeBytesRemaining;
    size_t blockAllocatedBit;
    size_t blockStructSize;
    size_t minBlockSize;
    size_t bytesAllocated;
    size_t bytesFreed;

    /*------------------------------------------------
    Internal FreeRTOS functions to manage the heap
    ------------------------------------------------*/
    void initHeap();
    void prvInsertBlockIntoFreeList( BlockLink_t *pxBlockToInsert );
    size_t xPortGetFreeHeapSize();
    size_t xPortGetMinimumEverFreeHeapSize();
  };
}  // namespace Aurora::Memory

#endif /* !AURORA_MEMORY_HEAP_HPP */