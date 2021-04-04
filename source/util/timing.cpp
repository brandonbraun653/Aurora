/********************************************************************************
 *  File Name:
 *    timing.cpp
 *
 *  Description:
 *    Helper utilities for timing
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* Aurora Includes */
#include <Aurora/utility>

/* Chimera Includes */
#include <Chimera/common>

namespace Aurora::Utility
{
  /*-------------------------------------------------------------------------------
  Classes
  -------------------------------------------------------------------------------*/
  PeriodicTimeout::PeriodicTimeout( const size_t period, const size_t initial ) : mPeriod( period ), mLast( initial )
  {

  }


  PeriodicTimeout::~PeriodicTimeout()
  {

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
