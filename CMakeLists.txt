# ====================================================
# 3rd Party Libraries
# ====================================================
add_subdirectory("lib/uLog")
add_subdirectory("lib/littlefs")
add_subdirectory("lib/embedded_template_library")

add_library(type_safe_inc INTERFACE)
target_include_directories(type_safe_inc INTERFACE "lib/type_safe/include")
export(TARGETS type_safe_inc FILE "${PROJECT_BINARY_DIR}/Aurora/lib/type-safe-inc.cmake")

# ====================================================
# Subproject Imports
# ====================================================
add_subdirectory("source/hmi")
add_subdirectory("source/logging")
add_subdirectory("source/math")
add_subdirectory("source/memory")

# ====================================================
# Library Exports
# ====================================================
# Public include location
add_library(aurora_inc INTERFACE)
target_include_directories(aurora_inc INTERFACE "./../" "./include")
target_link_libraries(aurora_inc INTERFACE etl_inc)
export(TARGETS aurora_inc FILE "${PROJECT_BINARY_DIR}/Aurora/aurora_inc.cmake")

# Core Library
add_library(aurora_core INTERFACE)
target_link_libraries(aurora_core INTERFACE
  aurora_math
  aurora_memory_generic
)
export(TARGETS aurora_core FILE "${PROJECT_BINARY_DIR}/Aurora/aurora_core.cmake")

# Human-Machine Interface Library
add_library(aurora_hmi INTERFACE)
target_link_libraries(aurora_hmi INTERFACE
  aurora_hmi_button
  aurora_hmi_rotary_encoder
)
export(TARGETS aurora_hmi FILE "${PROJECT_BINARY_DIR}/Aurora/aurora_hmi.cmake")

# Logging Library
add_library(aurora_logging INTERFACE)
target_link_libraries(aurora_logging INTERFACE
  ulog_core
  aurora_log_serial
)
export(TARGETS aurora_logging FILE "${PROJECT_BINARY_DIR}/Aurora/aurora_logging.cmake")


# ====================================================
# Custom Compiler Options:
# By default always build release variant
# ====================================================
set(BUILD_OPTIONS aurora_build_options)
add_library(${BUILD_OPTIONS} INTERFACE)
target_link_libraries(${BUILD_OPTIONS} INTERFACE
  prj_build_target
  #prj_build_target_rel
  #prj_build_target_dbg
  prj_device_target
)
export(TARGETS ${BUILD_OPTIONS} FILE "${PROJECT_BINARY_DIR}/Aurora/${BUILD_OPTIONS}.cmake")
