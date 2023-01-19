include("${COMMON_TOOL_ROOT}/cmake/utility/embedded.cmake")

# ====================================================
# 3rd Party Libraries
# ====================================================
add_subdirectory("lib/cobs")
add_subdirectory("lib/embedded_template_library")
add_subdirectory("lib/littlefs")
add_subdirectory("lib/nanopb")
add_subdirectory("lib/segger")
add_subdirectory("lib/spiffs")

add_library(type_safe_inc INTERFACE)
target_include_directories(type_safe_inc INTERFACE "lib/type_safe/include")
export(TARGETS type_safe_inc FILE "${PROJECT_BINARY_DIR}/Aurora/lib/type-safe-inc.cmake")

add_library(ctre_inc INTERFACE)
target_include_directories(ctre_inc INTERFACE "lib/ctre/single-header")
export(TARGETS ctre_inc FILE "${PROJECT_BINARY_DIR}/Aurora/lib/ctre_inc.cmake")

# ====================================================
# Subproject Imports
# ====================================================
add_subdirectory("source/container")
add_subdirectory("source/database")
add_subdirectory("source/datastore")
add_subdirectory("source/datastruct")
add_subdirectory("source/filesystem")
add_subdirectory("source/hmi")
add_subdirectory("source/logging")
add_subdirectory("source/math")
add_subdirectory("source/memory")
add_subdirectory("source/tracing")
add_subdirectory("source/util")

# ====================================================
# Library Exports
# ====================================================
set(AuroraExportDir "${PROJECT_BINARY_DIR}/Aurora")

# ====================================================
# 3rd Party Library
# ====================================================
gen_intf_lib(TARGET sprout_intf_inc INTF_INCLUDES "lib/sprout" EXPORT_DIR ${AuroraExportDir})
gen_intf_lib(TARGET nanoprintf_inc INTF_INCLUDES "lib/nanoprintf" EXPORT_DIR ${AuroraExportDir})

# ====================================================
# Interface Library
# ====================================================
gen_intf_lib(
  TARGET
    aurora_intf_inc
  INTF_INCLUDES
    "./../"
    "./include"
  INTF_LIBRARIES
    cobs_intf_inc
    etl_inc
    lfs_intf_inc
    nanopb_intf_inc
    nanoprintf_inc
    sprout_intf_inc
    segger_sys_view_intf
    project_intf_inc

  EXPORT_DIR
    ${AuroraExportDir}
)

# ====================================================
# Core Library
# ====================================================
gen_intf_lib_variants(
  TARGET
    aurora_core
  LIBRARIES
    aurora_datastruct
    aurora_math
    aurora_utility
  EXPORT_DIR
    ${AuroraExportDir}
)

# ====================================================
# Memory Library
# ====================================================
gen_intf_lib_variants(
  TARGET
    aurora_memory
  LIBRARIES
    aurora_memory_eeprom
    aurora_memory_flash_device_test
    aurora_memory_generic
    aurora_memory_nor_flash
    aurora_memory_heap
  EXPORT_DIR
    ${AuroraExportDir}
)

# ====================================================
# FileSystem Library
# ====================================================
gen_intf_lib_variants(
  TARGET
    aurora_filesystem_lfs
  LIBRARIES
    aurora_filesystem_lfs_core
    aurora_filesystem_lfs_driver

  EXPORT_DIR
    ${AuroraExportDir}
)

gen_intf_lib_variants(
  TARGET
    aurora_filesystem_spiffs
  LIBRARIES
    aurora_filesystem_spiffs_core
    aurora_filesystem_spiffs_driver
  EXPORT_DIR
    ${AuroraExportDir}
)

gen_intf_lib_variants(
  TARGET
    aurora_filesystem_yaffs
  LIBRARIES
    yaffs_core                      #Defined in the yaffs2 submodule
    aurora_filesystem_yaffs_driver
  EXPORT_DIR
    ${AuroraExportDir}
)


# ====================================================
# Human-Machine Interface Library
# ====================================================
gen_intf_lib_variants(
  TARGET
    aurora_hmi
  LIBRARIES
    aurora_hmi_button
    aurora_hmi_rotary_encoder
    aurora_hmi_shift_register
  EXPORT_DIR
    ${AuroraExportDir}
)
