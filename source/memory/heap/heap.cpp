/********************************************************************************
 *  File Name:
 *    heap.cpp
 *
 *  Description:
 *    Implements the managed heap allocator
 *
 *  Notes:
 *    This is a shameless wrapper around the FreeRTOS V10.0 heap4.c memory
 *    allocator
 *
 *  2019-2021 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

/* C++ Includes */
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <string.h>

/* Aurora Includes */
#include <Aurora/logging>
#include <Aurora/memory>

/* Chimera Includes */
#include <Chimera/assert>
#include <Chimera/thread>

/* FreeRTOS Includes */
#if defined( USING_FREERTOS_THREADS )
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#else  // Assume PC or other non-embedded system
#define portBYTE_ALIGNMENT 4

#if portBYTE_ALIGNMENT == 32
#define portBYTE_ALIGNMENT_MASK ( 0x001f )
#endif

#if portBYTE_ALIGNMENT == 16
#define portBYTE_ALIGNMENT_MASK ( 0x000f )
#endif

#if portBYTE_ALIGNMENT == 8
#define portBYTE_ALIGNMENT_MASK ( 0x0007 )
#endif

#if portBYTE_ALIGNMENT == 4
#define portBYTE_ALIGNMENT_MASK ( 0x0003 )
#endif

#if portBYTE_ALIGNMENT == 2
#define portBYTE_ALIGNMENT_MASK ( 0x0001 )
#endif

#if portBYTE_ALIGNMENT == 1
#define portBYTE_ALIGNMENT_MASK ( 0x0000 )
#endif
#endif /* USING_FREERTOS_THREADS */


#define DEBUG_MODULE    ( false )

namespace Aurora::Memory
{
  /*-------------------------------------------------------------------------------
  Constants
  -------------------------------------------------------------------------------*/
  static constexpr size_t heapBITS_PER_BYTE = 8u;

  /*-------------------------------------------------------------------------------
  Heap Implementation
  -------------------------------------------------------------------------------*/
  Heap::Heap()
  {
    mLock                         = std::make_shared<Chimera::Thread::Mutex>();
    heapBuffer                    = nullptr;
    heapSize                      = 0;
    blockStructSize               = ( sizeof( BlockLink_t ) + ( portBYTE_ALIGNMENT - 1 ) ) & ~( portBYTE_ALIGNMENT_MASK );
    minBlockSize                  = blockStructSize << 1;
    freeBytesRemaining            = 0;
    minimumEverFreeBytesRemaining = 0;
    blockAllocatedBit             = 0;
    bytesAllocated                = 0;
    bytesFreed                    = 0;

    /*------------------------------------------------
    Initialize the start/end list pointers
    ------------------------------------------------*/
    blockEnd = nullptr;
    memset( &blockStart, 0, sizeof( BlockLink_t ) );
  }


  Heap::Heap( Heap &&other ) : mLock( other.mLock )
  {
    heapBuffer                    = other.heapBuffer;
    heapSize                      = other.heapSize;
    blockStart                    = other.blockStart;
    blockEnd                      = other.blockEnd;
    freeBytesRemaining            = other.freeBytesRemaining;
    minimumEverFreeBytesRemaining = other.minimumEverFreeBytesRemaining;
    blockAllocatedBit             = other.blockAllocatedBit;
    blockStructSize               = other.blockStructSize;
    minBlockSize                  = other.minBlockSize;
    bytesAllocated                = other.bytesAllocated;
    bytesFreed                    = other.bytesFreed;
  }


  Heap::~Heap()
  {
  }


  /*-------------------------------------------------------------------------------
  Heap: Public Functions
  -------------------------------------------------------------------------------*/
  void Heap::staticReset()
  {
    using namespace Chimera::Thread;
    LockGuard lck( *mLock.get() );

    /*-------------------------------------------------
    Runtime protection
    -------------------------------------------------*/
    if ( !heapBuffer )
    {
      return;
    }

    /*-------------------------------------------------
    Reset the buffer
    -------------------------------------------------*/
    memset( heapBuffer, 0, heapSize );
  }


  bool Heap::assignPool( void *buffer, const size_t size )
  {
    using namespace Chimera::Thread;

    /*-------------------------------------------------
    Input Protection
    -------------------------------------------------*/
    if ( !buffer || !size )
    {
      return false;
    }

    /*-------------------------------------------------
    Attach the buffer
    -------------------------------------------------*/
    LockGuard lck( *mLock.get() );

    heapBuffer     = reinterpret_cast<uint8_t *>( buffer );
    heapSize       = size;
    bytesAllocated = 0;
    bytesFreed     = 0;

    return true;
  }


