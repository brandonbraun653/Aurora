/********************************************************************************
 *  File Name:
 *    file_types.hpp
 *
 *  Description:
 *    File system types
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_FILE_SYSTEM_TYPES_HPP
#define AURORA_FILE_SYSTEM_TYPES_HPP

/* STL Includes */
#include <cstddef>

namespace Aurora::FileSystem
{
  /*-------------------------------------------------------------------------------
  Enumerations
  -------------------------------------------------------------------------------*/
  enum BackendType : size_t
  {
    DRIVER_LITTLE_FS, /**< Little FS embedded driver */
    DRIVER_YAFFS2,    /**< Yet-Another-FAT-File-System (YAFFS) */
    DRIVER_OS,        /**< Compiled OS target file system, aka C++17 filesystem library */

    DRIVER_NUM_OPTIONS,
    DRIVER_UNKNOWN
  };
}  // namespace Aurora::FileSystem

#endif  /* !AURORA_FILE_SYSTEM_TYPES_HPP */
