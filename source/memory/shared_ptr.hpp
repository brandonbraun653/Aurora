/******************************************************************************
 *  File Name:
 *    shared_ptr.hpp
 *
 *  Description:
 *    Custom implementation of std::shared_ptr that focuses on memory allocated
 *    with a custom Heap allocator. This makes shared_ptrs a little more friendly
 *    to embedded systems with limited resources.
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *****************************************************************************/

#pragma once
#ifndef RIPPLE_UTIL_MEMORY_HPP
#define RIPPLE_UTIL_MEMORY_HPP

/* STL Includes */
#include <type_traits>

/* Chimera Includes */
#include <Chimera/assert>
#include <Chimera/thread>

/* Aurora Includes */
#include <Aurora/source/memory/heap/heap_intf.hpp>

namespace Aurora::Memory
{
  /*---------------------------------------------------------------------------
  Classes
  ---------------------------------------------------------------------------*/
  /**
   * @brief Reference counted thread safe pointer allocated from a fixed memory pool
   *
   * Emulates the functionality of std::shared_ptr but with a more embedded focus.
   * Memory for the object is allocated from a managed pool rather than the global
   * heap. This helps in embedded systems, which can allocate a dedicate buffer of
   * memory for a resource.
   *
   * @tparam T          Object type being managed
   * @tparam BUFSIZE    Allocates buffer of this size, assuming T is a pointer
   */
  template<typename T>
  class shared_ptr
  {
  public:
    /**
     * @brief Default construct a new Ref Ptr object
     */
    explicit shared_ptr() : mBufferSize( 0 ), mAllocator( nullptr ), mObjCount( nullptr ), mObjPtr( nullptr ), mLock( nullptr )
    {
    }

    explicit shared_ptr( IHeapAllocator *allocator ) : shared_ptr( allocator, 0 )
    {
    }

    /**
     * @brief Construct a new Ref Ptr object
     * Allocates enough memory for this object type + whatever data is being
     * held. Typically used for arrays, but can hold more complex types too.
     *
     * @param allocator   Memory allocator to allocate from
     */
    explicit shared_ptr( IHeapAllocator *allocator, const size_t size ) :
        mBufferSize( size ), mAllocator( allocator ), mObjCount( nullptr ), mObjPtr( nullptr ), mLock( nullptr )
    {
      Chimera::Thread::LockGuard lck( *allocator );

      /*-------------------------------------------------
      Check to ensure memory requirements are met
      -------------------------------------------------*/
      if ( allocator->available() < this->size() )
      {
        mObjPtr   = nullptr;
        mObjCount = nullptr;
        return;
      }

      /*-------------------------------------------------
      Allocate memory from the network allocator
      -------------------------------------------------*/
      uint8_t *pool                = reinterpret_cast<uint8_t *>( allocator->malloc( this->size() ) );
      const auto expected_end_addr = reinterpret_cast<std::uintptr_t>( pool ) + this->size();

      /*-------------------------------------------------
      Construct the reference counter. Order important.
      See do_cleanup() for details.
      -------------------------------------------------*/
      mObjCount = new ( pool ) CountType( 1 );
      pool += sizeof( CountType );

      mLock = new ( pool ) Chimera::Thread::Mutex();
      pool += sizeof( Chimera::Thread::Mutex );

      /*-------------------------------------------------
      Construct the underlying object
      -------------------------------------------------*/
      mObjPtr = new ( pool ) T();
      pool += sizeof( T );

      /*-------------------------------------------------
      If additional size was specified and the underlying
      type is a pointer, assume that pointer will hold
      the remaining bytes.
      -------------------------------------------------*/
      if constexpr ( std::is_pointer<T>::value )
      {
        *mObjPtr = pool;
        memset( *mObjPtr, 0xCC, mBufferSize );
        pool += mBufferSize;
      }

      /*-------------------------------------------------
      Make it abundantly clear at runtime if some kind of
      allocation error occurred.
      -------------------------------------------------*/
      RT_HARD_ASSERT( expected_end_addr == reinterpret_cast<std::uintptr_t>( pool ) );
    }

    /**
     * @brief Copy construct a new Ref Ptr object
     *
     * @param obj     Object being copied
     */
    shared_ptr( const shared_ptr<T> &obj )
    {
      this->mAllocator = obj.mAllocator;
      this->mObjPtr    = obj.mObjPtr;
      this->mObjCount  = obj.mObjCount;
      this->mLock      = obj.mLock;

      if ( obj.mObjPtr && obj.mObjCount )
      {
        RT_HARD_ASSERT( this->mLock );
        Chimera::Thread::LockGuard lck( *this->mLock );

        ( *this->mObjCount )++;
      }
    }

    /**
     * @brief Move construct a new Ref Ptr object
     *
     * @note This must be marked explicit. I don't know if this is due to
     * poor design of this class or if that's normal, but a networking lib
     * I use will crash without having a proper move constructor.
     *
     * @param obj     Dying object being moved
     */
    explicit shared_ptr( shared_ptr<T> &&obj )
    {
      /*-------------------------------------------------
      Clean up existing data
      -------------------------------------------------*/
      do_cleanup();

      this->mAllocator = obj.mAllocator;
      this->mObjPtr    = obj.mObjPtr;
      this->mObjCount  = obj.mObjCount;
      this->mLock      = obj.mLock;

      obj.mObjPtr    = nullptr;
      obj.mObjCount  = nullptr;
      obj.mAllocator = nullptr;
      obj.mLock      = nullptr;
    }

