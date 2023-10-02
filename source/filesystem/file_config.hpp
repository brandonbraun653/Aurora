/******************************************************************************
 *  File Name:
 *    file_config.hpp
 *
 *  Description:
 *    Configuration options for runtime behavior of the filesystem
 *
 *  2021-2023 | Brandon Braun | brandonbraun653@gmail.com
 *****************************************************************************/

#pragma once
#ifndef AURORA_FILE_SYSTEM_CONFIG_HPP
#define AURORA_FILE_SYSTEM_CONFIG_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <cstddef>
#include <Aurora/source/filesystem/file_types.hpp>

#if defined( AURORA_USER_FILESYSTEM_CONFIG )
#include "aurora_user_filesystem_config.hpp"
#endif

#endif /* !AURORA_FILE_SYSTEM_CONFIG_HPP */
