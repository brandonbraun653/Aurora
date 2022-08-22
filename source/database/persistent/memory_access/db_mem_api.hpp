/******************************************************************************
 *  File Name:
 *    db_mem_api.hpp
 *
 *  Description:
 *    Memory access layer interface
 *
 *  2022 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef AURORA_DATABASE_MEMORY_INTERFACE_HPP
#define AURORA_DATABASE_MEMORY_INTERFACE_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/


namespace Aurora::Database::Persistent
{
  /*---------------------------------------------------------------------------
  Classes
  ---------------------------------------------------------------------------*/
  class IMemoryController
  {
  public:
    virtual ~IMemoryController() = default;
  };
}  // namespace Aurora::Database::Persistent

#endif  /* !AURORA_DATABASE_MEMORY_INTERFACE_HPP */
