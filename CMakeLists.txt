include("${COMMON_TOOL_ROOT}/cmake/utility/embedded.cmake")

# ====================================================
# 3rd Party Libraries
# ====================================================
add_subdirectory("lib/uLog")
add_subdirectory("lib/littlefs")
add_subdirectory("lib/embedded_template_library")

add_library(type_safe_inc INTERFACE)
target_include_directories(type_safe_inc INTERFACE "lib/type_safe/include")
export(TARGETS type_safe_inc FILE "${PROJECT_BINARY_DIR}/Aurora/lib/type-safe-inc.cmake")

add_library(ctre_inc INTERFACE)
target_include_directories(ctre_inc INTERFACE "lib/ctre/single-header")
export(TARGETS ctre_inc FILE "${PROJECT_BINARY_DIR}/Aurora/lib/ctre_inc.cmake")

# ====================================================
# Subproject Imports
# ====================================================
add_subdirectory("source/database")
add_subdirectory("source/datastore")
add_subdirectory("source/hmi")
add_subdirectory("source/logging")
add_subdirectory("source/math")
add_subdirectory("source/memory")

# ====================================================
# Library Exports
# ====================================================
set(AuroraExportDir "${PROJECT_BINARY_DIR}/Aurora")

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
    etl_inc
    ulog_inc
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
    aurora_math
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
    aurora_memory_generic
    aurora_memory_nor_flash
    aurora_memory_heap
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
  EXPORT_DIR
    ${AuroraExportDir}
)


# ====================================================
# Logging Library
# ====================================================
gen_intf_lib_variants(
  TARGET
    aurora_logging
  LIBRARIES
    ulog_core
    aurora_log_serial
  EXPORT_DIR
    ${AuroraExportDir}
)
