/******************************************************************************
 *  File Name:
 *    list.hpp
 *
 *  Description:
 *    Generic list implementation that takes advantage of the memory allocator
 *    specified by the embedded system and uses a minimal amount of memory.
 *
 *  2022 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef AURORA_LIST_HPP
#define AURORA_LIST_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/


namespace Aurora::Container
{
  /*---------------------------------------------------------------------------
  Classes
  ---------------------------------------------------------------------------*/
  /**
   * @brief Circular doubly linked list implementation
   */
  class List
  {
  public:
    List *next;
    List *prev;


  };


}  // namespace Aurora::Container

#endif /* !AURORA_LIST_HPP */
