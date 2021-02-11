/********************************************************************************
 *  File Name:
 *    nor_adesto.cpp
 *
 *  Description:
 *    Adesto description information
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* STL Includes */
#include <cstdint>

/* Aurora Includes */
#include <Aurora/memory>
#include <Aurora/source/memory/flash/nor/manufacturer/nor_adesto.hpp>

/* Chimera Includes */
#include <Chimera/thread>

namespace Aurora::Flash::NOR::Adesto
{
  /*-------------------------------------------------------------------------------
  Public Data
  -------------------------------------------------------------------------------*/
  /**
   *  Device descriptors for Adesto memory chips. Must match the order of devices
   *  found in the Chip enum.
   */
  const Aurora::Memory::Properties ChipProperties[ static_cast<size_t>( Chip::ADESTO_END - Chip::ADESTO_START ) ] = {
    // AT25SF081
    { .writeChunk      = Aurora::Memory::Chunk::PAGE,
      .readChunk       = Aurora::Memory::Chunk::PAGE,
      .eraseChunk      = Aurora::Memory::Chunk::SECTOR,
      .jedec           = JEDEC_CODE,
      .pageSize        = 256,
      .blockSize       = 4 * 1024,
      .sectorSize      = 32 * 1024,
      .startAddress    = 0,
      .endAddress      = 8 * 1024 * 1024,
      .startUpDelay    = 20 * Chimera::Thread::TIMEOUT_1MS,
      .pagePgmDelay    = 5 * Chimera::Thread::TIMEOUT_1MS,
      .blockEraseDelay = 1300 * Chimera::Thread::TIMEOUT_1MS,
      .chipEraseDelay  = 30 * Chimera::Thread::TIMEOUT_1S,
      .eventPoll       = pollEvent },
  };

  /*-------------------------------------------------------------------------------
  Static Functions
  -------------------------------------------------------------------------------*/
  static uint16_t readStatusRegister_AT25SF081( const Chimera::SPI::Channel channel )
  {
    /*-------------------------------------------------
    Status Register Read Commands
    -------------------------------------------------*/
    static constexpr uint8_t READ_SR_BYTE1         = 0x05;
    static constexpr uint8_t READ_SR_BYTE1_CMD_LEN = 1;
    static constexpr uint8_t READ_SR_BYTE1_RSP_LEN = 1;
    static constexpr uint8_t READ_SR_BYTE1_OPS_LEN = READ_SR_BYTE1_CMD_LEN + READ_SR_BYTE1_RSP_LEN;

    static constexpr uint8_t READ_SR_BYTE2         = 0x35;
    static constexpr uint8_t READ_SR_BYTE2_CMD_LEN = 1;
    static constexpr uint8_t READ_SR_BYTE2_RSP_LEN = 1;
    static constexpr uint8_t READ_SR_BYTE2_OPS_LEN = READ_SR_BYTE2_CMD_LEN + READ_SR_BYTE2_RSP_LEN;

    /*-------------------------------------------------
    Initialize the command sequence
    -------------------------------------------------*/
    uint16_t result = 0;
    std::array<uint8_t, 5> cmdBuffer;
    std::array<uint8_t, 5> rxBuffer;
    cmdBuffer.fill( 0 );
    rxBuffer.fill( 0 );

    /*-------------------------------------------------
    Ensure the SPI driver/channel exists
    -------------------------------------------------*/
    auto spi = Chimera::SPI::getDriver( channel );
    if( !spi )
    {
      return 0xCCCC;
    }

    /*-------------------------------------------------
    Perform the SPI transaction
    -------------------------------------------------*/
    spi->lock();

    // Read out byte 1
    cmdBuffer[ 0 ] = READ_SR_BYTE1;
    spi->setChipSelect( Chimera::GPIO::State::LOW );
    spi->readWriteBytes( cmdBuffer.data(), rxBuffer.data(), READ_SR_BYTE1_OPS_LEN );
    spi->await( Chimera::Event::Trigger::TRIGGER_TRANSFER_COMPLETE, Chimera::Thread::TIMEOUT_BLOCK );
    spi->setChipSelect( Chimera::GPIO::State::HIGH );

    result |= rxBuffer[ 1 ];

    // Read out byte 2
    cmdBuffer[ 0 ] = READ_SR_BYTE2;
    cmdBuffer[ 1 ] = 0;
    spi->setChipSelect( Chimera::GPIO::State::LOW );
    spi->readWriteBytes( cmdBuffer.data(), rxBuffer.data(), READ_SR_BYTE2_OPS_LEN );
    spi->await( Chimera::Event::Trigger::TRIGGER_TRANSFER_COMPLETE, Chimera::Thread::TIMEOUT_BLOCK );
    spi->setChipSelect( Chimera::GPIO::State::HIGH );

    result |= ( rxBuffer[ 1 ] << 8 );

    spi->unlock();

    return result;
  }

