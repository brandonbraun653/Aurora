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

gen_static_lib_variants(
  TARGET
    aurora_filesystem_lfs_core
  SOURCES
    lfs/lfs_util.c
    lfs/lfs.c
    lfs/bd/lfs_rambd.c

  PRV_DEFINES
    #LFS_NO_ASSERT
    LFS_NO_MALLOC

  PRV_LIBRARIES
    aurora_intf_inc
    chimera_intf_inc
    lfs_intf_inc
  EXPORT_DIR
    "${PROJECT_BINARY_DIR}/Aurora/LittleFS"
)
