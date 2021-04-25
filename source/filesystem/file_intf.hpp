/********************************************************************************
 *  File Name:
 *    file_intf.hpp
 *
 *  Description:
 *    High level file system wrapper for consistent project interfacing
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_FILESYSTEM_INTERFACE_HPP
#define AURORA_FILESYSTEM_INTERFACE_HPP

/* STL Includes */
#include <cstdint>
#include <cstddef>

/* Aurora Includes */
#include <Aurora/source/filesystem/file_types.hpp>

namespace Aurora::FileSystem
{
  /*-------------------------------------------------------------------------------
  Public Functions
  -------------------------------------------------------------------------------*/
  /**
   *  Configures the file system to use the appropriate driver. This
   *  assumes that any driver specific data has already been passed
   *  to it and the system is in a state that is ready for initialization.
   *
   *  @param[in]  type        Which filesystem driver to use
   *  @return bool
   */
  bool configureDriver( const BackendType type );

  /**
   *  Mounts the file system
   *  @return int
   */
  int mount();

  /**
   *  Un-mounts the file system
   *  @return int
   */
  int unmount();

  /**
   * @brief Completely wipes the entire filesystem
   */
  void massErase();

  /**
   * @brief Checks if the given file handle is valid
   *
   * @param stream            Handle to check
   * @return true             Valid
   * @return false            Not valid
   */
  bool fIsValid( FileHandle &stream );

  /*-------------------------------------------------------------------------------
  Public Functions: stdio-like Interface
  -------------------------------------------------------------------------------*/
  FileHandle fopen( const char *filename, const char *mode );
  int fclose( FileHandle stream );
  int fflush( FileHandle stream );
  size_t fread( void *ptr, size_t size, size_t count, FileHandle stream );
  size_t fwrite( const void *ptr, size_t size, size_t count, FileHandle stream );
  int fseek( FileHandle stream, size_t offset, size_t origin );
  size_t ftell( FileHandle stream );
  void frewind( FileHandle stream );

}  // namespace Aurora::FileSystem

#endif  /* !AURORA_FILESYSTEM_INTERFACE_HPP */
