/********************************************************************************
 *  File Name:
 *    observable.hpp
 *
 *  Description:
 *    Interface to an observable object
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_STRUCTURE_OBSERVABLE_HPP
#define AURORA_STRUCTURE_OBSERVABLE_HPP

/* STL Includes */
#include <cstddef>

/* Chimera Includes */
#include <Chimera/common>
#include <Chimera/thread>

/* Observer Includes */
#include <Aurora/structure/obs/observer_intf.hpp>
#include <Aurora/structure/obs/types.hpp>

namespace Aurora::Structure::Observer
{
  /**
   *  A class that is capable of being observed. Typically called the "Subject" in
   *  most online guides. This particular implementation has been modified slightly
   *  to allow for more practical use in embedded systems, including:
   *    1. Static memory allocation for Observable registration list
   *    2. Multi-thread awareness
   *    3. Generic data types for the update procedure (avoids templates)
   */
  class Observable
  {
  public:
    Observable( ControlBlock *cb );
    Observable();
    ~Observable();

    /**
     *  Initializes the class to default values. Registered observables will be removed.
     *
     *  @return bool
     */
    bool initialize();

    /**
     *  Registers the control block with the class
     *
     *  @param[in]  cb          The control block being registered
     *  @param[in]  timeout     How long to wait for access to the observable (ms)
     *  @return Chimera::Status_t
     */
    Chimera::Status_t registerControlBlock( ControlBlock *cb, const size_t timeout );

    /**
     *  Attaches a new observer to the list
     *
     *  @param[in]  observer    The observer to be added
     *  @param[in]  timeout     How long to wait for access to the observable (ms)
     *  @return Chimera::Status_t
     */
    Chimera::Status_t attach( IObserver *const observer, const size_t timeout );

    /**
     *  Detaches an observer from the list
     *
     *  @param[in]  observer    The observer to be removed
     *  @param[in]  timeout     How long to wait for access to the observable (ms)
     *  @return Chimera::Status_t
     */
    Chimera::Status_t detach( IObserver *const observer, const size_t timeout );

    /**
     *  Notifies every observer of the changes. Capable of MIMO utilization
     *  due to this function's access being thread-safe.
     *
     *  @param[in]  event       The event data that just occurred
     *  @param[in]  timeout     How long to wait for access to the observable (ms)
     *  @return Chimera::Status_t
     */
    Chimera::Status_t update( UpdateArgs *event, const size_t timeout );

    /**
     *  Sets a numeric identifier that determines the kind of data
     *  associated with this observable.
     *
     *  @note Intentionally using a POD so projects can define their own
     *        enumerated values for complex types that are being observed.
     *
     *  @return void
     */
    void setType( const size_t type );

    /**
     *  Gets the data type associated with the observable.
     *
     *  @return size_t
     */
    size_t getType();

  private:
    ControlBlock *mControlBlock;
    size_t mUserType;
    size_t mNextEmptySlot;
    size_t mRegisteredObservers;

    Chimera::Threading::RecursiveTimedMutex mLock;
  };
}  // namespace Aurora::Structure

#endif  /* !AURORA_STRUCTURE_OBSERVABLE_HPP */
