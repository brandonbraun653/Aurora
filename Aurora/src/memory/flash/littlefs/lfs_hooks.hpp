/********************************************************************************
 *  File Name:
 *    lfs_hooks.hpp
 *
 *  Description:
 *    Hooks for integration of a generic memory device with LittleFS
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef LFS_HOOKS_HPP
#define LFS_HOOKS_HPP

/* LFS Includes */
#include "lfs.h"

/* Aurora Includes */
#include <Aurora/memory/generic/generic_intf.hpp>

/*-------------------------------------------------------------------------------
Public Functions
-------------------------------------------------------------------------------*/
int lfs_safe_read( const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);
int lfs_safe_prog( const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
int lfs_safe_erase( const struct lfs_config *c, lfs_block_t block );
int lfs_safe_sync( const struct lfs_config *c );

namespace Aurora::Memory::LFS
{
  /**
   *  Attaches a generic memory device to the opaque pointer contained in the
   *  LittleFS configuration structure. This allows the read/write/erase hooks
   *  to act on the proper device at runtime.
   *
   *  @param[in]  dev     The device to attach
   *  @param[in]  cfg     LittleFS configuration structure
   *  @return bool
   */
  bool attachDevice( IGenericDevice_sPtr& dev, lfs_config &cfg );
}

#endif  /* !LFS_HOOKS_HPP */
