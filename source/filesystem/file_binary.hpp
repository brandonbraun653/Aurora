/********************************************************************************
 *  File Name:
 *    file_binary.hpp
 *
 *  Description:
 *    High level interface to data stored as a binary file
 *
 *  2021-2022 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_BINARY_FILES_HPP
#define AURORA_BINARY_FILES_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/source/filesystem/file_types.hpp>
#include <cstdint>
#include <cstring>
#include <string_view>

namespace Aurora::FileSystem
{
  /**
   * @brief Context managed file interface to binary data
   *
   * Stores raw binary data to disk using a structured format such that it's easy to detect any errors.
   * Expects the entire file to be read/written on any IO operation due to the minimalist nature of the
   * underlying filesystem. Tracking a R/W offset is not supported, mainly due to how infrequently that
   * use case is needed and the complexity of maintaining it.
   */
  class BinaryFile
  {
  public:
    enum ECode : uint8_t
    {
      ERR_OK,           /**< No error */
      ERR_BAD_ARG,      /**< Invalid argument passed to function */
      ERR_CRC_FAIL,     /**< The data was corrupted */
      ERR_WRITE_FAIL,   /**< Data write reported a failure */
      ERR_READ_FAIL,    /**< Data failed to be read */
      ERR_NO_FILE,      /**< The file doesn't exist */
      ERR_NOT_OPEN,     /**< The file isn't open */
      ERR_SIZING,       /**< There was a sizing issue in a read/write command */
    };

    BinaryFile();
    ~BinaryFile();

    /**
     * @brief Creates an empty file
     * @note If the file already exists, it will be destroyed.
     *
     * @param filename      Name of the file to create
     * @return true         File created successfully
     * @return false        File was not created
     */
    bool create( const std::string_view &filename, const size_t size = 0 );

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
     * @note If already closed, does nothing.
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

    /**
     * @brief Clears any set error codes
     */
    void clearErrors();

  protected:
    struct LogStruct
    {
      uint32_t crc;           /**< CRC of the entire file + this LogStruct */
      const uint8_t version;  /**< Log structure version */
      uint8_t _pad[ 3 ];      /**< Unused */
      uint32_t file_size;     /**< Length of the user's data in bytes */
      uint32_t timestamp;     /**< Last time written */

      LogStruct() : version( StructVersion )
      {
        memset( _pad, 0, sizeof( _pad ) );
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
