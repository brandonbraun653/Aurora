/********************************************************************************
 *  File Name:
 *    generic_utils.cpp
 *
 *  Description:
 *    Implementation of utility functions for interacting with memory
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* Aurora Includes */
#include <Aurora/memory>

namespace Aurora::Memory
{
  bool validateProperties( const Properties &cfg )
  {
    /*-------------------------------------------------
    Initialize to true and allow any operation below
    to negate and then latch to false.
    -------------------------------------------------*/
    bool areValid = true;

    /*-------------------------------------------------
    Non-zero sizing fields
    -------------------------------------------------*/
    areValid &= ( cfg.pageSize != 0 ) && ( cfg.numPages != 0 );
    areValid &= ( cfg.blockSize != 0 ) && ( cfg.numBlocks != 0 );
    areValid &= ( cfg.sectorSize != 0 ) && ( cfg.numSectors != 0 );

    /*-------------------------------------------------
    Sizing follows logical structure
    -------------------------------------------------*/
    areValid &= ( cfg.pageSize < cfg.blockSize ) && ( cfg.pageSize < cfg.sectorSize );
    areValid &= ( cfg.blockSize < cfg.sectorSize );

    return areValid;
  }


  ChunkIndex_t chunkIndex( const Properties &cfg, const Chunk view, const SysAddress_t address )
  {
    /*-------------------------------------------------
    Input validity checks
    -------------------------------------------------*/
    if ( !validateProperties( cfg ) )
    {
      return BAD_CHUNK_IDX;
    }

    /*-------------------------------------------------
    Deduce the chunk index. Div-0 protected by above.
    All chunks are zero indexed.
    -------------------------------------------------*/
    switch( view )
    {
      case Chunk::PAGE:
          return address / cfg.pageSize;
        break;

      case Chunk::BLOCK:
          return address / cfg.blockSize;
        break;

      case Chunk::SECTOR:
          return address / cfg.sectorSize;
        break;

      default:
        return BAD_CHUNK_IDX;
        break;
    }
  }


  size_t startOffset( const Properties &cfg, const Chunk view, const SysAddress_t address )
  {
    /*-------------------------------------------------
    Find the starting address of the appropriate chunk
    -------------------------------------------------*/
    ChunkIndex_t index   = chunkIndex( cfg, view, address );
    SysAddress_t startAddress = chunkStartAddress( cfg, view, index);

    /*-------------------------------------------------
    Verify lookup was successful and return the offset
    -------------------------------------------------*/
    if( address >= startAddress )
    {
      return address - startAddress;
    }
    else
    {
      return BAD_OFFSET;
    }
  }


  size_t endOffset( const Properties &cfg, const Chunk view, const SysAddress_t address )
  {
    /*-------------------------------------------------
    Find the ending address of the appropriate chunk
    -------------------------------------------------*/
    ChunkIndex_t index   = chunkIndex( cfg, view, address );
    SysAddress_t endAddress = chunkEndAddress( cfg, view, index);

    /*-------------------------------------------------
    Verify lookup was successful and return the offset
    -------------------------------------------------*/
    if( address <= endAddress )
    {
      return endAddress - address;
    }
    else
    {
      return BAD_OFFSET;
    }
  }


  SysAddress_t chunkStartAddress( const Properties &cfg, const Chunk view, const ChunkIndex_t idx )
  {
    /*-------------------------------------------------
    Input validity checks
    -------------------------------------------------*/
    if ( !validateProperties( cfg ) )
    {
      return BAD_ADDRESS;
    }

    /*-------------------------------------------------
    Chunks are zero indexed, so the start address is
    a simple multiplication.
    -------------------------------------------------*/
    switch( view )
    {
      case Chunk::PAGE:
          return idx * cfg.pageSize;
        break;

      case Chunk::BLOCK:
          return idx * cfg.blockSize;
        break;

      case Chunk::SECTOR:
          return idx * cfg.sectorSize;
        break;

      default:
        return BAD_CHUNK_IDX;
        break;
    }
  }


  SysAddress_t chunkEndAddress( const Properties &cfg, const Chunk view, const ChunkIndex_t idx )
  {
    /*-------------------------------------------------
    Input validity checks
    -------------------------------------------------*/
    if ( !validateProperties( cfg ) )
    {
      return BAD_ADDRESS;
    }

    /*-------------------------------------------------
    Chunks are zero indexed, so the end address is the
    start of the next chunk, but minus 1 byte.
    -------------------------------------------------*/
    switch( view )
    {
      case Chunk::PAGE:
          return ( idx * cfg.pageSize ) + cfg.pageSize - 1u;
        break;

      case Chunk::BLOCK:
          return ( idx * cfg.blockSize ) + cfg.blockSize - 1u;
        break;

      case Chunk::SECTOR:
          return ( idx * cfg.sectorSize ) + cfg.sectorSize - 1u;
        break;

      default:
        return BAD_CHUNK_IDX;
        break;
    }
  }


  size_t chunkSize( const Properties &cfg, const Chunk view )
  {
    switch ( view )
    {
      case Chunk::PAGE:
        return cfg.pageSize;
        break;

      case Chunk::BLOCK:
        return cfg.blockSize;
        break;

      case Chunk::SECTOR:
        return cfg.sectorSize;
        break;

      default:
        return 0;
        break;
    }
  }
}  // namespace Aurora::Memory
