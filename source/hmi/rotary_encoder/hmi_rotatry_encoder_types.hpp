/********************************************************************************
 *  File Name:
 *    hmi_rotatry_encoder_types.hpp
 *
 *  Description:
 *    Types associated with the HMI rotary encoder driver
 *
 *  2020-2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_HMI_ROTARY_ENCODER_TYPES_HPP
#define AURORA_HMI_ROTARY_ENCODER_TYPES_HPP

/* STL Includes */
#include <cstddef>
#include <cstdint>

/* Aurora Includes */
#include <Aurora/source/hmi/button/hmi_button_types.hpp>

/* Chimera Includes */
#include <Chimera/function>
#include <Chimera/gpio>
#include <Chimera/thread>

namespace Aurora::HMI::Encoder
{
  /*-------------------------------------------------------------------------------
  Structures
  -------------------------------------------------------------------------------*/
  struct Config
  {
    /*-------------------------------------------------
    Encoder Settings
    -------------------------------------------------*/
    bool encIdleHigh;                 /**< If true, indicates the encoder idles at logic high */
    Button::ActiveEdge encActiveEdge; /**< Which edge to trigger the encoder processing on */
    Chimera::GPIO::PinInit encACfg;   /**< Encoder pin A gpio configuration */
    Chimera::GPIO::PinInit encBCfg;   /**< Encoder pin B gpio configuration */

    /*-------------------------------------------------
    Center Button Settings
    -------------------------------------------------*/
    Chimera::GPIO::PinInit btnCfg;    /**< Optional config if the encoder also has a center button */
    Button::ActiveEdge btnActiveEdge; /**< Which edge to trigger the center button on */
    size_t btnDebounceTime;           /**< Max sample time before GPIO is considered debounced (mS) */
    size_t btnSampleRate;             /**< Period at which to sample GPIO logic state (mS) */
    size_t btnNumSamples;             /**< Number of samples at which to consider GPIO state stable */
  };

  /**
   *  Tracks the runtime state of the encoder
   */
  struct State
  {
    int absolutePosition;    /**< Absolute position of the encoder */
    int diffPosition;        /**< Relative encoder position since last sample */
    size_t diffCenterClicks; /**< How many center clicks ocurred since last sample */

    void clear()
    {
      diffCenterClicks = 0;
      diffPosition     = 0;
      absolutePosition = 0;
    }

    void clearAccumulated()
    {
      diffCenterClicks = 0;
      diffPosition     = 0;
    }
  };

  /*-------------------------------------------------------------------------------
  Aliases
  -------------------------------------------------------------------------------*/
  using RotationCallback = etl::delegate<void( State & )>;

}  // namespace Aurora::HMI::Encoder

#endif /* !AURORA_HMI_ROTARY_ENCODER_TYPES_HPP */
