/******************************************************************************
 *  File Name:
 *    lfs_tests.hpp
 *
 *  Description:
 *    Tests ported from the LFS library
 *
 *  2022 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef AURORA_FILESYSTEM_LFS_TESTS_HPP
#define AURORA_FILESYSTEM_LFS_TESTS_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/source/filesystem/littlefs/lfs_driver.hpp>

namespace Aurora::FileSystem::LFS::Test
{
  namespace Alloc
  {
    int parallel_allocation( lfs_t &lfs, const struct lfs_config &cfg );
  }
}  // namespace Aurora::FileSystem::LFS::Test

#endif /* !AURORA_FILESYSTEM_LFS_TESTS_HPP */
