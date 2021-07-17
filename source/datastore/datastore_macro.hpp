/********************************************************************************
 *  File Name:
 *    datastore_macro.hpp
 *
 *  Description:
 *    Macros to help with the generation of observable data types that are used
 *    by the datastore manager.
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_DATASTORE_MACROS_HPP
#define AURORA_DATASTORE_MACROS_HPP

/* STL Includes */
#include <cstddef>

/* ETL Includes */
#include <etl/macros.h>

/* Aurora Includes */
#include <Aurora/source/database/database_intf.hpp>
#include <Aurora/source/datastore/datastore_types.hpp>

/* Chimera Includes */
#include <Chimera/assert>
#include <Chimera/common>

/* Project Includes */
#if __has_include( "aurora_datastore_types_prj.hpp" )
#include "aurora_datastore_types_prj.hpp"
#endif

/**
 *  Helper for declaring an observable data type. The definition of this class must go into
 *  a separate file.
 *
 *  @param[in]  Name          Name of the observable data
 *  @param[in]  AccessKey     Key identifier used to access the underlying database
 *  @param[in]  NumObservers  How many observers this observable can support
 *  @param[in]  Rate          How often the update function should be called in milliseconds
 *  @param[in]  Timeout       How long the data can remain valid without an other update
 */
#define DECLARE_OBSERVABLE( Name, Type, AccessKey, NumObservers, Rate, Timeout )             \
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
 *  Given a friendly observable name, converts it into a pointer
 *  reference of the type created by DECLARE_OBSERVABLE
 */
#define OBSERVABLE_PTR( Name ) ( &ETL_CONCAT( Param_, Name ) )

#endif /* !AURORA_DATASTORE_MACROS_HPP */
