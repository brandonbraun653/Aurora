/******************************************************************************
 *  File Name:
 *    hmi_sr_input.hpp
 *
 *  Description:
 *    Shift register digital input processing
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *****************************************************************************/

#pragma once
#ifndef AURORA_SR_INPUT_HPP
#define AURORA_SR_INPUT_HPP

/* STL Includes */
#include <cstdint>
#include <cstddef>

/* ETL Includes */
#include <etl/queue.h>

/* Aurora Includes */
#include <Aurora/hmi>

/* Chimera Includes */
#include <Chimera/common>
#include <Chimera/gpio>
#include <Chimera/spi>
#include <Chimera/thread>

namespace Aurora::HMI::SR
{
  /*---------------------------------------------------------------------------
  Forward Declarations
  ---------------------------------------------------------------------------*/
  struct InputEvent;

  /*---------------------------------------------------------------------------
  Aliases
  ---------------------------------------------------------------------------*/
  using InputBits  = uint32_t;
  using EventQueue = etl::queue<InputEvent, 10, etl::memory_model::MEMORY_MODEL_SMALL>;

  /*---------------------------------------------------------------------------
  Enumerations
  ---------------------------------------------------------------------------*/
  enum class Edge : uint8_t
  {
    RISING,
    FALLING,

    NUM_OPTIONS,
    INVALID
  };

  enum class State : uint8_t
  {
    ACTIVE,
    INACTIVE,

    NUM_OPTIONS,
    INVALID
  };

  enum class Polarity : uint8_t
  {
    ACTIVE_HIGH,
    ACTIVE_LOW,

    NUM_OPTIONS,
    INVALID
  };

  /*---------------------------------------------------------------------------
  Structures
  ---------------------------------------------------------------------------*/
  struct InputConfig
  {
    bool configured;      /**< Has this bit been configured? */
    InputBits bit;        /**< Which bit is being configured */
    Polarity polarity;    /**< Polarity of the bit */
    size_t debounceTime;  /**< Minimum time to sample the bit */
  };

  struct BitState
  {
    bool active;
    size_t debounceStart;
    size_t numSamples;
    Chimera::GPIO::State lastState;
  };

  struct InputEvent
  {
    uint8_t bit;      /**< Which bit caused the event */
    Edge edge;        /**< Edge transition type */
    State state;      /**< Is the bit active or inactive? */
    size_t timestamp; /**< Time the event was processed */
  };

  struct ShifterConfig
  {
    Chimera::SPI::Channel spiChannel;   /**< SPI channel shift registers use */
    Chimera::GPIO::Port chipSelectPort; /**< GPIO port of the SPI chip select */
    Chimera::GPIO::Pin chipSelectPin;   /**< GPIO pin of the SPI chip select */
    Chimera::GPIO::Port sampleKeyPort;  /**< GPIO port of the SR sample input */
    Chimera::GPIO::Pin sampleKeyPin;    /**< GPIO pin of the SR sample input */
    size_t byteWidth;                   /**< Number of bytes to read */
    uint32_t inputMask;                 /**< Bit field to enable/disable parsing */
  };

  /*---------------------------------------------------------------------------
  Classes
  ---------------------------------------------------------------------------*/
  /**
   * @brief Debounced parallel input shift register driver.
   *
   * Converts shift register input samples into discrete bit events that
   * can be processed further by the application. Intended primarily for
   * a large number of digital inputs with something like a 74HC165.
   *
   * @note The driver currently supports a max of 32 input signals.
   */
  class ShiftInput : public Chimera::Thread::Lockable<ShiftInput>
  {
  public:
    ShiftInput();
    ~ShiftInput();

    Chimera::Status_t init( const ShifterConfig &cfg );
    Chimera::Status_t configureBit( const InputConfig &cfg );
    bool nextEvent( InputEvent &event );
    void processHardware();

    /**
     * @brief Maximum number of input signals the driver supports
     * @return size_t
     */
    static constexpr size_t maxInputs()
    {
      return 32;
    }

  protected:
    bool readSR( uint32_t *const data, const size_t bytes );

  private:
    friend Chimera::Thread::Lockable<ShiftInput>;

    ShifterConfig mDriverCfg;
    InputConfig mBitConfig[ 32 ];
    BitState mBitState[ 32 ];
    EventQueue mEventQueue;
  };

}  // namespace Aurora::HMI::SR

#endif /* !AURORA_SR_INPUT_HPP */
