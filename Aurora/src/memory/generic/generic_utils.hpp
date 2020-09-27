/********************************************************************************
 *  File Name:
 *    generic_utils.hpp
 *
 *  Description:
 *    Utility functions for interacting with memory devices
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_GENERIC_UTILS_HPP
#define AURORA_GENERIC_UTILS_HPP

/* STL Includes */
#include <cstddef>
#include <limits>

/* Aurora Includes */
#include <Aurora/src/memory/generic/generic_types.hpp>

namespace Aurora::Memory
{
  /**
   *  Validates the data fields of a memory property descriptor.
   *
   *  @param[in]  cfg       The properties to be validated
   *  @return bool
   */
  bool validateProperties( const Properties &cfg );

  /**
   *  Calculates the chunk index of the given address
   *
   *  In the example below, this function will return the chunk number
   *  associated with 'a' because this is the chunk that the address
   *  currently resides.
   *
   *  ================== Example ==================
   *  Data Range: ***   StartAddr: p1   EndAddr: p2
   *  Don't Care: ---   ChunkBoundaries: a,b,c,d,e
   *
   *  a    p1    b          c          d   p2     e
   *  |-----*****|**********|**********|****------|
   *
   *  @param[in]  cfg       The device configuration data
   *  @param[in]  view      How to view the memory (page, block, etc)
   *  @param[in]  address   Which address to calculate the chunk idx on
   *  @return ChunkIndex_t
   */
  ChunkIndex_t chunkIndex( const Properties &cfg, const Chunk view, const SysAddress_t address );

  /**
   *  Calculates offset of the given address from the beginning of its chunk. Note
   *  that the chunk could be variable sizes.
   *
   *  In the example below, it would return the difference between
   *  the address of 'p1' and the address of 'a'. Mathematically this is:
   *
   *  startOffset = p1 - a;
   *
   *  ================== Example ==================
   *  Data Range: ***   StartAddr: p1   EndAddr: p2
   *  Don't Care: ---   chunkBoundaries: a,b,c,d,e
   *
   *  a    p1    b          c          d   p2     e
   *  |-----*****|**********|**********|****------|
   *
   *  @param[in]  cfg       The device configuration data
   *  @param[in]  view      How to view the memory (page, block, etc)
   *  @param[in]  address   Which address to calculate the offset on
   *  @return size_t
   */
  size_t startOffset( const Properties &cfg, const Chunk view, const SysAddress_t address );

  /**
   *  Starting from the given address, calculates the number of bytes until
   *  the next chunk boundary point has been reached.
   *
   *  In the example below, it would return the difference between
   *  the address of 'b' and the address of 'p1'. Mathematically this is:
   *
   *  endOffset = b - p1;
   *
   *  ================== Example ==================
   *  Data Range: ***   StartAddr: p1   EndAddr: p2
   *  Don't Care: ---   chunkBoundaries: a,b,c,d,e
   *
   *  a    p1    b          c          d   p2     e
   *  |-----*****|**********|**********|****------|
   *
   *  @param[in]  cfg       The device configuration data
   *  @param[in]  view      How to view the memory (page, block, etc)
   *  @param[in]  address   Which address to calculate the offset on
   *  @return size_t
   */
  size_t endOffset( const Properties &cfg, const Chunk view, const SysAddress_t address );

  /**
   *  Gets the starting address of a given chunk
   *
   *  @param[in]  cfg       The device configuration data
   *  @param[in]  view      How to view the memory (page, block, etc)
   *  @param[in]  idx       Which index to calculate the offset on
   *  @return SysAddress_t
   */
  SysAddress_t chunkStartAddress( const Properties &cfg, const Chunk view, const ChunkIndex_t idx );

  /**
   *  Gets the ending address of a given chunk
   *
   *  @param[in]  cfg       The device configuration data
   *  @param[in]  view      How to view the memory (page, block, etc)
   *  @param[in]  idx       Which index to calculate the offset on
   *  @return SysAddress_t
   */
  SysAddress_t chunkEndAddress( const Properties &cfg, const Chunk view, const ChunkIndex_t idx );

  /**
   *  Gets the size of the chunk as specified in the device properties
   *
   *  @param[in]  cfg       The device configuration data
   *  @param[in]  view      Which chunk to get
   *  @return size_t
   */
  size_t chunkSize( const Properties &cfg, const Chunk view );

}  // namespace Aurora::Memory

#endif /* !AURORA_GENERIC_UTILS_HPP */
