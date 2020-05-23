/********************************************************************************
 *  File Name:
 *    observer.hpp
 *
 *  Description:
 *    Interface for an Observer object
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_STRUCTURE_OBSERVER_HPP
#define AURORA_STRUCTURE_OBSERVER_HPP

/* Observer Includes */
#include <Aurora/structure/obs/types.hpp>

namespace Aurora::Structure::Observer
{
  class IObserver
  {
  public:
    ~IObserver() = default;

    virtual void update( UpdateArgs *event ) = 0;
  };
}  // namespace Aurora::Structure

#endif  /* !AURORA_STRUCTURE_OBSERVER_HPP */
