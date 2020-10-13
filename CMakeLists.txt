# ====================================================
# Subproject Imports
# ====================================================
add_subdirectory("Aurora/src/memory")

# ====================================================
# Various exports
# ====================================================

# Public include location
add_library(aurora_inc INTERFACE)
target_include_directories(aurora_inc INTERFACE ".")
export(TARGETS aurora_inc FILE "${PROJECT_BINARY_DIR}/Aurora/aurora_inc.cmake")


add_library(aurora_core INTERFACE)
target_link_libraries(aurora_core INTERFACE
  aurora_memory_generic
)
export(TARGETS aurora_core FILE "${PROJECT_BINARY_DIR}/Aurora/aurora_core.cmake")


# ====================================================
# 3rd Party Libraries
# ====================================================
add_subdirectory("lib/littlefs")

add_library(type_safe_inc INTERFACE)
target_include_directories(type_safe_inc INTERFACE "lib/type_safe/include")
export(TARGETS type_safe_inc FILE "${PROJECT_BINARY_DIR}/Aurora/lib/type-safe-inc.cmake")


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
