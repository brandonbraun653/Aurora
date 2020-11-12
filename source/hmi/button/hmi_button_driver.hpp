/********************************************************************************
 *  File Name:
 *    hmi_button_driver.hpp
 *
 *  Description:
 *    Human machine interface driver for a GPIO based button. The purpose of this
 *    driver is to provide clean event signals for when a button has been pressed
 *    or released. Any higher level functionalities like hold behavior or system
 *    level notification is left to another library.
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_HMI_GPIO_BUTTON_HPP
#define AURORA_HMI_GPIO_BUTTON_HPP

/* Chimera Includes */
#include <Chimera/gpio>
#include <Chimera/thread>

/* Aurora Includes */
#include <Aurora/source/hmi/button/hmi_button_types.hpp>

namespace Aurora::HMI::Button
{
  /*-------------------------------------------------------------------------------
  Classes
  -------------------------------------------------------------------------------*/
  class EdgeTrigger : public Chimera::Threading::Lockable
  {
  public:
    EdgeTrigger();
    ~EdgeTrigger();

    /**
     *  Initializes the system and hardware for processing GPIO edge events.
     *
     *  @param[in]  cfg     The configuration to be applied
     *  @return bool
     */
    bool initialize( const EdgeConfig &cfg );

    /**
     *  Resets the driver to the same state as if 'initialize' was just called.
     *  This will keep any existing hardware settings, but disables the GPIO
     *  listener processing and removes any registered callbacks.
     *
     *  @return void
     */
    void reset();

    /**
     *  Enables listening to the GPIO pin for the active edge transition. Must
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
     *  edge event.
     *
     *  @note The call to this function will occur in thread mode, so use
     *  whatever systems resources are needed. There are no ISR constraints.
     *
     *  @param[in]  callback        The function to be executed
     *  @return void
     */
    void onActiveEdge( EdgeCallback callback );

    /**
     *  How many edge events have occurred since the last time this function
     *  was called. Upon calling, will clear the recorded number of events.
     *
     *  @return size_t
     */
    size_t numEdgeEvents();

    /**
     *  Gets the currently configured active edge state
     *  @return ActiveEdge
     */
    ActiveEdge getActiveEdge();


  private:
    EdgeCallback mCallback;    /**< User callback when the configured edge fires */
    EdgeConfig mConfig;        /**< Cached configuration settings */
    size_t mNumEvents;         /**< How many edge events pending being processed */
    size_t mDebounced;         /**< Internal filter for tracking GPIO state on each sample */
    size_t mMaxNumSamples;     /**< How many samples are needed for detecting a debunced edge */
    size_t mCurrentNumSamples; /**< Number of samples currently processed */

    /**
     *  Callback used for the EXTI drivers. This handles simple edge
     *  detection behaviors
     *
     *  @param[in]  arg       Unused
     *  @return void
     */
    void gpioEdgeTriggerCallback( void *arg );

    /**
     *  Callback used for Scheduler to periodically sample the GPIO
     *  state to see when it becomes stable.
     *
     *  @return void
     */
    void gpioEdgeSamplerCallback();
  };
}  // namespace Aurora::HMI::Button

#endif /* !AURORA_HMI_GPIO_BUTTON_HPP */