  void *Heap::malloc( size_t size )
  {
    using namespace Chimera::Thread;
    LockGuard lck( *mLock.get() );

    BlockLink_t *pxBlock;
    BlockLink_t *pxPreviousBlock;
    BlockLink_t *pxNewBlockLink;
    void *pvReturn = nullptr;

    /* If this is the first call to malloc then the heap will require
    initialization to setup the list of free blocks. */
    if ( blockEnd == nullptr )
    {
      init();
    }

    /* Check the requested block size is not so large that the top bit is
    set.  The top bit of the block size member of the BlockLink_t structure
    is used to determine who owns the block - the application or the
    kernel, so it must be free. */
    if ( ( size & blockAllocatedBit ) == 0 )
    {
      /* The wanted size is increased so it can contain a BlockLink_t
      structure in addition to the requested amount of bytes. */
      if ( size > 0 )
      {
        size += blockStructSize;

        /* Ensure that blocks are always aligned to the required number
        of bytes. */
        if ( ( size & portBYTE_ALIGNMENT_MASK ) != 0x00 )
        {
          /* Byte alignment required. */
          size += ( portBYTE_ALIGNMENT - ( size & portBYTE_ALIGNMENT_MASK ) );
          RT_HARD_ASSERT( ( size & portBYTE_ALIGNMENT_MASK ) == 0 );
        }
      }

      if ( ( size > 0 ) && ( size <= freeBytesRemaining ) )
      {
        /* Traverse the list from the start	(lowest address) block until
        one	of adequate size is found. */
        pxPreviousBlock = &blockStart;
        pxBlock         = blockStart.next;
        while ( ( pxBlock->size < size ) && ( pxBlock->next != nullptr ) )
        {
          pxPreviousBlock = pxBlock;
          pxBlock         = pxBlock->next;
        }

        /* If the end marker was reached then a block of adequate size
        was	not found. */
        if ( pxBlock != blockEnd )
        {
          /* Return the memory space pointed to - jumping over the
          BlockLink_t structure at its start. */
          pvReturn = ( void * )( ( ( uint8_t * )pxPreviousBlock->next ) + blockStructSize );

          /* This block is being returned for use so must be taken out
          of the list of free blocks. */
          pxPreviousBlock->next = pxBlock->next;

          /* If the block is larger than required it can be split into
          two. */
          if ( ( pxBlock->size - size ) > minBlockSize )
          {
            /* This block is to be split into two.  Create a new
            block following the number of bytes requested. The void
            cast is used to prevent byte alignment warnings from the
            compiler. */
            pxNewBlockLink = reinterpret_cast<BlockLink_t *>( ( ( uint8_t * )pxBlock ) + size );
            RT_HARD_ASSERT( ( ( ( size_t )pxNewBlockLink ) & portBYTE_ALIGNMENT_MASK ) == 0 );

            /* Calculate the sizes of two blocks split from the
            single block. */
            pxNewBlockLink->size = pxBlock->size - size;
            pxBlock->size        = size;

            /* Insert the new block into the list of free blocks. */
            prvInsertBlockIntoFreeList( pxNewBlockLink );
          }

          freeBytesRemaining -= pxBlock->size;

          /* Track bytes allocated */
          bytesAllocated += size;
          LOG_IF_DEBUG( DEBUG_MODULE, "Alloc %d bytes at address 0x%.8X\r\n", pxBlock->size, reinterpret_cast<std::uintptr_t>( pxBlock ) );

          if ( freeBytesRemaining < minimumEverFreeBytesRemaining )
          {
            minimumEverFreeBytesRemaining = freeBytesRemaining;
          }

          /* The block is being returned - it is allocated and owned
          by the application and has no "next" block. */
          pxBlock->size |= blockAllocatedBit;
          pxBlock->next = nullptr;
        }
      }
    }

    RT_HARD_ASSERT( ( ( ( size_t )pvReturn ) & ( size_t )portBYTE_ALIGNMENT_MASK ) == 0 );
    return pvReturn;
  }


  void Heap::free( void *pv )
  {
    using namespace Chimera::Thread;
    LockGuard lck( *mLock.get() );

    uint8_t *puc = ( uint8_t * )pv;
    BlockLink_t *pxLink;

    if ( pv != nullptr )
    {
      /* The memory being freed will have an BlockLink_t structure immediately before it. */
      puc -= blockStructSize;

      /* This casting is to keep the compiler from issuing warnings. */
      pxLink = reinterpret_cast<BlockLink_t *>( puc );

      /* Check the block is actually allocated. */
      RT_HARD_ASSERT( ( pxLink->size & blockAllocatedBit ) != 0 );
      RT_HARD_ASSERT( pxLink->next == nullptr );

      if ( ( pxLink->size & blockAllocatedBit ) != 0 )
      {
        if ( pxLink->next == nullptr )
        {
          /* The block is being returned to the heap - it is no longer allocated. */
          pxLink->size &= ~blockAllocatedBit;

          /* Track allocated bytes */
          bytesFreed += pxLink->size;
          LOG_IF_DEBUG( DEBUG_MODULE, "Freed %d bytes at address 0x%.8X\r\n", pxLink->size, reinterpret_cast<std::uintptr_t>( pxLink ) );

          /* Add this block to the list of free blocks. */
          freeBytesRemaining += pxLink->size;
          prvInsertBlockIntoFreeList( pxLink );
        }
      }
    }
  }


