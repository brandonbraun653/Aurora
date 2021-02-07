/********************************************************************************
 *  File Name:
 *    file_config.hpp
 *
 *  Description:
 *    Configuration options for runtime behavior of the filesystem
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_FILE_SYSTEM_CONFIG_HPP
#define AURORA_FILE_SYSTEM_CONFIG_HPP

/* STL Includes */
#include <cstddef>

/* Aurora Includes */
#include <Aurora/source/filesystem/file_types.hpp>

#if defined( AURORA_USER_FILESYSTEM_CONFIG )
#include "aurora_user_filesystem_config.hpp"
#else

/*-------------------------------------------------------------------------------
Configuration Constants: These need to be duplicated in a user configuration file
-------------------------------------------------------------------------------*/
/**
 *  What the default file system implemenation will hook into.
 */
#define DEFAULT_FILESYSTEM    ( DRIVER_OS )

#endif /* AURORA_USER_FILESYSTEM_CONFIG */

#endif /* !AURORA_FILE_SYSTEM_CONFIG_HPP */
