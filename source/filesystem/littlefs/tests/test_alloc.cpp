/******************************************************************************
 *  File Name:
 *    test_alloc.cpp
 *
 *  Description:
 *    Embedded test for test_alloc.toml
 *
 *  2022 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <Aurora/filesystem>

namespace Aurora::FileSystem::LFS::Test::Alloc
{
  int parallel_allocation( lfs_t &lfs, const struct lfs_config &cfg )
  {
    /*-------------------------------------------------------------------------
    Config Constants
    -------------------------------------------------------------------------*/
    static constexpr size_t FILES = 3;
    static constexpr size_t SIZE  = 33;

    int         result         = 0;
    const char *names[ FILES ] = { "bacon", "eggs", "pancakes" };
    lfs_file_t  files[ FILES ];
    char        path[ SIZE ];
    uint8_t     buffer[ 128 ];
    size_t      size = 0;

    lfs_file_t file;

    result |= lfs_format( &lfs, &cfg );
    result |= lfs_mount( &lfs, &cfg );
    result |= lfs_mkdir( &lfs, "breakfast" );
    result |= lfs_unmount( &lfs );

    result |= lfs_mount( &lfs, &cfg );
    for ( int n = 0; n < FILES; n++ )
    {
      sprintf( path, "breakfast/%s", names[ n ] );
      result |= lfs_file_open( &lfs, &files[ n ], path, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND );
    }
    for ( int n = 0; n < FILES; n++ )
    {
      size = strlen( names[ n ] );
      for ( lfs_size_t i = 0; i < SIZE; i += size )
      {
        RT_HARD_ASSERT( size == lfs_file_write( &lfs, &files[ n ], names[ n ], size ) );
      }
    }
    for ( int n = 0; n < FILES; n++ )
    {
      result |= lfs_file_close( &lfs, &files[ n ] );
    }
    result |= lfs_unmount( &lfs );

    result |= lfs_mount( &lfs, &cfg );
    for ( int n = 0; n < FILES; n++ )
    {
      sprintf( path, "breakfast/%s", names[ n ] );
      result |= lfs_file_open( &lfs, &file, path, LFS_O_RDONLY );
      size = strlen( names[ n ] );
      for ( lfs_size_t i = 0; i < SIZE; i += size )
      {
        RT_HARD_ASSERT( size == lfs_file_read( &lfs, &file, buffer, size ) );
        RT_HARD_ASSERT( memcmp( buffer, names[ n ], size ) == 0 );
      }
      result |= lfs_file_close( &lfs, &file );
    }
    result |= lfs_unmount( &lfs );

    RT_HARD_ASSERT( result == 0 );
    return result;
  }
}  // namespace Aurora::FileSystem::LFS::Test::Alloc
