/******************************************************************************
 *  File Name:
 *    db_shared_types.hpp
 *
 *  Description:
 *    Shared types between databases
 *
 *  2022 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef AURORA_DATABASE_SHARED_TYPES_HPP
#define AURORA_DATABASE_SHARED_TYPES_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <cstdint>

namespace Aurora::Database
{
  /*---------------------------------------------------------------------------
  Aliases
  ---------------------------------------------------------------------------*/
  using Key = uint16_t;

}  // namespace Aurora::Database

#endif  /* !AURORA_DATABASE_SHARED_TYPES_HPP */
