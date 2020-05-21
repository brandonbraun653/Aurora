/********************************************************************************
 *  File Name:
 *    aurora_utility.cpp
 *
 *  Description:
 *    Implementation of common utility functions
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* Aurora Includes */
#include <Aurora/utility>

namespace Aurora::Utility
{

  // Valid only for ARM
  void insertBreakpoint()
  {
    __asm( "BKPT #0\n" );
  }

}  // namespace Aurora::Utility
