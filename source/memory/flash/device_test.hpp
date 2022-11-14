/******************************************************************************
 *  File Name:
 *    device_test.hpp
 *
 *  Description:
 *    Utilities for testing out a flash memory device driver's read/write/erase
 *    capabilities and performance.
 *
 *  2022 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef AURORA_MEMORY_FLASH_DEVICE_TEST_HPP
#define AURORA_MEMORY_FLASH_DEVICE_TEST_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/source/memory/generic/generic_intf.hpp>

namespace Aurora::Memory::Flash
{

  class DeviceTest
  {
  public:
    DeviceTest( IGenericDevice *device );

  private:
    IGenericDevice *mDev;
  };
}  // namespace Aurora::Memory::Flash

#endif /* !AURORA_MEMORY_FLASH_DEVICE_TEST_HPP */
