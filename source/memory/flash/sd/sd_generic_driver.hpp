/******************************************************************************
 *  File Name:
 *    sd_generic_driver.hpp
 *
 *  Description:
 *    Generic memory interface applied to SD card devices
 *
 *  2023 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef AURORA_MEMORY_SD_GENERIC_DRIVER_HPP
#define AURORA_MEMORY_SD_GENERIC_DRIVER_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Chimera/common>
#include <Chimera/thread>
#include <Chimera/sdio>
#include <Aurora/source/memory/generic/generic_intf.hpp>
#include <Aurora/source/memory/generic/generic_types.hpp>
#include <Aurora/source/memory/flash/sd/sd_generic_types.hpp>


namespace Aurora::Memory::Flash::SD
{
  /*---------------------------------------------------------------------------
  Classes
  ---------------------------------------------------------------------------*/
  class Driver : public virtual Aurora::Memory::IGenericDevice,
                 public Chimera::Thread::Lockable<Driver>
  {
  public:
    Driver();
    ~Driver();

    /*-------------------------------------------------------------------------
    SD Card Specific Interface
    -------------------------------------------------------------------------*/

    /**
     * @brief Initializes the SDIO driver
     *
     * @param cfg   Configuration parameters for the SDIO driver
     * @return bool
     */
    bool init( const Chimera::SDIO::HWConfig &cfg );

    /*-------------------------------------------------------------------------
    Generic Memory Device Interface
    -------------------------------------------------------------------------*/
    Aurora::Memory::Status open( const DeviceAttr *const attributes ) final override;
    Aurora::Memory::Status close() final override;
    Aurora::Memory::Status write( const size_t chunk, const size_t offset, const void *const data,
                                  const size_t length ) final override;
    Aurora::Memory::Status write( const size_t address, const void *const data, const size_t length ) final override;
    Aurora::Memory::Status read( const size_t chunk, const size_t offset, void *const data,
                                 const size_t length ) final override;
    Aurora::Memory::Status read( const size_t address, void *const data, const size_t length ) final override;
    Aurora::Memory::Status erase( const size_t chunk ) final override;
    Aurora::Memory::Status erase( const size_t address, const size_t length ) final override;
    Aurora::Memory::Status erase() final override;
    Aurora::Memory::Status flush() final override;
    Aurora::Memory::Status pendEvent( const Aurora::Memory::Event event, const size_t timeout ) final override;

  private:
    friend Chimera::Thread::Lockable<Driver>;

    Chimera::SDIO::Driver_rPtr mSDIO; /**< SDIO driver instance */
  };
}  // namespace Aurora::Memory::Flash::SD

#endif  /* !AURORA_MEMORY_SD_GENERIC_DRIVER_HPP */