  size_t Heap::available() const
  {
    using namespace Chimera::Thread;
    LockGuard lck( *mLock.get() );

    return freeBytesRemaining;
  }


  /*-------------------------------------------------------------------------------
  Heap: Private Functions
  -------------------------------------------------------------------------------*/
  size_t Heap::xPortGetFreeHeapSize( void )
  {
    return freeBytesRemaining;
  }


  size_t Heap::xPortGetMinimumEverFreeHeapSize( void )
  {
    return minimumEverFreeBytesRemaining;
  }


  void Heap::init()
  {
    using namespace Chimera::Thread;

    BlockLink_t *pxFirstFreeBlock;
    uint8_t *pucAlignedHeap;
    size_t uxAddress;
    size_t xTotalHeapSize = heapSize;

    /* Ensure the heap starts on a correctly aligned boundary. */
    uxAddress = reinterpret_cast<size_t>( heapBuffer );

    if ( ( uxAddress & portBYTE_ALIGNMENT_MASK ) != 0 )
    {
      uxAddress += ( portBYTE_ALIGNMENT - 1 );
      uxAddress &= ~( ( size_t )portBYTE_ALIGNMENT_MASK );
      xTotalHeapSize -= uxAddress - ( size_t )heapBuffer;
    }

    pucAlignedHeap = ( uint8_t * )uxAddress;

    /* xStart is used to hold a pointer to the first item in the list of free
    blocks.  The void cast is used to prevent compiler warnings. */
    blockStart.next = reinterpret_cast<BlockLink_t *>( pucAlignedHeap );
    blockStart.size = ( size_t )0;

    /* pxEnd is used to mark the end of the list of free blocks and is inserted
    at the end of the heap space. */
    uxAddress = ( ( size_t )pucAlignedHeap ) + xTotalHeapSize;
    uxAddress -= blockStructSize;
    uxAddress &= ~( ( size_t )portBYTE_ALIGNMENT_MASK );
    blockEnd       = reinterpret_cast<BlockLink_t *>( uxAddress );
    blockEnd->size = 0;
    blockEnd->next = nullptr;

    /* To start with there is a single free block that is sized to take up the
    entire heap space, minus the space taken by pxEnd. */
    pxFirstFreeBlock       = reinterpret_cast<BlockLink_t *>( pucAlignedHeap );
    pxFirstFreeBlock->size = uxAddress - ( size_t )pxFirstFreeBlock;
    pxFirstFreeBlock->next = blockEnd;

    /* Only one block exists - and it covers the entire usable heap space. */
    minimumEverFreeBytesRemaining = pxFirstFreeBlock->size;
    freeBytesRemaining            = pxFirstFreeBlock->size;

    /* Work out the position of the top bit in a size_t variable. */
    blockAllocatedBit = ( ( size_t )1 ) << ( ( sizeof( size_t ) * heapBITS_PER_BYTE ) - 1 );
  }


  void Heap::prvInsertBlockIntoFreeList( BlockLink_t *pxBlockToInsert )
  {
    using namespace Chimera::Thread;

    BlockLink_t *pxIterator;
    uint8_t *puc;

    /* Iterate through the list until a block is found that has a higher address
    than the block being inserted. */
    for ( pxIterator = &blockStart; pxIterator->next < pxBlockToInsert; pxIterator = pxIterator->next )
    {
      /* Nothing to do here, just iterate to the right position. */
    }

    /* Do the block being inserted, and the block it is being inserted after
    make a contiguous block of memory? */
    puc = ( uint8_t * )pxIterator;
    if ( ( puc + pxIterator->size ) == ( uint8_t * )pxBlockToInsert )
    {
      pxIterator->size += pxBlockToInsert->size;
      pxBlockToInsert = pxIterator;
    }

    /* Do the block being inserted, and the block it is being inserted before
    make a contiguous block of memory? */
    puc = ( uint8_t * )pxBlockToInsert;
    if ( ( puc + pxBlockToInsert->size ) == ( uint8_t * )pxIterator->next )
    {
      if ( pxIterator->next != blockEnd )
      {
        /* Form one big block from the two blocks. */
        pxBlockToInsert->size += pxIterator->next->size;
        pxBlockToInsert->next = pxIterator->next->next;
      }
      else
      {
        pxBlockToInsert->next = blockEnd;
      }
    }
    else
    {
      pxBlockToInsert->next = pxIterator->next;
    }

    /* If the block being inserted plugged a gab, so was merged with the block
    before and the block after, then it's pxNextFreeBlock pointer will have
    already been set, and should not be set here as that would make it point
    to itself. */
    if ( pxIterator != pxBlockToInsert )
    {
      pxIterator->next = pxBlockToInsert;
    }
  }
}  // namespace Aurora::Memory
