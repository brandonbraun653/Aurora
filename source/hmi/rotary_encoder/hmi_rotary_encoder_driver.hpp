/********************************************************************************
 *  File Name:
 *    hmi_rotary_encoder_driver.hpp
 *
 *  Description:
 *    Human machine interface driver for a GPIO based rotary encoder
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_HMI_ROTARY_ENCODER_HPP
#define AURORA_HMI_ROTARY_ENCODER_HPP

/* Chimera Includes */
#include <Chimera/function>
#include <Chimera/gpio>
#include <Chimera/thread>

/* Aurora Includes */
#include <Aurora/source/hmi/button/hmi_button_types.hpp>
#include <Aurora/source/hmi/rotary_encoder/hmi_rotatry_encoder_types.hpp>

namespace Aurora::HMI::RotaryEncoder
{
  /*-------------------------------------------------------------------------------
  Classes
  -------------------------------------------------------------------------------*/
  class Encoder : public Chimera::Threading::Lockable<Encoder>
  {
  public:
    Encoder();
    ~Encoder();

    /**
     *  Initializes the system and hardware for processing rotational and GPIO edge events.
     *
     *  @param[in]  cfg     The configuration to be applied
     *  @return bool
     */
    bool initialize( const REConfig &cfg );

    /**
     *  Resets the driver to the same state as if 'initialize' was just called.
     *  This will keep any existing hardware settings, but disables the GPIO
     *  listener processing and removes any registered callbacks.
     *
     *  @return void
     */
    void reset();

    /**
     *  Enables listening to the GPIO pins for the active edge transition. Must
     *  be called after initializing the driver in order for processing to work.
     *
     *  @return void
     */
    void enable();

    /**
     *  Disables the GPIO listeners that handle edge event processing. This
     *  does not destroy any configurations. Re-enable listening by calling
     *  'enable()'.
     *
     *  @return void
     */
    void disable();

    /**
     *  Registers some function to perform upon a successfully debounced
     *  rotation event.
     *
     *  @note The call to this function will occur in thread mode, so use
     *  whatever systems resources are needed. There are no ISR constraints.
     *
     *  @param[in]  callback        The function to be executed
     *  @return void
     */
    void onRotation( RotationCallback callback );

    /**
     *  If the encoder supports a center push button as well, this registers
     *  some function to perform upon a successfully debounced edge event.
     *
     *  @note The call to this function will occur in thread mode, so use
     *  whatever systems resources are needed. There are no ISR constraints.
     *
     *  @param[in]  callback        The function to be executed
     *  @return void
     */
    void onCenterPush( HMI::Button::EdgeCallback callback );

    /**
     *  How many rotation events have occurred since the last time this function
     *  was called. Upon calling, will clear the recorded number of events.
     *
     *  @note Rotations can be positive or negative, so this will return the
     *  cumulative effect of a rotation sequence. For example, if the user
     *  rotates -5 clicks, then +3 clicks, the returned result will be -2.
     *
     *  @return int
     */
    int numRotateEvents();

    /**
     *  How many edge events have occurred since the last time this function
     *  was called. Upon calling, will clear the recorded number of events.
     *
     *  @return size_t
     */
    size_t numPushEvents();

    /**
     *  Gets the currently configured active edge state for the center push
     *  button, but only if it's configured.
     *
     *  @return ActiveEdge
     */
    HMI::Button::ActiveEdge getPushActiveEdge();

  private:
    friend Chimera::Threading::Lockable<Encoder>;


    /*-------------------------------------------------
    Center Push Button Resources
    -------------------------------------------------*/
    HMI::Button::EdgeTrigger mCenterButton; /**< Debouncing push button class */

    /*-------------------------------------------------
    Encoder Attributes
    -------------------------------------------------*/
    REConfig mConfig;                 /**< Cached configuration settings */
    RotationCallback mRotateCallback; /**< User callback when a rotation event happens */
    int mNumRotateEvents;             /**< Number of rotation events */

    Chimera::GPIO::State a0;
    Chimera::GPIO::State b0;

    /**
     *  Internal callback for pulling out rotational information from a series
     *  of encoder pulses.
     *
     *  @param[in]  arg       Unused
     *  @return void
     */
    void processRotateEventCallback( void *arg );
  };

}  // namespace Aurora::HMI::RotaryEncoder

#endif /* !AURORA_HMI_ROTARY_ENCODER_HPP */
