/******************************************************************************
 *  File Name:
 *    datastore_macro.hpp
 *
 *  Description:
 *    Macros to help with the generation of observable data types that are used
 *    by the datastore manager.
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *****************************************************************************/

#pragma once
#ifndef AURORA_DATASTORE_MACROS_HPP
#define AURORA_DATASTORE_MACROS_HPP

/* STL Includes */
#include <cstddef>

/* ETL Includes */
#include <etl/macros.h>

/* Aurora Includes */
#include <Aurora/source/database/volatile/database_intf.hpp>
#include <Aurora/source/datastore/datastore_types.hpp>

/* Chimera Includes */
#include <Chimera/assert>
#include <Chimera/common>

/* Project Includes */
#if __has_include( "aurora_datastore_types_prj.hpp" )
#include "aurora_datastore_types_prj.hpp"
#endif

/**
 * @brief Helper for declaring an observable data type
 * @note  The definition of this class must go into a separate file.
 *
 * An "observable" is a living object that acts as a proxy for some kind of changing data
 * in the system. It knows how to grab updates to that data and, upon a change, will notify
 * all registered observers of the new information.
 *
 * @param[in]  Name          Name of the observable data
 * @param[in]  AccessKey     Key identifier used to access the underlying database
 * @param[in]  NumObservers  How many observers this observable can support
 * @param[in]  Rate          How often the update function should be called in milliseconds
 * @param[in]  Timeout       How long the data can remain valid without an other update
 */
#define ObservableDeclare( Name, Type, AccessKey, NumObservers, Rate, Timeout )              \
  class ETL_CONCAT( Observable_, Name ) :                                                    \
      public Aurora::Datastore::BaseObservable<Type, AccessKey, NumObservers, Rate, Timeout> \
  {                                                                                          \
  public:                                                                                    \
    /*-------------------------------------------------------------------------------        \
    Implementer Supplied Functionality                                                       \
    -------------------------------------------------------------------------------*/        \
    void initialize() final override;                                                        \
    void update() final override;                                                            \
    void onTimeout() final override;                                                         \
    bool validate( const void *const data, const size_t size ) const final override;         \
  };                                                                                         \
                                                                                             \
  extern ETL_CONCAT( Observable_, Name ) ETL_CONCAT( Param_, Name );


/**
 * @brief Helper to generate a reference to the object created by ObservableDeclare
 */
#define ObservablePointer( Name ) ( &ETL_CONCAT( Param_, Name ) )


/**
 * @brief Helper for declaring an observer for an observable.
 * @note The type should match the associated Observable declaration.
 *
 * @param Name    Name of the observer
 * @param Type    Observed data type
 */
#define ObserverDeclare( Name, Type )              \
  class Name : public etl::observer<Type>          \
  {                                                \
  public:                                          \
    void notification( Type data ) final override; \
  };

#endif /* !AURORA_DATASTORE_MACROS_HPP */
