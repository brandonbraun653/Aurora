/********************************************************************************
 *  File Name:
 *    generic_utils.hpp
 *
 *  Description:
 *    Utility functions and classes for interacting with memory devices
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_GENERIC_UTILS_HPP
#define AURORA_GENERIC_UTILS_HPP

/* STL Includes */
#include <cstddef>
#include <limits>

namespace Aurora::Memory
{
  /**
   *  A helper class that generates descriptive information about a range of memory addresses.
   *  Intended to help lessen repetetive calculations in low level operations.
   */
  class BlockRange
  {
  public:
    /**
     *	Simple constructor to generate a new class from scratch
     *
     *	@param[in]	startAddress    Absolute address to start at
     *	@param[in]	endAddress      Absolute address greater than startAddress to end at
     *	@param[in]	blockSize       The block size in bytes
     */
    BlockRange( const size_t startAddress, const size_t endAddress, const size_t blockSize );

    /**
     *	Copy constructor to generate a new class from an existing one
     *
     *	@param[in]	cls             The existing class object
     */
    BlockRange( const BlockRange &cls );

    ~BlockRange() = default;

    /**
     *  Calculates the block of the startAddress
     *
     *  In the example below, this function will return the block number
     *  associated with 'a' because this is the block that the startAddress
     *  currently resides.
     *
     *  ================== Example ==================
     *  Data Range: ***   StartAddr: p1   EndAddr: p2
     *  Don't Care: ---   BlockBoundaries: a,b,c,d,e
     *
     *  a    p1    b          c          d   p2     e
     *  |-----*****|**********|**********|****------|
     *
     *	@return uint32_t
     *
     *	|             Return Value             |                             Explanation                             |
     *  |:------------------------------------:|:-------------------------------------------------------------------:|
     *  | std::numeric_limits<uint32_t>::max() | The class was not initialized with valid address range / block size |
     *  |                     All other values | The calculated data                                                 |
     */
    size_t startBlock();

    /**
     *  Starting from the beginning of startblock(), calculates the number
     *  of bytes until the startAddress has been reached (point 'p1').
     *
     *  In the example below, it would return the difference between
     *  the address of 'p1' and the address of 'a'. Mathematically this is:
     *
     *  startOffset = p1 - a;
     *
     *  ================== Example ==================
     *  Data Range: ***   StartAddr: p1   EndAddr: p2
     *  Don't Care: ---   blockBoundaries: a,b,c,d,e
     *
     *  a    p1    b          c          d   p2     e
     *  |-----*****|**********|**********|****------|
     *
     *	@return uint32_t
     *
     *	|             Return Value             |                             Explanation                             |
     *  |:------------------------------------:|:-------------------------------------------------------------------:|
     *  | std::numeric_limits<uint32_t>::max() | The class was not initialized with valid address range / block size |
     *  |                     All other values | The calculated data                                                 |
     */
    size_t startOffset();

    /**
     *  Starting from startAddress, calculates the number of bytes until the next block
     *  boundary point has been reached.
     *
     *  In the example below, it would return the difference between
     *  the address of 'b' and the address of 'p1'. Mathematically this is:
     *
     *  startBytes = b - p1;
     *
     *  ================== Example ==================
     *  Data Range: ***   StartAddr: p1   EndAddr: p2
     *  Don't Care: ---   blockBoundaries: a,b,c,d,e
     *
     *  a    p1    b          c          d   p2     e
     *  |-----*****|**********|**********|****------|
     *
     *	@return uint32_t
     *
     *	|             Return Value             |                             Explanation                             |
     *  |:------------------------------------:|:-------------------------------------------------------------------:|
     *  | std::numeric_limits<uint32_t>::max() | The class was not initialized with valid address range / block size |
     *  |                     All other values | The calculated data                                                 |
     */
    size_t startBytes();

    /**
     *  Calculates the block of the endAddress
     *
     *  In the example below, this function will return the block number
     *  associated with 'd' because this is the block that the endAddress
     *  currently resides.
     *
     *  ================== Example ==================
     *  Data Range: ***   StartAddr: p1   EndAddr: p2
     *  Don't Care: ---   blockBoundaries: a,b,c,d,e
     *
     *  a    p1    b          c          d   p2     e
     *  |-----*****|**********|**********|****------|
     *
     *	@return uint32_t
     *
     *	|             Return Value             |                             Explanation                             |
     *  |:------------------------------------:|:-------------------------------------------------------------------:|
     *  | std::numeric_limits<uint32_t>::max() | The class was not initialized with valid address range / block size |
     *  |                     All other values | The calculated data                                                 |
     */
    size_t endBlock();

    /**
     *  Starting from the beginning of endblock(), calculates the number
     *  of bytes until the endAddress has been reached (point 'p2').
     *
     *  In the example below, it would return the difference between
     *  the address of 'p2' and the address of 'd'. Mathematically this is:
     *
     *  endOffset = p2 - d;
     *
     *  ================== Example ==================
     *  Data Range: ***   StartAddr: p1   EndAddr: p2
     *  Don't Care: ---   blockBoundaries: a,b,c,d,e
     *
     *  a    p1    b          c          d   p2     e
     *  |-----*****|**********|**********|****------|
     *
     *	@return uint32_t
     *
     *	|             Return Value             |                             Explanation                             |
     *  |:------------------------------------:|:-------------------------------------------------------------------:|
     *  | std::numeric_limits<uint32_t>::max() | The class was not initialized with valid address range / block size |
     *  |                     All other values | The calculated data                                                 |
     */
    size_t endOffset();

    /**
     *  Starting from endAddress, calculates the number of bytes until the next block
     *  boundary point has been reached.
     *
     *  In the example below, it would return the difference between
     *  the address of 'e' and the address of 'p2'. Mathematically this is:
     *
     *  startBytes = e - p2;
     *
     *  ================== Example ==================
     *  Data Range: ***   StartAddr: p1   EndAddr: p2
     *  Don't Care: ---   blockBoundaries: a,b,c,d,e
     *
     *  a    p1    b          c          d   p2     e
     *  |-----*****|**********|**********|****------|
     *
     *	@return uint32_t
     *
     *	|             Return Value             |                             Explanation                             |
     *  |:------------------------------------:|:-------------------------------------------------------------------:|
     *  | std::numeric_limits<uint32_t>::max() | The class was not initialized with valid address range / block size |
     *  |                     All other values | The calculated data                                                 |
     */
    size_t endBytes();

  protected:
    static constexpr size_t SIZE_T_MAX = std::numeric_limits<size_t>::max();

    bool initialized = false;

    size_t _blockSize    = SIZE_T_MAX;
    size_t _startBlock   = SIZE_T_MAX;
    size_t _startAddress = SIZE_T_MAX;
    size_t _startOffset  = SIZE_T_MAX;
    size_t _startBytes   = SIZE_T_MAX;
    size_t _endBlock     = SIZE_T_MAX;
    size_t _endAddress   = SIZE_T_MAX;
    size_t _endOffset    = SIZE_T_MAX;
    size_t _endBytes     = SIZE_T_MAX;
  };

}  // namespace Aurora::Memory

#endif  /* !AURORA_GENERIC_UTILS_HPP */
