/********************************************************************************
 *  File Name:
 *    hmi_button_types.hpp
 *
 *  Description:
 *    Types associated with the HMI button driver
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_HMI_BUTTON_TYPES_HPP
#define AURORA_HMI_BUTTON_TYPES_HPP

/* STL Includes */
#include <cstddef>

/* Chimera Includes */
#include <Chimera/function>
#include <Chimera/gpio>

namespace Aurora::HMI::Button
{
  /*-------------------------------------------------------------------------------
  Enumerations
  -------------------------------------------------------------------------------*/
  enum class ActiveEdge : uint8_t
  {
    RISING_EDGE,  /**< Signal active on rising edge triggers */
    FALLING_EDGE, /**< Signal active on falling edge triggers */
    BOTH_EDGES,   /**< Signal active on both edges */

    NUM_OPTIONS,
    UNKNOWN
  };

  /*-------------------------------------------------------------------------------
  Aliases
  -------------------------------------------------------------------------------*/
  using EdgeCallback = etl::delegate<void( ActiveEdge )>;

  /*-------------------------------------------------------------------------------
  Structures
  -------------------------------------------------------------------------------*/
  struct EdgeConfig
  {
    Chimera::GPIO::PinInit gpioConfig; /**< GPIO hardware configuration */
    ActiveEdge activeEdge;             /**< Which edge to trigger on */
    size_t debounceTime;               /**< Max sample time before GPIO is considered debounced (mS) */
    size_t sampleRate;                 /**< Period at which to sample GPIO logic state (mS) */
    size_t stableSamples;              /**< Number of samples at which to consider GPIO state stable */
  };
}  // namespace Aurora::HMI::Button

#endif /* !AURORA_HMI_BUTTON_TYPES_HPP */
