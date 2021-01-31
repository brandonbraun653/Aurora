/********************************************************************************
 *  File Name:
 *    datastore_types.hpp
 *
 *  Description:
 *    Types for the datastore
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_DATASTORE_TYPES_HPP
#define AURORA_DATASTORE_TYPES_HPP

/* STL Includes */
#include <cstdint>

/* ETL Includes */
#include <etl/reference_flat_map.h>

/* Aurora Includes */
#include <Aurora/source/datastore/datastore_intf.hpp>
#include <Aurora/source/database/database_types.hpp>


namespace Aurora::Datastore
{
  /*-------------------------------------------------------------------------------
  Aliases
  -------------------------------------------------------------------------------*/
  using ObservableMap = etl::ireference_flat_map<Database::Key, IObservableAttr>;

  template<const size_t SIZE>
  using ObservableMapStorage = etl::reference_flat_map<Database::Key, IObservableAttr, SIZE>;

  /*-------------------------------------------------------------------------------
  Enumerations
  -------------------------------------------------------------------------------*/
  enum CallbackId : uint8_t
  {
    CB_UNHANDLED,
    CB_REGISTER_FAIL,
    CB_INVALID_KEY,

    CB_NUM_OPTIONS
  };

}  // namespace Aurora::Datastore

#endif  /* !AURORA_DATASTORE_TYPES_HPP */
