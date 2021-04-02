/********************************************************************************
 *  File Name:
 *    accel_intf.hpp
 *
 *  Description:
 *    Describes an interface for a generic accelerometer
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef VIRTUAL_SENSOR_ACCELEROMETER_HPP
#define VIRTUAL_SENSOR_ACCELEROMETER_HPP

/* Project Includes */
#include <Aurora/source/sensors/sensor_intf.hpp>
#include <Aurora/source/sensors/sensor_types.hpp>

namespace Aurora::Sensor::Accelerometer
{
  /*-------------------------------------------------------------------------------
  Enumerations
  -------------------------------------------------------------------------------*/
  enum class Range : uint8_t
  {
    ACCEL_1G,
    ACCEL_2G,
    ACCEL_4G,
    ACCEL_6G,
    ACCEL_8G,
    ACCEL_16G,

    NUM_OPTIONS
  };

  /*-------------------------------------------------------------------------------
  Classes
  -------------------------------------------------------------------------------*/
  /**
   * @brief Virtual accelerometer interface
   */
  class IAccel : public IBase
  {
  public:
    virtual ~IAccel() = default;

    /**
     * @brief Set the device measurement range in Gs
     *
     * @param range     The range to be set
     * @return true     Range was set ok
     * @return false    Range is not supported
     */
    virtual bool setAccelRange( const Range range ) = 0;
  };
}  // namespace Aurora::Sensor::Accelerometer

#endif /* !VIRTUAL_SENSOR_ACCELEROMETER_HPP */
