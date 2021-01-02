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
function(build_core variant)
  add_library(aurora_core${variant} INTERFACE)
  target_link_libraries(aurora_core${variant} INTERFACE
    aurora_math${variant}
    aurora_memory_generic${variant}
  )
  export(TARGETS aurora_core${variant} FILE "${PROJECT_BINARY_DIR}/Aurora/aurora_core${variant}.cmake")
endfunction()
add_target_variants(build_core)


# Human-Machine Interface Library
function(build_hmi variant)
  add_library(aurora_hmi${variant} INTERFACE)
  target_link_libraries(aurora_hmi${variant} INTERFACE
    aurora_hmi_button${variant}
    aurora_hmi_rotary_encoder${variant}
  )
  export(TARGETS aurora_hmi${variant} FILE "${PROJECT_BINARY_DIR}/Aurora/aurora_hmi${variant}.cmake")
endfunction()
add_target_variants(build_hmi)


# Logging Library
function(build_logging variant)
  add_library(aurora_logging${variant} INTERFACE)
  target_link_libraries(aurora_logging${variant} INTERFACE
    ulog_core${variant}
    aurora_log_serial${variant}
  )
  export(TARGETS aurora_logging${variant} FILE "${PROJECT_BINARY_DIR}/Aurora/aurora_logging${variant}.cmake")
endfunction()
add_target_variants(build_logging)
