/******************************************************************************
 *  File Name:
 *    db_nvm_driver.hpp
 *
 *  Description:
 *    High level driver for the database manager
 *
 *  2022 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef AURORA_DATABASE_NVM_HPP
#define AURORA_DATABASE_NVM_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/source/database/db_shared_types.hpp>
#include <Aurora/source/database/persistent/db_nvm_types.hpp>
#include <Aurora/source/database/persistent/memory_access/db_mem_api.hpp>
#include <Aurora/source/database/persistent/storage_control/db_sto_api.hpp>
#include <etl/list.h>

namespace Aurora::Database::Persistent
{
  /*---------------------------------------------------------------------------
  Structures
  ---------------------------------------------------------------------------*/
  struct DBConfig
  {
    etl::ilist<Key>    keyList;           /**< Keys allowed in implementation */
    IMemoryController  memoryController;  /**< User memory controller */
    IStorageController storageController; /**< User storage controller */
  };

  /*---------------------------------------------------------------------------
  Classes
  ---------------------------------------------------------------------------*/
  class DBStore
  {
  public:
    DBStore();
    ~DBStore();

    /**
     * @brief Initializes the controller with the given configuration
     *
     * @param cfg   Configuration to use
     * @return Zero if ready, another number if not
     */
    int open( const DBConfig &cfg );

    int close();

    int read( const Key key, Storage_t &value );

    int write( const Key key, const Storage_t &value );

  private:
    DBConfig mCfg;
  };
}  // namespace Aurora::DB

#endif  /* !AURORA_DATABASE_NVM_HPP */
