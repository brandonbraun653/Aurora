include("${COMMON_TOOL_ROOT}/cmake/utility/embedded.cmake")

gen_intf_lib(
  TARGET
    lfs_intf_inc
  INTF_INCLUDES
    "./lfs"
    "./lfs/bd"
  EXPORT_DIR
    "${PROJECT_BINARY_DIR}/Aurora/LittleFS"
)

# NOTE: If the project needs to override the lfs_util.h configuration, extend
# this interface by providing the following declaration:
#
#   target_compile_definitions(lfs_intf_inc INTERFACE LFS_CONFIG=<project_lfs_config.h(pp)>)
#   target_include_directories(lfs_intf_inc INTERFACE <directory_where_header_lives>)


gen_static_lib_variants(
  TARGET
    aurora_filesystem_lfs_core
  SOURCES
    lfs/lfs_util.c
    lfs/lfs.c
    lfs/bd/lfs_rambd.c

  PRV_DEFINES
    # LFS_NO_MALLOC
    # LFS_YES_TRACE
    # LFS_NO_DEBUG
    # LFS_NO_WARN
    # LFS_NO_ERROR

  PRV_LIBRARIES
    aurora_intf_inc
    chimera_intf_inc
    lfs_intf_inc
  EXPORT_DIR
    "${PROJECT_BINARY_DIR}/Aurora/LittleFS"
)
