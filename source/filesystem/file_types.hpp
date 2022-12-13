/******************************************************************************
 *  File Name:
 *    file_types.hpp
 *
 *  Description:
 *    File system types
 *
 *  2021-2022 | Brandon Braun | brandonbraun653@gmail.com
 *****************************************************************************/

#pragma once
#ifndef AURORA_FILE_SYSTEM_TYPES_HPP
#define AURORA_FILE_SYSTEM_TYPES_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <cstddef>
#include <Aurora/utility>

namespace Aurora::FileSystem
{
  /*---------------------------------------------------------------------------
  Constants
  ---------------------------------------------------------------------------*/
  static constexpr size_t MAX_VOLUMES          = 5;
  static constexpr size_t MAX_OPEN_FILES       = 5;
  static constexpr size_t MAX_FILE_NAME_LEN    = 48;
  static constexpr size_t MAX_DRIVE_PREFIX_LEN = 32;

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
    O_RDONLY  = 1,
    O_WRONLY  = 2,
    O_RDWR    = 3,
    O_APPEND  = ( 1u << 4 ),
    O_CREAT   = ( 1u << 5 ),
    O_EXCL    = ( 1u << 6 ),
    O_TRUNC   = ( 1u << 7 ),

    O_ACCESS_MSK = 0x3,
    O_MODIFY_MSK = 0xFFFFFFF0
  };

  ENUM_CLS_BITWISE_OPERATOR( AccessFlags, | );
  ENUM_CLS_BITWISE_OPERATOR( AccessFlags, & );

  enum WhenceFlags : uint32_t
  {
    F_SEEK_SET,
    F_SEEK_CUR,
    F_SEEK_END
  };

#if defined( SIMULATOR )
  static_assert( F_SEEK_SET == SEEK_SET );
  static_assert( F_SEEK_CUR == SEEK_CUR );
  static_assert( F_SEEK_END == SEEK_END );
#endif

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
    void *context;  /**< Any context information needed by the implementation */

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
     * @param drive     Unique drive ID assigned by the high level driver
     * @param context   Context assigned to the interface
     * @return 0 if OK, negative otherwise
     */
    int ( *mount )( const VolumeId drive, void *context );

    /**
     * @brief Tears down implementation specific aspects of the drive
     *
     * @param drive   Unique drive ID assigned by the high level driver
     * @return 0 if OK, negative otherwise
     */
    int ( *unmount )( const VolumeId drive );

    /*-------------------------------------------------------------------------
    Standard Filesystem Interface
    -------------------------------------------------------------------------*/
    /**
     * This declaration differs slightly from the one in file_intf.hpp => the
     * "file" parameter is marked const. Only the high level filesystem manager
     * will assign the FileId to use and it's expected the low level details
     * consume that information for mapping its own file trackers.
     */
    int ( *fopen )( const char *filename, const AccessFlags mode, const FileId file, const VolumeId vol );
    int ( *fclose )( const FileId stream );
    int ( *fflush )( const FileId stream );
    size_t ( *fread )( void *const ptr, const size_t size, const size_t count, const FileId stream );
    size_t ( *fwrite )( const void *const ptr, const size_t size, const size_t count, const FileId stream );
    int ( *fseek )( const FileId stream, const size_t offset, const WhenceFlags whence );
    size_t ( *ftell )( const FileId stream );
    void ( *frewind )( const FileId stream );
    size_t ( *fsize )( const FileId stream );

    void clear()
    {
      context    = nullptr;
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
      fsize      = nullptr;
    }
  };
}  // namespace Aurora::FileSystem

#endif /* !AURORA_FILE_SYSTEM_TYPES_HPP */
