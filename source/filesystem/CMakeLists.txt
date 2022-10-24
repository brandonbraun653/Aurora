include("${COMMON_TOOL_ROOT}/cmake/utility/embedded.cmake")

# Core Library
gen_static_lib_variants(
  TARGET
    aurora_filesystem_core
  SOURCES
    file_binary.cpp
    file_intf.cpp
  PRV_LIBRARIES
    aurora_intf_inc
    chimera_intf_inc
    lfs_intf_inc
  EXPORT_DIR
    "${PROJECT_BINARY_DIR}/Aurora"
)
# STD Target
gen_static_lib_variants(
  TARGET
    aurora_filesystem_generic_driver
  SOURCES
    generic/generic_driver.cpp
  PRV_LIBRARIES
    aurora_intf_inc
    chimera_intf_inc
  EXPORT_DIR
    "${PROJECT_BINARY_DIR}/Aurora"
)

# Little FS Target
gen_static_lib_variants(
  TARGET
    aurora_filesystem_lfs_driver
  SOURCES
    littlefs/lfs_driver.cpp
  PRV_LIBRARIES
    aurora_intf_inc
    chimera_intf_inc
    lfs_intf_inc
    aurora_filesystem_lfs_core
  EXPORT_DIR
    "${PROJECT_BINARY_DIR}/Aurora"
)

# SPIFFS Target
gen_static_lib_variants(
  TARGET
    aurora_filesystem_spiffs_driver
  SOURCES
    spiffs/spiffs_driver.cpp
    spiffs/spiffs_posix.cpp
  PRV_LIBRARIES
    aurora_intf_inc
    chimera_intf_inc
    spiffs_intf_inc
  EXPORT_DIR
    "${PROJECT_BINARY_DIR}/Aurora"
)