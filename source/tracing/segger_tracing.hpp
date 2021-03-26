/********************************************************************************
 *  File Name:
 *    segger_tracing.hpp
 *
 *  Description:
 *    Pulls in the descriptions needed for tracing with the Segger System View
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_TRACING_SEGGER_SYSTEM_VIEW_HPP
#define AURORA_TRACING_SEGGER_SYSTEM_VIEW_HPP

namespace Aurora::Trace
{
  /*-------------------------------------------------------------------------------
  Import the Segger driver
  -------------------------------------------------------------------------------*/
  #include "SEGGER_SYSVIEW.h"
}  // namespace Aurora::Tracing

#endif  /* !AURORA_TRACING_SEGGER_SYSTEM_VIEW_HPP */
