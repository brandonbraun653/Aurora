/********************************************************************************
 *  File Name:
 *    yaffs2_driver.hpp
 *
 *  Description:
 *    Filesystem interface to the YAFFS2 driver
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef YAFFS2_HOOKS_HPP
#define YAFFS2_HOOKS_HPP

/* Aurora Includes */
#include <Aurora/source/filesystem/file_types.hpp>

namespace Aurora::FileSystem::YAFFS
{
  /*-------------------------------------------------------------------------------
  Public Data
  -------------------------------------------------------------------------------*/
  extern const Interface implementation;

}  // namespace Aurora::FileSystem::YAFFS

#endif /* !YAFFS2_HOOKS_HPP */
