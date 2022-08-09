/********************************************************************************
 *  File Name:
 *    file_types.hpp
 *
 *  Description:
 *    File system types
 *
 *  2021-2022 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_FILE_SYSTEM_TYPES_HPP
#define AURORA_FILE_SYSTEM_TYPES_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <cstddef>

namespace Aurora::FileSystem
{
/*-------------------------------------------------------------------------------
Aliases
-------------------------------------------------------------------------------*/
#if defined( SIMULATOR )
  using FileHandle = void *;
#else
  using FileHandle = int;
#endif

  /*-------------------------------------------------------------------------------
  Enumerations
  -------------------------------------------------------------------------------*/
  enum BackendType : size_t
  {
    DRIVER_LITTLE_FS, /**< Little FS embedded driver */
    DRIVER_SPIFFS,    /**< SPI Flash FileSystem */
    DRIVER_EEFS,      /**< EEPROM FileSystem */
    DRIVER_OS,        /**< Compiled OS target file system, aka C++17 filesystem library */

    DRIVER_NUM_OPTIONS,
    DRIVER_UNKNOWN
  };

  enum IODirection : size_t
  {
    IO_READ,  /**< Reads data */
    IO_WRITE, /**< Overwrites all data */
  };


  /*-------------------------------------------------------------------------------
  Structures
  -------------------------------------------------------------------------------*/
  /**
   * @brief Convenience container for registering a file system driver
   * @note See documentation in file_intf.hpp for more details on function behavior
   */
  struct Interface
  {
    int ( *mount )();
    int ( *unmount )();
    FileHandle ( *fopen )( const char *filename, const char *mode, const size_t size );
    int ( *fclose )( FileHandle stream );
    int ( *fflush )( FileHandle stream );
    size_t ( *fread )( void *ptr, size_t size, size_t count, FileHandle stream );
    size_t ( *fwrite )( const void *ptr, size_t size, size_t count, FileHandle stream );
    int ( *fseek )( FileHandle stream, size_t offset, size_t origin );
    size_t ( *ftell )( FileHandle stream );
    void ( *frewind )( FileHandle stream );
  };
}  // namespace Aurora::FileSystem

#endif /* !AURORA_FILE_SYSTEM_TYPES_HPP */
