/********************************************************************************
 *  File Name:
 *    file_binary.hpp
 *
 *  Description:
 *    High level interface to data stored as a binary file
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_BINARY_FILES_HPP
#define AURORA_BINARY_FILES_HPP

/* STL Includes */
#include <cstdint>
#include <cstring>
#include <string_view>

/* Aurora Includes */
#include <Aurora/source/filesystem/file_types.hpp>

namespace Aurora::FileSystem
{
  /**
   * @brief Context managed file interface to binary data
   *
   * Stores raw binary data to disk using a structured format such that it's easy
   * to detect any errors. The class expects the entire file to be read/written
   * on any IO operation due to the minimalist nature of the underlying filesystem.
   */
  class BinaryFile
  {
  public:
    enum ECode : uint8_t
    {
      ERR_OK,           /**< No error */
      ERR_CRC_FAIL,     /**< The data was corrupted */
      ERR_NO_FILE,      /**< The file doesn't exist */
      ERR_PERMISSION,   /**< The requested IO operation was not allowed */
    };

    /*-------------------------------------------------
    Public API
    -------------------------------------------------*/
    BinaryFile();
    ~BinaryFile();

    /**
     * @brief Opens the specified file
     *
     * @param filename      Name of the file to open
     * @param mode          Mode to open the file in
     * @return true         File successfully opened
     * @return false        An error occurred, see getError()
     */
    bool open( const std::string_view &filename, const std::string_view &mode );

    /**
     * @brief Closes the file
     * If already closed, does nothing.
     */
    void close();

    /**
     * @brief Read data from an open file
     *
     * @param buffer        Buffer to read data into
     * @param size          Number of bytes to read
     * @return true         The read completed successfully
     * @return false        An error occurred, see getError()
     */
    bool read( void *const buffer, const size_t size );

    /**
     * @brief Writes data to an open file
     *
     * @param buffer        Data to write
     * @param size          Number of bytes to write
     * @return true         Write completed successfully
     * @return false        An error occurred, see getError()
     */
    bool write( const void *const buffer, const size_t size );

    /**
     * @brief Retrieves the last error that occurred
     *
     * @return ECode
     */
    ECode getError();

  protected:
    struct LogStruct
    {
      uint32_t crc;           /**< CRC of the entire file */
      const uint8_t version;  /**< Log structure version */
      uint8_t pad[ 3 ];       /**< Unused */
      uint32_t file_size;     /**< Length of the file in bytes */
      uint32_t timestamp;     /**< Last time written */

      LogStruct() : version( StructVersion )
      {
        memset( pad, 0, sizeof( pad ) );
        crc       = 0xCCCCCCCC;
        file_size = 0xCCCCCCCC;
        timestamp = 0xCCCCCCCC;
      }

    private:
      static constexpr uint8_t StructVersion = 1u;
    };

  private:
    bool mIsOpen;
    FileHandle mFileHandle;
    ECode mError;
  };
}  // namespace Aurora::FileSystem

#endif  /* !AURORA_BINARY_FILES_HPP */