    /**
     * @brief Destroy the Ref Ptr object
     */
    ~shared_ptr()
    {
      this->do_cleanup();
    }

    /**
     * @brief Copy assignment operator
     *
     * @param obj   Object being copied
     * @return T&
     */
    shared_ptr<T> &operator=( const shared_ptr<T> &obj )
    {
      /*-------------------------------------------------
      Clean up existing data
      -------------------------------------------------*/
      do_cleanup();

      /*-------------------------------------------------
      Copy in the new data
      -------------------------------------------------*/
      this->mAllocator = obj.mAllocator;
      this->mObjPtr    = obj.mObjPtr;
      this->mObjCount  = obj.mObjCount;
      this->mLock      = obj.mLock;

      if ( obj.mObjPtr && obj.mObjCount )
      {
        RT_HARD_ASSERT( this->mLock );
        Chimera::Thread::LockGuard lck( *this->mLock );
        ( *this->mObjCount )++;
      }

      return *this;
    }

    /**
     * @brief Move assignment operator
     *
     * @param obj     Object being moved
     * @return T&
     */
    shared_ptr<T> &operator=( shared_ptr<T> &&obj )
    {
      /*-------------------------------------------------
      Clean up existing data
      -------------------------------------------------*/
      do_cleanup();

      /*-------------------------------------------------
      Copy in the new data
      -------------------------------------------------*/
      this->mAllocator = obj.mAllocator;
      this->mObjPtr    = obj.mObjPtr;
      this->mObjCount  = obj.mObjCount;
      this->mLock      = obj.mLock;

      obj.mObjPtr    = nullptr;
      obj.mObjCount  = nullptr;
      obj.mAllocator = nullptr;
      obj.mLock      = nullptr;

      return *this;
    }

    /**
     * @brief Pointer access overload
     *
     * @return T*
     */
    T *operator->() const
    {
      return this->mObjPtr;
    }

    /**
     * @brief Reference access overload
     *
     * @return T&
     */
    T &operator*() const
    {
      return *( this->mObjPtr );
    }

    /**
     * @brief Evaluates if this object is valid
     *
     * @return true
     * @return false
     */
    explicit operator bool() const
    {
      return mAllocator && mObjCount && mObjPtr && mLock;
    }

    /**
     * @brief Returns pointer to underlying data
     *
     * @return T*
     */
    T *get() const
    {
      if( static_cast<bool>( *this ) )
      {
        return this->mObjPtr;
      }
      else
      {
        return nullptr;
      }
    }

    /**
     * @brief Gets the number of valid references to the object
     *
     * @return size_t
     */
    size_t references() const
    {
      if( static_cast<bool>( *this ) )
      {
        Chimera::Thread::LockGuard lck( *this->mLock );
        return *mObjCount;
      }
      else
      {
        return 0;
      }
    }

    /**
     * @brief Returns the total size of the object and managed data
     *
     * @return constexpr size_t
     */
    size_t size() const
    {
      return sizeof( T ) + sizeof( CountType ) + sizeof( Chimera::Thread::Mutex ) + mBufferSize;
    }

  protected:
    using CountType = size_t;

  private:
    size_t mBufferSize;
    IHeapAllocator *mAllocator;
    CountType *mObjCount;
    T *mObjPtr;
    Chimera::Thread::Mutex *mLock;

    /**
     * @brief Handle reference counting on destruction
     * Can optionally release memory.
     */
    void do_cleanup()
    {
      /*-------------------------------------------------
      No references?
      -------------------------------------------------*/
      if ( !mObjCount || ( *mObjCount == 0 ) )
      {
        return;
      }

      /*-------------------------------------------------
      Decrement the reference count
      -------------------------------------------------*/
      RT_HARD_ASSERT( mLock );
      Chimera::Thread::LockGuard lck( *this->mLock );

      ( *mObjCount )--;
      if ( *mObjCount != 0 )
      {
        return;
      }

      /* By this point, all references need to be valid */
      RT_HARD_ASSERT( mObjCount && mObjPtr && mAllocator && mLock );

      /*-------------------------------------------------
      If the type isn't a pointer, there is a non-trivial
      destructor that needs calling. Type was constructed
      with placement new, which won't automatically call
      the destructor on free.
      -------------------------------------------------*/
      if constexpr ( !std::is_pointer<T>::value )
      {
        mObjPtr->~T();
      }

      mLock->~Mutex();

      /*-------------------------------------------------
      The entire packet was allocated in a single block,
      with mObjCount being the first item. This points to
      the start of the block and thus is the only item
      that needs freeing. All other pointers from this
      class are assigned or allocated with placement new.
      -------------------------------------------------*/
      mAllocator->free( mObjCount );

      /*-------------------------------------------------
      All memory is released. Invalidate the pointers.
      -------------------------------------------------*/
      mObjCount  = nullptr;
      mObjPtr    = nullptr;
      mAllocator = nullptr;
    }
  };

}  // namespace Aurora::Memory

#endif /* !RIPPLE_UTIL_MEMORY_HPP */
