/********************************************************************************
 *  File Name:
 *    yaffs_osglue.cpp
 *
 *  Description:
 *    OS level hook implementations for the YAFFS2 driver
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* STL Includes */
#include <cstddef>
#include <cstdlib>

/* Chimera Includes */
#include <Chimera/common>
#include <Chimera/thread>

/* Yaffs Includes */
#include <yaffs2/direct/yaffsfs.h>
#include <yaffs2/direct/yaffs_osglue.h>

/*-------------------------------------------------------------------------------
Static Data
-------------------------------------------------------------------------------*/
static int s_yaffs_errno;
static Chimera::Threading::RecursiveMutex s_yaffs_mutex;

/*-------------------------------------------------------------------------------
Public Functions
-------------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C"
{
#endif

  void yaffsfs_OSInitialisation()
  {
    s_yaffs_errno = 0;
    yaffsfs_LockInit();
  }


  void yaffsfs_Lock()
  {
    s_yaffs_mutex.lock();
  }


  void yaffsfs_Unlock()
  {
    s_yaffs_mutex.unlock();
  }


  void yaffsfs_LockInit()
  {
    // Do nothing
  }


  u32 yaffsfs_CurrentTime()
  {
    return static_cast<u32>( Chimera::millis() );
  }


  void yaffsfs_SetError( int err )
  {
    s_yaffs_errno = err;
  }


  int yaffsfs_GetLastError( void )
  {
    return s_yaffs_errno;
  }


  void *yaffsfs_malloc( size_t size )
  {
    // Overloaded by the RTOS driver if needed
    return malloc( size );
  }


  void yaffsfs_free( void *ptr )
  {
    // Overloaded by the RTOS driver if needed
    free( ptr );
  }


  void yaffsfs_get_malloc_values( unsigned *current, unsigned *high_water )
  {
    // Do nothing
  }


  int yaffsfs_CheckMemRegion( const void *addr, size_t size, int write_request )
  {
    // All memory is valid to access for now (except nullptr)
    return addr ? 0 : -1;
  }

  void yaffs_bug_fn( const char *file_name, int line_no )
  {
    // Do nothing
  }

#ifdef __cplusplus
}
#endif
