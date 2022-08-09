/********************************************************************************
 *  File Name:
 *    file_intf.hpp
 *
 *  Description:
 *    High level file system wrapper for consistent project interfacing
 *
 *  2021-2022 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_FILESYSTEM_INTERFACE_HPP
#define AURORA_FILESYSTEM_INTERFACE_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/source/filesystem/file_types.hpp>
#include <cstddef>
#include <cstdint>

namespace Aurora::FileSystem
{
  /*-------------------------------------------------------------------------------
  Public Functions
  -------------------------------------------------------------------------------*/
  /**
   * @brief Poor man's polymorphism to attach the correct FS driver.
   *
   * @param intf      Which filesystem driver to use
   * @return true
   * @return false
   */
  void attachImplementation( const Interface *const intf );

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
  Public Functions
    Note: Undocumented functions are expected to behave the same as STDIO
  -------------------------------------------------------------------------------*/
  /**
   * @brief Open a file for reading or writing
   *
   * @param filename  File to open
   * @param mode      What mode to open in "r" for read, "w" for write, "a" for append, "r+" for read/write
   * @param size      Hint the size of the file in bytes
   * @return FileHandle   Handle to the opened file
   */
  FileHandle fopen( const char *filename, const char *mode, const size_t size = 0 );

  int fclose( FileHandle stream );
  int fflush( FileHandle stream );
  size_t fread( void *ptr, size_t size, size_t count, FileHandle stream );
  size_t fwrite( const void *ptr, size_t size, size_t count, FileHandle stream );
  int fseek( FileHandle stream, size_t offset, size_t origin );
  size_t ftell( FileHandle stream );
  void frewind( FileHandle stream );

}  // namespace Aurora::FileSystem

#endif  /* !AURORA_FILESYSTEM_INTERFACE_HPP */
