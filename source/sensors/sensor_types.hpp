/********************************************************************************
 *  File Name:
 *    sensorypes.hpp
 *
 *  Description:
 *    High level types to support all generic sensor classes
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef SENSOR_INTERFACE_TYPES_HPP
#define SENSOR_INTERFACE_TYPES_HPP

/* STL Includes */
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <limits>
#include <type_traits>

/* ETL Includes */
#include <etl/string.h>

namespace Aurora::Sensor
{
  /*-------------------------------------------------------------------------------
  Enumerations
  -------------------------------------------------------------------------------*/
  /**
   * @brief What type of sensors are supported
   */
  enum class Variant : uint8_t
  {
    ACCELEROMETER,
    GYROSCOPE,
    MAGNETOMETER,

    NUM_OPTIONS,
    INVALID
  };

  /**
   * @brief Event types that observers may register callbacks against
   */
  enum Event : size_t
  {
    CB_UNHANDLED,         /**< Required id for default handler */
    CB_ON_SAMPLE_SUCCESS, /**< A sample operation succeeded */
    CB_ON_SAMPLE_FAIL,    /**< A sample operation failed */
    CB_ON_DEVICE_ERROR,   /**< A generic device error occurred */

    CB_NUM_OPTIONS
  };

  /*-------------------------------------------------------------------------------
  Structures
  -------------------------------------------------------------------------------*/
  /**
   * @brief Measurement type for 3-axis sensors
   */
  struct TriAxisSample
  {
    float x; /**< X-Axis sample */
    float y; /**< Y-Axis sample */
    float z; /**< Z-Axis sample */

    /**
     * @brief Construct a new Tri Axis Sample object
     */
    TriAxisSample()
    {
      x = std::numeric_limits<decltype( x )>::max();
      y = std::numeric_limits<decltype( y )>::max();
      z = std::numeric_limits<decltype( z )>::max();
    }
  };

  /**
   * @brief Common sensor attribute description
   */
  struct Details
  {
    etl::string<16> name; /**< Name of the device, usually the P/N */
    uint8_t version;      /**< Software driver version */
    uint8_t uuid;         /**< Unique identifier to distinguish between similar sensors */
    Variant type;         /**< Sensor type */
    float maxValue;       /**< Maximum value the sensor may output */
    float minValue;       /**< Minimum value the sensor may output */
    float resolution;     /**< Smallest delta the sensor can report, in base SI units */
    size_t maxSampleRate; /**< Maximum sampling rate in milliseconds */

    /**
     * @brief Construct a new Details object
     */
    Details()
    {
      name          = "UNKNOWN";
      type          = Variant::INVALID;
      version       = std::numeric_limits<decltype( version )>::max();
      uuid          = std::numeric_limits<decltype( uuid )>::max();
      maxValue      = std::numeric_limits<decltype( maxValue )>::max();
      minValue      = std::numeric_limits<decltype( minValue )>::min();
      resolution    = std::numeric_limits<decltype( resolution )>::min();
      maxSampleRate = std::numeric_limits<decltype( maxSampleRate )>::max();
    }
  };


  /**
   * @brief Core container for a generic sensor sample
   */
  struct Sample
  {
    size_t timestamp; /**< System time the sample was taken */
    Variant type;     /**< Sensor type */
    bool isValid;     /**< Data validity */

    /**
     * @brief Stores raw data for supported sensor types
     * @note  Should only store POD types
     */
    union Conglomerate
    {
      Conglomerate()
      {
        memset( this, 0, sizeof( Conglomerate ) );
      }

      TriAxisSample accel; /**< Accelerometer */
      TriAxisSample gyro;   /**< Gyroscope */
      TriAxisSample mag;     /**< Magnetometer */
    } data;


    /**
     * @brief Construct a new Sample object
     */
    Sample()
    {
      timestamp = std::numeric_limits<decltype( timestamp )>::max();
      type      = Variant::INVALID;
      isValid   = false;
      data      = {};
    }
  };


}  // namespace Aurora::Sensor

#endif /* !SENSOR_INTERFACE_TYPES_HPP */
