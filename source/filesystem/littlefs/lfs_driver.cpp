/********************************************************************************
 *  File Name:
 *    file_driver.cpp
 *
 *  Description:
 *    Driver level implementation
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* STL Includes */
#include <cstdint>

/* Aurora Include */
#include <Aurora/memory>
#include <Aurora/filesystem>
#include <Aurora/logging>
#include <Aurora/source/filesystem/file_driver.hpp>
#include <Aurora/source/filesystem/file_config.hpp>

/* Chimera Includes */
#include <Chimera/assert>

/*-------------------------------------------------------------------------------
Static Data
-------------------------------------------------------------------------------*/
static bool s_initialized                          = false;   /**< Is the module initialized? */
static lfs_t *s_fs                                 = nullptr; /**< External LFS control block */
static const lfs_config *s_fs_cfg                  = nullptr; /**< External LFS memory config */
static const Aurora::Memory::Properties *sNORProps = nullptr; /**< External NOR flash properties */
static Aurora::Flash::NOR::Driver sNORFlash;                  /**< Flash memory driver supporting the file system */


/*-------------------------------------------------------------------------------
Public Functions
-------------------------------------------------------------------------------*/
int lfs_safe_read( const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size )
{
  using namespace Aurora::Memory;

  /*-------------------------------------------------
  Input protection
  -------------------------------------------------*/
  if ( !c )
  {
    return LFS_ERR_INVAL;
  }

  /*-------------------------------------------------
  Figure out the real address & invoke the device driver
  -------------------------------------------------*/
  sNORFlash.lock();

  auto error = LFS_ERR_OK;
  if ( Status::ERR_OK != sNORFlash.read( block, off, buffer, size ) )
  {
    error = LFS_ERR_IO;
  }

  sNORFlash.unlock();
  return error;
}


int lfs_safe_prog( const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size )
{
  using namespace Aurora::Memory;

  /*-------------------------------------------------
  Input protection
  -------------------------------------------------*/
  if ( !c )
  {
    return LFS_ERR_INVAL;
  }

  /*-------------------------------------------------
  Figure out the real address & invoke the device driver
  -------------------------------------------------*/
  sNORFlash.lock();

  auto error = LFS_ERR_OK;
  if ( ( Status::ERR_OK != sNORFlash.write( block, off, buffer, size ) )
       || ( Status::ERR_OK != sNORFlash.pendEvent( Event::MEM_WRITE_COMPLETE, sNORProps->pagePgmDelay ) ) )
  {
    error = LFS_ERR_IO;
  }

  sNORFlash.unlock();
  return error;
}


int lfs_safe_erase( const struct lfs_config *c, lfs_block_t block )
{
  using namespace Aurora::Memory;

  /*-------------------------------------------------
  Input protection
  -------------------------------------------------*/
  if ( !c )
  {
    return LFS_ERR_INVAL;
  }

  /*-------------------------------------------------
  Invoke the device driver
  -------------------------------------------------*/
  sNORFlash.lock();

  auto error = LFS_ERR_OK;
  if ( ( Status::ERR_OK != sNORFlash.erase( block ) )
       || ( Status::ERR_OK != sNORFlash.pendEvent( Event::MEM_ERASE_COMPLETE, sNORProps->blockEraseDelay ) ) )
  {
    error = LFS_ERR_IO;
  }

  sNORFlash.unlock();
  return error;
}


int lfs_safe_sync( const struct lfs_config *c )
{
  using namespace Aurora::Memory;

  /*-------------------------------------------------
  Input protection
  -------------------------------------------------*/
  if ( !c )
  {
    return LFS_ERR_INVAL;
  }

  /*-------------------------------------------------
  Invoke the device driver
  -------------------------------------------------*/
  sNORFlash.lock();

  auto error = LFS_ERR_OK;
  if ( auto sts = sNORFlash.flush(); sts != Status::ERR_OK )
  {
    error = LFS_ERR_IO;
  }

  sNORFlash.unlock();
  return error;
}


namespace Aurora::FileSystem::LFS
{
  /*-------------------------------------------------------------------------------
  Driver Specific Implementation
  -------------------------------------------------------------------------------*/
  bool attachFS( lfs_t *const fs, const lfs_config *const cfg )
  {
    /*-------------------------------------------------
    Input Protection
    -------------------------------------------------*/
    if ( !fs || !cfg )
    {
      return false;
    }

    /*-------------------------------------------------
    Cache the pointers. This assumes the given data is
    never destroyed.
    -------------------------------------------------*/
    s_fs     = fs;
    s_fs_cfg = cfg;
    return true;
  }


  bool attachDevice( const Aurora::Flash::NOR::Chip_t dev, const Chimera::SPI::Channel channel, const lfs_config &cfg )
  {
    sNORProps = Aurora::Flash::NOR::getProperties( dev );
    return sNORFlash.configure( dev, channel );
  }


  bool fullChipErase( const size_t timeout )
  {
    using namespace Aurora::Memory;

    /*-------------------------------------------------
    Issue the erase command
    -------------------------------------------------*/
    if ( sNORFlash.erase() != Status::ERR_OK )
    {
      return false;
    }

    /*-------------------------------------------------
    Wait for the erase to complete
    -------------------------------------------------*/
    return sNORFlash.pendEvent( Event::MEM_ERASE_COMPLETE, timeout ) == Status::ERR_OK;
  }


  std::string_view err2str( const int error )
  {
    return "INVALID";
  }


  int mount()
  {
    using namespace Aurora::Logging;

    /*-------------------------------------------------
    Input Protection
    -------------------------------------------------*/
    RT_HARD_ASSERT( s_fs );
    RT_HARD_ASSERT( s_fs_cfg );
    RT_HARD_ASSERT( sNORProps );

    /*-------------------------------------------------
    Try mounting. It's possible to get a clean chip,
    which will need some formatting before mounting.
    -------------------------------------------------*/
    int err = lfs_mount( s_fs, s_fs_cfg );
    if ( err )
    {
      /*-------------------------------------------------
      Attempt to quick format
      -------------------------------------------------*/
      getRootSink()->flog( Level::LVL_DEBUG, "Initial mount failed with code %d. Reformatting.\r\n", err );
      err = lfs_format( s_fs, s_fs_cfg );
      if ( err )
      {
        getRootSink()->flog( Level::LVL_DEBUG, "Reformatting failed with code %d\r\n", err );
      }

      /*-------------------------------------------------
      Retry the mount
      -------------------------------------------------*/
      err = lfs_mount( s_fs, s_fs_cfg );
    }

    /*-------------------------------------------------
    Log the mount status for posterity
    -------------------------------------------------*/
    if ( err )
    {
      getRootSink()->flog( Level::LVL_DEBUG, "Mount failed with code %d\r\n", err );
    }
    else
    {
      getRootSink()->flog( Level::LVL_DEBUG, "File system mounted\r\n" );
    }

    return err == 0;
  }


  int unmount()
  {
    return 0;
  }

}  // namespace Aurora::FileSystem
