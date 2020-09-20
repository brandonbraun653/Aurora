/********************************************************************************
 *  File Name:
 *    lfs_hooks.hpp
 *
 *  Description:
 *    Hooks for integration of a block device with LittleFS
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef LFS_HOOKS_HPP
#define LFS_HOOKS_HPP

/* LFS Includes */
#include "lfs.h"

/* Chimera Includes */
#include <Aurora/memory/virtual/blockDevice.hpp>

int lfs_safe_read( const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);
int lfs_safe_prog( const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
int lfs_safe_erase( const struct lfs_config *c, lfs_block_t block );
int lfs_safe_sync( const struct lfs_config *c );

namespace Aurora::Memory::LFS
{
  void initDevice();
}

#endif  /* !LFS_HOOKS_HPP */
