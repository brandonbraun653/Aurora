/********************************************************************************
 *  File Name:
 *    hmi_rotatry_encoder_types.hpp
 *
 *  Description:
 *    Types associated with the HMI rotary encoder driver
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_HMI_ROTARY_ENCODER_TYPES_HPP
#define AURORA_HMI_ROTARY_ENCODER_TYPES_HPP

/* STL Includes */
#include <cstddef>

/* Aurora Includes */
#include <Aurora/source/hmi/button/hmi_button_types.hpp>

/* Chimera Includes */
#include <Chimera/function>
#include <Chimera/gpio>
#include <Chimera/thread>

namespace Aurora::HMI::RotaryEncoder
{
  /*-------------------------------------------------------------------------------
  Aliases
  -------------------------------------------------------------------------------*/
  using RotationCallback = etl::delegate<void( int )>;

  /*-------------------------------------------------------------------------------
  Structures
  -------------------------------------------------------------------------------*/
  struct REConfig
  {
    bool idleHigh;                        /**< If true, indicates the encoder idles at logic high */
    Chimera::GPIO::PinInit pinACfg;       /**< Encoder pin A gpio configuration */
    Chimera::GPIO::PinInit pinBCfg;       /**< Encoder pin B gpio configuration */
    Chimera::GPIO::PinInit pinPushCfg;    /**< Optional config if the encoder also has a center button */
    Button::ActiveEdge pinPushActiveEdge; /**< Which edge to trigger the center button on */
    Button::ActiveEdge pinAActiveEdge;    /**< Which edge to trigger the encoder processing on */
    size_t debounceTime;                  /**< Max sample time before GPIO is considered debounced (mS) */
    size_t sampleRate;                    /**< Period at which to sample GPIO logic state (mS) */
    size_t stableSamples;                 /**< Number of samples at which to consider GPIO state stable */
  };

}  // namespace Aurora::HMI::RotaryEncoder

#endif /* !AURORA_HMI_ROTARY_ENCODER_TYPES_HPP */
