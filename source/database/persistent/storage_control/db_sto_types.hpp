/******************************************************************************
 *  File Name:
 *    db_sto_types.hpp
 *
 *  Description:
 *    Storage types for NVM
 *
 *  2022 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef AURORA_DATABASE_STORAGE_TYPES_HPP
#define AURORA_DATABASE_STORAGE_TYPES_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <etl/crc.h>

namespace Aurora::Database::Persistent
{
  /*---------------------------------------------------------------------------
  Classes
  ---------------------------------------------------------------------------*/
  template<class Base>
  class SerializedData
  {
  public:

  };


  class NVMEntry : SerializedData<NVMEntry>
  {
    // Unique Header (32-bit? Mnemonic)
    // CRC (variable size?)
    // Key
    // Data
  };
}  // namespace Aurora::Database::Persistent

#endif  /* !AURORA_DATABASE_STORAGE_TYPES_HPP */
