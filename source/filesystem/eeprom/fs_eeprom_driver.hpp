/******************************************************************************
 *  File Name:
 *    fs_eeprom_driver.cpp
 *
 *  Description:
 *    Interface for an EEPROM based file system
 *
 *  2022 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef AURORA_FILESYSTEM_EEPROM_HPP
#define AURORA_FILESYSTEM_EEPROM_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/source/filesystem/eeprom/fs_eeprom_types.hpp>
#include <Aurora/source/filesystem/file_types.hpp>
#include <Aurora/source/memory/flash/eeprom/eeprom_generic_driver.hpp>
#include <Chimera/common>
#include <cstdint>


namespace Aurora::FileSystem::EEPROM
{
  /*---------------------------------------------------------------------------
  Public Data
  ---------------------------------------------------------------------------*/
  extern const Interface implementation; /**< Function pointers to the EEPROM file system driver */

  /*---------------------------------------------------------------------------
  Classes
  ---------------------------------------------------------------------------*/
  /**
   * @brief Core manager for a single file system instance inside an EEPROM
   */
  class Manager
  {
  public:
    Manager();
    ~Manager();

    /**
     * @brief Initialize the filesystem manager
     *
     * @param driver      Non-volatile memory driver for raw data access
     * @param mbrCache    Project's working memory to cache the MBR
     */
    void configure( Aurora::Flash::EEPROM::Driver *const driver, iMBR *const mbrCache );

    /**
     * @brief Validate that MBR is initialized and in a good state
     *
     * @return bool
     */
    bool mount();

    /**
     * @brief Destroy this manager's knowledge of the underlying filesystem
     */
    void unmount();

    /**
     * @brief Does a soft reset of the filesystem by clearing the MBR
     *
     * @return int
     */
    int softReset();

  protected:
    /**
     * @brief Loads the MBR from NVM into local memory, checking for validity.
     *
     * @return true   MBR is valid and loaded
     * @return false  Can't trust the state of the local MBR
     */
    bool refreshMBRCache();

  private:
    iMBR                          *mMBRCache;
    Aurora::Flash::EEPROM::Driver *mNVMDriver;
  };
}  // namespace Aurora::FileSystem::EEPROM

#endif /* !AURORA_FILESYSTEM_EEPROM_HPP */
