/********************************************************************************
 *  File Name:
 *    file_binary.cpp
 *
 *  Description:
 *    Implementation of a binary file overlay to the Aurora core filesystem.
 *
 *  2021-2022 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/filesystem>
#include <Chimera/common>
#include <etl/crc32.h>

namespace Aurora::FileSystem
{
  /*-------------------------------------------------------------------------------
  Binary File
  -------------------------------------------------------------------------------*/
  BinaryFile::BinaryFile() : mIsOpen( false ), mFileId( -1 ), mError( ERR_OK )
  {
  }


  BinaryFile::~BinaryFile()
  {
    this->close();
  }


  bool BinaryFile::open( const std::string_view &filename, const AccessFlags mode )
  {
    return fopen( filename.data(), mode, mFileId ) == 0;
  }


  void BinaryFile::close()
  {
    if ( mIsOpen )
    {
      fclose( mFileId );
      mFileId = {};
      mIsOpen = false;
    }
  }


  bool BinaryFile::read( void *const buffer, const size_t size )
  {
    /*-------------------------------------------------
    Input protection
    -------------------------------------------------*/
    if ( !mIsOpen )
    {
      mError = this->ERR_NOT_OPEN;
      return false;
    }

    /*-------------------------------------------------
    Read out file metadata and do a few checks
    -------------------------------------------------*/
    BinaryFile::LogStruct meta;

    auto metaReadSize = fread( &meta, 1, sizeof( BinaryFile::LogStruct ), mFileId );
    if ( metaReadSize != sizeof( BinaryFile::LogStruct ) )
    {
      mError = this->ERR_READ_FAIL;
      return false;
    }

    if ( size > meta.file_size )
    {
      mError = this->ERR_SIZING;
      return false;
    }

    /*-------------------------------------------------
    Read the file data into the user's buffer
    -------------------------------------------------*/
    auto dataReadSize = fread( buffer, 1, size, mFileId );
    if ( dataReadSize != size )
    {
      mError = this->ERR_READ_FAIL;
      return false;
    }

    /*-------------------------------------------------
    Validate the CRC
    -------------------------------------------------*/
    /* Calculate the CRC boundaries */
    auto userData     = reinterpret_cast<const uint8_t *const>( buffer );
    uint8_t *metaAddr = reinterpret_cast<uint8_t *>( &meta );
    uint8_t *start    = metaAddr + offsetof( BinaryFile::LogStruct, version );
    uint8_t *end      = metaAddr + offsetof( BinaryFile::LogStruct, timestamp ) + sizeof( BinaryFile::LogStruct::timestamp );

    /* Calculate the CRC over the structure and user data */
    etl::crc32 crc;
    crc.reset();
    crc.add( start, end );
    crc.add( userData, userData + size );

    uint32_t calc_crc = crc.value();
    if ( meta.crc != calc_crc )
    {
      mError = this->ERR_CRC_FAIL;
      return false;
    }

    return true;
  }


  bool BinaryFile::write( const void *const buffer, const size_t size )
  {
    /*-------------------------------------------------
    Input protection
    -------------------------------------------------*/
    if ( !buffer || !size )
    {
      mError = this->ERR_BAD_ARG;
      return false;
    }
    else if ( !mIsOpen )
    {
      mError = this->ERR_NOT_OPEN;
      return false;
    }

    /*-------------------------------------------------
    Generate a new structure for writing the file
    -------------------------------------------------*/
    BinaryFile::LogStruct entry;
    entry.timestamp = Chimera::millis();
    entry.file_size = size;

    /* Calculate the CRC boundaries */
    auto userData      = reinterpret_cast<const uint8_t *const>( buffer );
    uint8_t *entryAddr = reinterpret_cast<uint8_t *>( &entry );
    uint8_t *start     = entryAddr + offsetof( BinaryFile::LogStruct, version );
    uint8_t *end       = entryAddr + offsetof( BinaryFile::LogStruct, timestamp ) + sizeof( BinaryFile::LogStruct::timestamp );

    /* Calculate the CRC over the structure and user data */
    etl::crc32 crc;
    crc.reset();
    crc.add( start, end );
    crc.add( userData, userData + size );

    /* Update entry's notion of the CRC */
    entry.crc = crc.value();

    /*-------------------------------------------------
    Write the structure to file first, then the data
    -------------------------------------------------*/
    auto entryWriteSize = fwrite( &entry, 1, sizeof( entry ), mFileId );
    if ( entryWriteSize != sizeof( entry ) )
    {
      mError = this->ERR_WRITE_FAIL;
      return false;
    }

    auto dataWriteSize = fwrite( buffer, 1, size, mFileId );
    if ( dataWriteSize != size )
    {
      mError = this->ERR_WRITE_FAIL;
      return false;
    }

    return true;
  }


  BinaryFile::ECode BinaryFile::getError()
  {
    return mError;
  }


  void BinaryFile::clearErrors()
  {
    mError = this->ERR_OK;
  }

}  // namespace Aurora::FileSystem
