/******************************************************************************
 *  File Name:
 *    db_sto_api.hpp
 *
 *  Description:
 *    Storage controller interface
 *
 *  2022 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef AURORA_STORAGE_CONTROL_INTERFACE_HPP
#define AURORA_STORAGE_CONTROL_INTERFACE_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/source/database/persistent/db_nvm_types.hpp>

namespace Aurora::Database::Persistent
{

  /*---------------------------------------------------------------------------
  Classes
  ---------------------------------------------------------------------------*/
  class IStorageController
  {
  public:
    virtual ~IStorageController() = default;
  };
}  // namespace Aurora::Database::Persistent

#endif  /* !AURORA_STORAGE_CONTROL_INTERFACE_HPP */
