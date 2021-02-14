/********************************************************************************
 *  File Name:
 *    file_types.hpp
 *
 *  Description:
 *    File system types
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_FILE_SYSTEM_TYPES_HPP
#define AURORA_FILE_SYSTEM_TYPES_HPP

/* STL Includes */
#include <cstddef>

namespace Aurora::FileSystem
{
  /*-------------------------------------------------------------------------------
  Aliases
  -------------------------------------------------------------------------------*/
  using FileHandle = void *;

  /*-------------------------------------------------------------------------------
  Enumerations
  -------------------------------------------------------------------------------*/
  enum BackendType : size_t
  {
    DRIVER_LITTLE_FS, /**< Little FS embedded driver */
    DRIVER_YAFFS2,    /**< Yet-Another-FAT-File-System (YAFFS) */
    DRIVER_OS,        /**< Compiled OS target file system, aka C++17 filesystem library */

    DRIVER_NUM_OPTIONS,
    DRIVER_UNKNOWN
  };


  /*-------------------------------------------------------------------------------
  Structures
  -------------------------------------------------------------------------------*/
  struct Interface
  {
    int ( *mount )();
    int ( *unmount )();
    FileHandle ( *fopen )( const char *filename, const char *mode );
    int ( *fclose )( FileHandle stream );
    int ( *fflush )( FileHandle stream );
    size_t ( *fread )( void *ptr, size_t size, size_t count, FileHandle stream );
    size_t ( *fwrite )( const void *ptr, size_t size, size_t count, FileHandle stream );
    int ( *fseek )( FileHandle stream, size_t offset, size_t origin );
    size_t ( *ftell )( FileHandle stream );
    void ( *frewind )( FileHandle stream );
  };
}  // namespace Aurora::FileSystem

#endif  /* !AURORA_FILE_SYSTEM_TYPES_HPP */
