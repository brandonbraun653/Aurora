/******************************************************************************
 *  File Name:
 *    database_config.hpp
 *
 *  Description:
 *    Compile and runtime database config options
 *
 *  2021-2022 | Brandon Braun | brandonbraun653@gmail.com
 *****************************************************************************/

#pragma once
#ifndef AURORA_DATABASE_CONFIG_HPP
#define AURORA_DATABASE_CONFIG_HPP

#if defined( AURORA_DATABASE_USER_CONFIG)
#include "aurora_database_user_config.hpp"
#else

/* STL Includes */
#include <cstddef>
#include <cstdint>

namespace Aurora::Database::Volatile
{

  static constexpr size_t MAX_FILE_NAME_LENGTH = 32;  /**< Maximum characters, including null termination */
  static constexpr size_t MAX_RAM_ENTRIES    = 32;  /**< Number of entries allowed in RAM database */

}  // namespace Aurora::Database

#endif  /* AURORA_DATABASE_USER_CONFIG */
#endif  /* !AURORA_DATABASE_CONFIG_HPP */