  static Aurora::Memory::Status pollEvent_AT25SF081( const Chimera::SPI::Channel ch, const Aurora::Memory::Event event,
                                                     const size_t timeout )
  {
    /*-------------------------------------------------
    Status Register Byte 1 Definitions
    -------------------------------------------------*/
    static constexpr size_t SR_RDY_BUSY_POS = 0;
    static constexpr size_t SR_RDY_BUSY_MSK = 0x01;
    static constexpr size_t SR_RDY_BUSY     = SR_RDY_BUSY_MSK << SR_RDY_BUSY_POS;

    /*-------------------------------------------------
    Decide the bits used to indicate events occurred.
    -------------------------------------------------*/
    uint16_t eventBitMask = 0;  // Indicates bits to look at
    size_t pollDelay      = 0;  // How long to wait between checks

    switch ( event )
    {
      case Aurora::Memory::Event::MEM_ERASE_COMPLETE:
      case Aurora::Memory::Event::MEM_READ_COMPLETE:
      case Aurora::Memory::Event::MEM_WRITE_COMPLETE:
        eventBitMask = SR_RDY_BUSY;
        pollDelay    = Chimera::Thread::TIMEOUT_5MS;
        break;

      default:
        return Aurora::Memory::Status::ERR_UNSUPPORTED;
        break;
    };

    /*-------------------------------------------------
    For the AT25SF081, the device is busy when the
    RDY/BSY flag is set. Assuming this extends to other
    AT25 devices as well.

    See Table 10-1 of device datasheet.
    -------------------------------------------------*/
    uint16_t statusRegister = readStatusRegister_AT25SF081( ch );
    const size_t startTime  = Chimera::millis();

    while ( statusRegister & eventBitMask )
    {
      /*-------------------------------------------------
      Check for timeout, otherwise suspend this thread
      and allow others to do something.
      -------------------------------------------------*/
      if ( ( Chimera::millis() - startTime ) > timeout )
      {
        return Aurora::Memory::Status::ERR_TIMEOUT;
        break;
      }
      else
      {
        Chimera::delayMilliseconds( pollDelay );
      }

      /*-------------------------------------------------
      Poll the latest info
      -------------------------------------------------*/
      statusRegister = readStatusRegister_AT25SF081( ch );
    };

    return Aurora::Memory::Status::ERR_OK;
  }

  /*-------------------------------------------------------------------------------
  Public Functions
  -------------------------------------------------------------------------------*/
  Aurora::Memory::Status pollEvent( const uint8_t channel, const uint8_t device, const Aurora::Memory::Event event,
                                    const size_t timeout )
  {
    /*-------------------------------------------------
    For the NOR flash generic driver, channel is always
    SPI and the device is always Chip_t.
    -------------------------------------------------*/
    const Chimera::SPI::Channel spiChannel = static_cast<Chimera::SPI::Channel>( channel );
    const Chip_t chip                      = static_cast<Chip_t>( device );

    /*-------------------------------------------------
    Input Protection
    -------------------------------------------------*/
    if ( ( chip < Chip::ADESTO_START ) || ( chip >= Chip::ADESTO_END ) )
    {
      return Aurora::Memory::Status::ERR_BAD_ARG;
    }

    /*-------------------------------------------------
    Redirect to the proper device handler
    -------------------------------------------------*/
    switch ( chip )
    {
      case Chip::AT25SF081:
        return pollEvent_AT25SF081( spiChannel, event, timeout );
        break;

      default:
        return Aurora::Memory::Status::ERR_UNSUPPORTED;
        break;
    };
  }

}  // namespace Aurora::Flash::NOR::Adesto
