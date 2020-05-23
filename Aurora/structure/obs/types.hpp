/********************************************************************************
 *  File Name:
 *    types.hpp
 *
 *  Description:
 *    Types used in the Observer Pattern
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef AURORA_STRUCTURE_OBSERVER_TYPES_HPP
#define AURORA_STRUCTURE_OBSERVER_TYPES_HPP

/* STL Includes */
#include <cstddef>

namespace Aurora::Structure::Observer
{
  // Fwd decl
  class IObserver;

  static constexpr size_t EMPTY_TYPE = 0;

  struct ControlBlock
  {
    size_t elements;    /**< Number of elements in the list */
    IObserver ** list;  /**< Some external array of IObserver* objects */
    bool inUse;         /**< Flag to mark if the list is in use by an Observable object */

    /**
     *  Resets the data members of the structure
     *  @return void
     */
    void clear()
    {
      for ( size_t idx = 0; idx < elements; idx++ )
      {
        list[ idx ] = nullptr;
      }

      inUse = false;
    }
  };

  struct UpdateArgs
  {
    void * data;  /**< Data to be passed in */
    size_t size;  /**< How large the data is in bytes*/
  };

}  // namespace Aurora::Structure

#endif  /* !AURORA_STRUCTURE_OBSERVER_TYPES_HPP */
