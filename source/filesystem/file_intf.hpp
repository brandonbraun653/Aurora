/********************************************************************************
 *  File Name:
 *    file_intf.hpp
 *
 *  Description:
 *    High level file system wrapper for consistent project interfacing. This
 *    driver works under the assumption of a few constraints:
 *
 *      1. This is not your fully featured file system. It's an embedded system.
 *      2. The primary goal is to allow read/write access to small files.
 *      3. Absolute paths are always used.
 *      4. Don't expect fancy features like searching for files, resizing them,
 *         or inspecting the mount drive. Again, we're targeting small systems.
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
  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/
  /**
   * @brief Initialize system level module aspects of the filesystem manager
   *
   * Call this function once during boot up of the system to ensure the
   * memory associated with the filesystem manager is ready for use.
   */
  void initialize();

  /**
   * @brief Registers a drive with the filesystem manager
   *
   * @param drive     The drive letter to mount against
   * @param intf      Which filesystem driver to use
   * @return A volume ID >= 0 if successful, negative otherwise
   */
  VolumeId mount( const std::string_view &drive, Interface &intf );

  /**
   * @brief Unmounts a previously mounted volume
   *
   * @param volume    Which volume to unmount
   */
  void unmount( const VolumeId volume );

  /**
   * @brief Open a file stream
   *
   * Expects the filename to be an absolute path that starts with one of the
   * drive letters assigned when a volume was mounted.
   *
   * @param filename  File to open
   * @param mode      File access flags
   * @param file      Output file descriptor
   * @return int      0 if all ok, negative otherwise
   */
  int fopen( const char *filename, const AccessFlags mode, FileId &file );

  /**
   * @brief Close a file stream
   *
   * @param stream    File stream being closed
   * @return int      0 if all ok, negative otherwise
   */
  int fclose( const FileId stream );

  /**
   * @brief Forces a write of all user-space buffered data
   *
   * @param stream    The stream to flush
   * @return int      0 if all ok, negative otherwise
   */
  int fflush( const FileId stream );

  /**
   * @brief Read data from the stream
   *
   * Reads "count" items of data, each "size" bytes long, from the stream
   * pointed to by "stream", storing them at the location given by "ptr".
   *
   * @param ptr       Output storage location for the read data
   * @param size      Size in bytes of each item to read
   * @param count     Total number of items to read
   * @param stream    Stream to read from
   * @return size_t   Number of bytes read, else zero on error
   */
  size_t fread( void *const ptr, const size_t size, const size_t count, const FileId stream );

  /**
   * @brief Write data to the stream
   *
   * Write "count" items of data, each "size" bytes long, from the location
   * given by "ptr", into the "stream" file.
   *
   * @param ptr       Output storage location for the read data
   * @param size      Size in bytes of each item to read
   * @param count     Total number of items to read
   * @param stream    Stream to read from
   * @return size_t   Number of bytes read, else zero on error
   */
  size_t fwrite( const void *const ptr, const size_t size, const size_t count, const FileId stream );

  /**
   * @brief Sets the file position indicator for the stream pointed to by stream
   *
   * The new position, measured in bytes, is obtained by adding "offset" bytes to
   * the position specified by "origin".
   *
   * @param stream    Stream to act on
   * @param offset    Number of bytes to offset from origin
   * @param origin    Starting location offset is applied to
   * @return int      0 if all ok, negative otherwise
   */
  int fseek( const FileId stream, const size_t offset, const WhenceFlags whence );

  /**
   * @brief Gets the current file position indicator
   *
   * @param stream    Stream to inspect
   * @return size_t   Current offset
   */
  size_t ftell( const FileId stream );

  /**
   * @brief Sets the file position indicator back to SOF
   *
   * @param stream    Stream to act on
   */
  void frewind( const FileId stream );

}  // namespace Aurora::FileSystem

#endif /* !AURORA_FILESYSTEM_INTERFACE_HPP */
