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
  /*---------------------------------------------------------------------------
  Constants
  ---------------------------------------------------------------------------*/
  static constexpr size_t MAX_VOLUMES          = 5;
  static constexpr size_t MAX_OPEN_FILES       = 5;
  static constexpr size_t MAX_FILE_NAME_LEN    = 32;
  static constexpr size_t MAX_DRIVE_PREFIX_LEN = 8;

  /*---------------------------------------------------------------------------
  Aliases
  ---------------------------------------------------------------------------*/
  using VolumeId = int;   /**< Identifier for a specific volume */
  using FileId   = int;   /**< Identifier for a specific file */

  /*---------------------------------------------------------------------------
  Enumerations
  ---------------------------------------------------------------------------*/
  enum AccessFlags : uint32_t
  {
    O_RDONLY  = ( 1u << 0 ),
    O_WRONLY  = ( 1u << 1 ),
    O_RDWR    = ( 1u << 2 ),
    O_APPEND  = ( 1u << 3 ),
    O_CREAT   = ( 1u << 4 ),
    O_EXCL    = ( 1u << 5 ),
    O_TRUNC   = ( 1u << 6 )
  };

  /*---------------------------------------------------------------------------
  Structures
  ---------------------------------------------------------------------------*/
  /**
   * @brief Function pointers implemented by all filesystem drivers
   *
   * By implementing these functions, a wide variety of filesystem types can
   * be supported. There are quite a few variations out there ranging from
   * full blown computers to highly restricted embedded systems that just need
   * the simple ability to access a file to log some data. Ideally this will
   * be abstract enough to port across this range of system types.
   */
  struct Interface
  {
    /*-------------------------------------------------------------------------
    Interface Specific Control Functions
    -------------------------------------------------------------------------*/
    /**
     * @brief Powers up the filesystem interface.
     * @return 0 if OK, negative otherwise
     */
    int ( *initialize )();

    /**
     * @brief Perform implementation specific aspects of mounting
     *
     * @param drive   Unique drive string assigned by the high level driver
     * @return 0 if OK, negative otherwise
     */
    int ( *mount )( const std::string_view &drive );

    /**
     * @brief Tears down implementation specific aspects of the drive
     *
     * @return 0 if OK, negative otherwise
     */
    int ( *unmount )();

    /*-------------------------------------------------------------------------
    Standard Filesystem Interface
    -------------------------------------------------------------------------*/
    int ( *fopen )( const char *filename, const AccessFlags mode, FileId &file );
    int ( *fclose )( const FileId stream );
    int ( *fflush )( const FileId stream );
    size_t ( *fread )( void *const ptr, const size_t size, const size_t count, const FileId stream );
    size_t ( *fwrite )( const void *const ptr, const size_t size, const size_t count, const FileId stream );
    int ( *fseek )( const FileId stream, const size_t offset, const size_t origin );
    size_t ( *ftell )( const FileId stream );
    void ( *frewind )( const FileId stream );

    void clear()
    {
      initialize = nullptr;
      mount      = nullptr;
      fopen      = nullptr;
      fclose     = nullptr;
      fflush     = nullptr;
      fread      = nullptr;
      fwrite     = nullptr;
      fseek      = nullptr;
      ftell      = nullptr;
      frewind    = nullptr;
    }
  };
}  // namespace Aurora::FileSystem

#endif /* !AURORA_FILE_SYSTEM_TYPES_HPP */
