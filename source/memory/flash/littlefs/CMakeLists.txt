include("${COMMON_TOOL_ROOT}/cmake/utility/embedded.cmake")

# ====================================================
# Common
# ====================================================
set(LINK_LIBS
  chimera_inc       # Chimera public headers
  aurora_inc        # Aurora public headers
  lfs_inc           # Little FS public headers
)

# ====================================================
# Driver Library (+ Variant)
# ====================================================
function(build_library variant)
  set(LIB memory_flash_littlefs${variant})
  add_library(${LIB} STATIC
    lfs_hooks.cpp
  )
  target_link_libraries(${LIB} PRIVATE ${LINK_LIBS} prj_build_target${variant} prj_device_target)
  export(TARGETS ${LIB} FILE "${PROJECT_BINARY_DIR}/Aurora/${LIB}.cmake")
endfunction()

add_target_variants(build_library)
