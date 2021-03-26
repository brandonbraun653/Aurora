include("${COMMON_TOOL_ROOT}/cmake/utility/embedded.cmake")

gen_static_lib_variants(
  TARGET
    aurora_filesystem_lfs_driver
  SOURCES
    file_intf.cpp
    generic/generic_driver.cpp
    littlefs/lfs_driver.cpp
  PRV_LIBRARIES
    aurora_intf_inc
    chimera_intf_inc
    lfs_intf_inc
  EXPORT_DIR
    "${PROJECT_BINARY_DIR}/Aurora"
)