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
#include <Chimera/function>
#include <Chimera/gpio>
#include <Chimera/thread>

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

  /*-------------------------------------------------------------------------------
  Classes
  -------------------------------------------------------------------------------*/
  class EdgeTrigger
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
    EdgeCallback mCallback;
    EdgeConfig mConfig;
    size_t mNumEvents;
    size_t mDebounced;
    Chimera::Threading::RecursiveMutex mLock;

    void gpioEdgeTriggerCallback( void *arg );
    void gpioEdgeSamplerCallback();
  };
}  // namespace Aurora::HMI::Button

#endif /* !AURORA_HMI_GPIO_BUTTON_HPP */
