/********************************************************************************
 *  File Name:
 *    datastore_driver.hpp
 *
 *  Description:
 *    Driver for the datastore
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_DATASTORE_DRIVER_HPP
#define AURORA_DATASTORE_DRIVER_HPP

/* STL Includes */
#include <cstdint>
#include <cstddef>

/* Aurora Includes */
#include <Aurora/source/datastore/datastore_types.hpp>

/* Chimera Includes */
#include <Chimera/callback>
#include <Chimera/thread>

namespace Aurora::Datastore
{
  /*-------------------------------------------------------------------------------
  Classes
  -------------------------------------------------------------------------------*/
  class Manager : public Chimera::Thread::Lockable<Manager>, public Chimera::Callback::DelegateService<Manager, CallbackId>
  {
  public:
    Manager();
    ~Manager();

    /**
     *  Assigns the manager's working memory
     *
     *  @param[in]  map           Statically allocated observable map
     *  @return void
     */
    void assignCoreMemory( ObservableMap *const map );

    /**
     *  Clears the observable registry
     *  @return void
     */
    void resetRegistry();

    /**
     *  Registers a new observable object with the manager
     *
     *  @param[in]  observable    The observable data
     *  @return bool
     */
    bool registerObservable( IObservableAttr &observable );

    /**
     *  Processes the observables to update their data. Must be called
     *  periodically so as to not stall any measurements.
     *
     *  @return void
     */
    void process();

    /**
     *  Reads the data associated with a key into a data buffer
     *
     *  @note The buffer size must exactly equal the size registered with the key
     *
     *  @param[in]  key           The key to look up
     *  @param[in]  data          Output buffer to write into
     *  @param[in]  size          Size of the output buffer
     *  @return bool              Validity of the data
     */
    bool readDataSafe( const Database::Key key, void *const data, const size_t size );

    /**
     *  Manually requests an observable to perform an update. Typically used
     *  in conjunction with readDataSafe() in order to get the latest data.
     *
     *  @param[in]  key           The key to request against
     *  @return bool
     */
    bool requestUpdate( const Database::Key key );

  private:
    friend Chimera::Thread::Lockable<Manager>;
    friend Chimera::Callback::DelegateService<Manager, CallbackId>;

    ObservableMap *mObservableMap;
  };
}  // namespace Aurora::Datastore

#endif  /* !AURORA_DATASTORE_DRIVER_HPP */
