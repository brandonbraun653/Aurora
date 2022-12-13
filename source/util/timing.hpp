/******************************************************************************
 *  File Name:
 *    timing.hpp
 *
 *  Description:
 *    Helper utilities for timing
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *****************************************************************************/

#pragma once
#ifndef AURORA_TIMING_UTILITIES_HPP
#define AURORA_TIMING_UTILITIES_HPP

/* STL Includes */
#include <cstddef>

namespace Aurora::Utility
{
  /*---------------------------------------------------------------------------
  Classes
  ---------------------------------------------------------------------------*/
  /**
   * @brief Tracks elapsed time to know when a timeout has occurred.
   *
   * This class is useful for periodic events that need to run only
   * once a timeout period has expired. It will track the system time
   * and provides an interface to query if the timeout has expired.
   */
  class PeriodicTimeout
  {
  public:
    /**
     * @brief Default construct a new Periodic Timeout object
     */
    PeriodicTimeout();

    /**
     * @brief Construct a new Periodic Timeout object
     *
     * @param period    How often the timeout should expire in milliseconds
     * @param initial   Initial value of the timer (ms)
     */
    PeriodicTimeout( const size_t period, const size_t initial );
    ~PeriodicTimeout();

    /**
     * @brief Sets the timeout period
     *
     * @param period    Timeout period in milliseconds
     */
    void setTimeout( const size_t period );

    /**
     * @brief Has the periodic timeout expired yet?
     *
     * @return true     Timeout has expired
     * @return false    Timeout has not expired
     */
    bool expired();

    /**
     * @brief Refresh the timeout based on the current time
     * Very similar to kicking a watchdog.
     */
    void refresh();

  protected:
    size_t mLast;
    size_t mPeriod;
  };

}  // namespace Aurora::Utility

#endif  /* !AURORA_TIMING_UTILITIES_HPP */
