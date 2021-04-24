/********************************************************************************
 *  File Name:
 *    file_binary.cpp
 *
 *  Description:
 *    Implementation of a binary file overlay to the Aurora core filesystem.
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* Aurora Includes */
#include <Aurora/filesystem>

/* ETL Includes */
#include <etl/crc32.h>

namespace Aurora::FileSystem
{
  /*-------------------------------------------------------------------------------
  Binary File
  -------------------------------------------------------------------------------*/

  BinaryFile::BinaryFile() : mIsOpen( false ), mFileHandle( {} ), mError( BinaryFile::ECode::ERR_OK )
  {
  }


  BinaryFile::~BinaryFile()
  {
    this->close();
  }


  bool BinaryFile::open( const std::string_view &filename, const std::string_view &mode )
  {
  }


  void BinaryFile::close()
  {
    if( mIsOpen )
    {
      // Nothing much to do. Simply close out the file.
    }
  }


  bool BinaryFile::read( void *const buffer, const size_t size )
  {
    /*
      - Read a few bytes from the start of the file to determine the structure version
      - Go to the offset where the data is stored
      - Match the reported size <= buffer size
      - Read the data in to the user's buffer
      - Do multi-step CRC32 accumulate to verify integrity over structure and raw data
      - Return pass/fail
    */
  }


  bool BinaryFile::write( const void *const buffer, const size_t size )
  {
    /*
      - Ensure the file is open
      - Generate a new structure and calculate the entire CRC
      - Write the structure, then the buffer data to file
      - Return pass/fail
    */
  }


  BinaryFile::ECode BinaryFile::getError()
  {
    return mError;
  }

}  // namespace Aurora::FileSystem
