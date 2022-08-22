/******************************************************************************
 *  File Name:
 *    db_nvm_types.hpp
 *
 *  Description:
 *    Types for the persistent memory database
 *
 *  2022 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef AURORA_DATABASE_NVM_TYPES_HPP
#define AURORA_DATABASE_NVM_TYPES_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <cstdint>
#include <etl/variant.h>
#include <etl/span.h>


namespace Aurora::Database::Persistent
{
  /*---------------------------------------------------------------------------
  Aliases
  ---------------------------------------------------------------------------*/
  /**
   * @brief Core storage type that can be R/W to NVM
   *
   * Using the etl::span type to act as a view into array like memory without
   * actually having to store or pass around that data.
   */
  using Storage_t = etl::variant<bool, uint8_t, uint16_t, uint32_t, uint64_t, float, double, etl::span<uint8_t>>;

  /*---------------------------------------------------------------------------
  Enumerations
  ---------------------------------------------------------------------------*/
  enum ReturnCode : int
  {
    ERR_INVALID_KEY = -1,
  };
}  // namespace Aurora::Database::Persistent

#endif  /* !AURORA_DATABASE_NVM_TYPES_HPP */
