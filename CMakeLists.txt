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