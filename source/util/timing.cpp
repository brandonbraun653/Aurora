/********************************************************************************
 *  File Name:
 *    timing.cpp
 *
 *  Description:
 *    Helper utilities for timing
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* STL Includes */
#include <cstddef>
#include <limits>

/* Aurora Includes */
#include <Aurora/utility>

/* Chimera Includes */
#include <Chimera/common>

namespace Aurora::Utility
{
  /*-------------------------------------------------------------------------------
  Classes
  -------------------------------------------------------------------------------*/
  PeriodicTimeout::PeriodicTimeout() : mLast( 0 ), mPeriod( std::numeric_limits<size_t>::max() )
  {
  }

  PeriodicTimeout::PeriodicTimeout( const size_t period, const size_t initial ) : mLast( initial ), mPeriod( period )
  {
  }


  PeriodicTimeout::~PeriodicTimeout()
  {
  }


  void PeriodicTimeout::setTimeout( const size_t timeout )
  {
    mPeriod = timeout;
  }


  bool PeriodicTimeout::expired()
  {
    return ( Chimera::millis() - mLast ) >= mPeriod;
  }


  void PeriodicTimeout::refresh()
  {
    mLast = Chimera::millis();
  }
}  // namespace Aurora::Utility
