include("${COMMON_TOOL_ROOT}/cmake/utility/embedded.cmake")

gen_intf_lib(
  TARGET
    fatfs_intf_inc
  INTF_INCLUDES
    "./v0.15"
  EXPORT_DIR
    "${PROJECT_BINARY_DIR}/Aurora/FatFS"
)

gen_static_lib_variants(
  TARGET
    aurora_filesystem_fatfs_core
  SOURCES
    v0.15/diskio.c
    v0.15/ff.c
    v0.15/ffsystem.c
    v0.15/ffunicode.c
  PRV_LIBRARIES
    aurora_intf_inc
    chimera_intf_inc
    fatfs_intf_inc
  EXPORT_DIR
    "${PROJECT_BINARY_DIR}/Aurora/FatFS"
)
