/******************************************************************************
 *  File Name:
 *    gyro_intf.hpp
 *
 *  Description:
 *    Describes an interface for a generic gyroscope
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *****************************************************************************/

#pragma once
#ifndef VIRTUAL_SENSOR_GYROSCOPE_HPP
#define VIRTUAL_SENSOR_GYROSCOPE_HPP

/* Project Includes */
#include <Aurora/source/sensors/sensor_intf.hpp>
#include <Aurora/source/sensors/sensor_types.hpp>

namespace Aurora::Sensor::Gyroscope
{
  /*---------------------------------------------------------------------------
  Enumerations
  ---------------------------------------------------------------------------*/
  enum class Range : uint8_t
  {

    NUM_OPTIONS
  };

  /*---------------------------------------------------------------------------
  Classes
  ---------------------------------------------------------------------------*/
  /**
   * @brief Virtual accelerometer interface
   */
  class IGyro : public IBase
  {
  public:
    virtual ~IGyro() = default;

    /**
     * @brief Set the device measurement range
     *
     * @param range     The range to be set
     * @return true     Range was set ok
     * @return false    Range is not supported
     */
    virtual bool setGyroRange( const Range range ) = 0;
  };
}  // namespace Aurora::Sensor::Accelerometer

#endif /* !VIRTUAL_SENSOR_GYROSCOPE_HPP */
