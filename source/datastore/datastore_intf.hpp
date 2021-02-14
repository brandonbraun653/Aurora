/********************************************************************************
 *  File Name:
 *    datastore_intf.hpp
 *
 *  Description:
 *    Virtual interfaces for the datastore
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_DATASTORE_INTF_HPP
#define AURORA_DATASTORE_INTF_HPP

/* STL Includes */
#include <cstdint>
#include <cstddef>
#include <limits>

/* ETL Includes */
#include <etl/observer.h>

/* Aurora Includes */
#include <Aurora/source/database/database_intf.hpp>

namespace Aurora::Datastore
{
  /*-------------------------------------------------------------------------------
  Forward Declarations
  -------------------------------------------------------------------------------*/
  class Manager;

  /*-------------------------------------------------------------------------------
  Classes
  -------------------------------------------------------------------------------*/
  /**
   *  Defines an interface to access various properties of a generic
   *  observable data type. This is mostly for the manager class to have a
   *  common way of interfacing with data.
   */
  class IObservableAttr
  {
  public:
    virtual ~IObservableAttr() = default;

    /**
     *  Initialize the data to default values
     *  @return void
     */
    virtual void initialize()
    {
    }

    /**
     *  Performs the update procedure to refresh the data
     *  @return void
     */
    virtual void update()
    {
    }

    /**
     *  Performs timeout procedures
     *  @return void
     */
    virtual void onTimeout()
    {
    }

    /**
     *  Checks if the currently stored data is valid
     *  @return bool
     */
    virtual bool valid() const
    {
      return false;
    }

    /**
     *  Checks if the supplied data is valid
     *
     *  @param[in]  data      Data to be validated
     *  @param[in]  size      Number of bytes in data buffer
     *  @return bool          Validity of the data
     */
    virtual bool validate( const void *const data, const size_t size ) const
    {
      return false;
    };

    /**
     *  Reads out the latest data from the observable
     *
     *  @param[in]  data      Data to read into, sized for observable type
     *  @param[in]  size      Size of the data buffer
     *  @return bool          Validity of the data
     */
    virtual bool read( void *const data, const size_t size ) const
    {
      return false;
    }

    /**
     *  Checks if the currently stored data is stale
     *  @return bool
     */
    virtual bool isExpired() const
    {
      return true;
    }

    /**
     *  Gets the timeout value for the data
     *  @return size_t
     */
    virtual size_t timeout() const
    {
      return 0;
    }

    /**
     *  Gets the timeout value for the data
     *  @return size_t
     */
    virtual size_t updateRate() const
    {
      return 0;
    }

    /**
     *  Gets the timeout value for the data
     *  @return size_t
     */
    virtual size_t size() const
    {
      return 0;
    }

    /**
     *  Gets the timeout value for the data
     *  @return size_t
     */
    virtual size_t key() const
    {
      return std::numeric_limits<Database::Key>::max();
    }

  protected:
    friend Aurora::Datastore::Manager;
    virtual void assignDatabase( Database::RAM *const database ){};
    virtual Aurora::Database::RAM *getDatabase()
    {
      return nullptr;
    };
  };


  template<typename DataType, const Database::Key AccessKey, const size_t NumObservers, const size_t Rate, const size_t Timeout>
  class BaseObservable : public etl::observable<etl::observer<DataType>, NumObservers>, virtual public IObservableAttr
  {
  public:
    BaseObservable() : mDB( nullptr ), mLastUpdate( 0 ), mKey( AccessKey ), mRate( Rate ), mTimeout( Timeout )
    {
    }

    /*-------------------------------------------------------------------------------
    Public Functions
    -------------------------------------------------------------------------------*/
    bool isExpired() const final override
    {
      return ( ( Chimera::millis() - mLastUpdate ) > timeout() );
    }


    size_t timeout() const final override
    {
      return mTimeout;
    }


    size_t updateRate() const final override
    {
      return mRate;
    }


    size_t size() const final override
    {
      return sizeof( DataType );
    }


    size_t key() const final override
    {
      return mKey;
    }


    bool read( void *const data, const size_t size ) const final override
    {
      if ( !data || !size || ( size != this->size() ) )
      {
        return false;
      }

      mDB->lock();
      bool result = mDB->read( key(), data );
      mDB->unlock();
      return result;
    }


    bool valid() const final override
    {
      DataType tmp;
      return read( &tmp, sizeof( tmp ) ) && validate( &tmp, sizeof( tmp ) );
    }

  protected:
    friend Aurora::Datastore::Manager;
    size_t mLastUpdate;

    /*-------------------------------------------------------------------------------
    Protected Functions
    -------------------------------------------------------------------------------*/
    void assignDatabase( Aurora::Database::RAM *const database ) final override
    {
      RT_HARD_ASSERT( database );
      mDB = database;
    }

    Aurora::Database::RAM *getDatabase() final override
    {
      return mDB;
    }

    void basicInit()
    {
      mDB         = nullptr;
      mLastUpdate = 0;
    }

  private:
    Aurora::Database::RAM *mDB;
    const Database::Key mKey;
    const size_t mRate;
    const size_t mTimeout;
  };
}  // namespace Aurora::Datastore

#endif /* !AURORA_DATASTORE_INTF_HPP */
