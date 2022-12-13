/******************************************************************************
 *  File Name:
 *    spiffs_driver.hpp
 *
 *  Description:
 *    Interface to the SPIFFS implementation
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *****************************************************************************/

#pragma once
#ifndef AURORA_FILESYSTEM_SPIFFS_DRIVER_HPP
#define AURORA_FILESYSTEM_SPIFFS_DRIVER_HPP

/* STL Includes */
#include <cstdint>

/* Aurora Includes */
#include <Aurora/source/filesystem/file_types.hpp>


namespace Aurora::FileSystem::SPIFFS
{
  /*---------------------------------------------------------------------------
  Public Data
  ---------------------------------------------------------------------------*/
  extern const Interface implementation;

  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/
  int nor_read( unsigned int addr, unsigned int size, uint8_t *dst );
  int nor_write( unsigned int addr, unsigned int size, uint8_t *src );
  int nor_erase( unsigned int addr, unsigned int size );

}  // namespace Aurora::FileSystem::SPIFFS

#endif  /* !AURORA_FILESYSTEM_SPIFFS_DRIVER_HPP */
