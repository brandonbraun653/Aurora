/********************************************************************************
 *  File Name:
 *    logging_config.hpp
 *
 *  Description:
 *    Configuration options for the logger system
 *
 *  2019-2021 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#pragma once
#ifndef MICRO_LOGGER_CONFIGURATION_HPP
#define MICRO_LOGGER_CONFIGURATION_HPP

/**
 *  The max number of sinks that are allowed to be registered with
 *  the backend. This will directly impact the memory usage and log
 *  performance.
 */
#define ULOG_MAX_REGISTERABLE_SINKS ( 10u )

/**
 *  Defines the max number of characters used for an internal buffer to
 *  format messages into.
 */
#define ULOG_MAX_SNPRINTF_BUFFER_LENGTH ( 256u )

#endif /* MICRO_LOGGER_CONFIGURATION_HPP */
