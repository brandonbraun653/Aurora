/********************************************************************************
 *  File Name:
 *    hmi_button_driver.hpp
 *
 *  Description:
 *    Human machine interface driver for a GPIO based button
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_HMI_GPIO_BUTTON_HPP
#define AURORA_HMI_GPIO_BUTTON_HPP

/* Chimera Includes */
#include <Chimera/function>
#include <Chimera/gpio>

namespace Aurora::HMI::Button
{
  /*-------------------------------------------------------------------------------
  Enumerations
  -------------------------------------------------------------------------------*/
  enum class ActiveState : uint8_t
  {
    ACTIVE_LOW,   /**< Signal active on stable logic low */
    RISING_EDGE,  /**< Signal active on rising edge triggers */
    ACTIVE_HIGH,  /**< Signal active on stable logic high */
    FALLING_EDGE, /**< Signal active on falling edge triggers */

    NUM_OPTIONS,
    UNKNOWN
  };

  /*-------------------------------------------------------------------------------
  Structures
  -------------------------------------------------------------------------------*/
  struct Config
  {
    Chimera::GPIO::PinInit pinConfig;
    ActiveState activeState;
    size_t debounceTime;
  };

  /*-------------------------------------------------------------------------------
  Classes
  -------------------------------------------------------------------------------*/
  class Driver
  {
  public:
    Driver();
    ~Driver();

    /**
     *
     */
    bool initialize( const Config &cfg );

    /**
     *
     */
    void enable();

    /**
     *
     */
    void disable();

    /**
     *
     */
    void onTrigger( Chimera::Function::Opaque &callback );

  private:
    Chimera::Function::Opaque mTrigCallback;
    Chimera::GPIO::Driver_sPtr mPin;
  };
}  // namespace Aurora::HMI::Button

#endif  /* !AURORA_HMI_GPIO_BUTTON_HPP */
