include("${COMMON_TOOL_ROOT}/cmake/utility/embedded.cmake")

# Little FS Target
gen_static_lib_variants(
  TARGET
    aurora_filesystem_lfs_driver
  SOURCES
    file_intf.cpp
    littlefs/lfs_driver.cpp
  PRV_LIBRARIES
    aurora_intf_inc
    chimera_intf_inc
    lfs_intf_inc
  EXPORT_DIR
    "${PROJECT_BINARY_DIR}/Aurora"
)

# SPIFFS Target
gen_static_lib_variants(
  TARGET
    aurora_filesystem_spiffs_driver
  SOURCES
    file_intf.cpp
    spiffs/spiffs_driver.cpp
  PRV_LIBRARIES
    aurora_intf_inc
    chimera_intf_inc
    spiffs_intf_inc
  EXPORT_DIR
    "${PROJECT_BINARY_DIR}/Aurora"
)

# YAFFS Target
gen_static_lib_variants(
  TARGET
    aurora_filesystem_yaffs_driver
  SOURCES
    file_intf.cpp
    yaffs/yaffs2_driver.cpp
  PRV_LIBRARIES
    aurora_intf_inc
    chimera_intf_inc
    yaffs_intf_inc
  EXPORT_DIR
    "${PROJECT_BINARY_DIR}/Aurora"
)
