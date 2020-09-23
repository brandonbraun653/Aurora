# ====================================================
# Common
# ====================================================
set(LINK_LIBS
  chimera_inc       # Chimera public headers
  aurora_inc        # Aurora public headers
  lfs_inc           # Little FS public headers
  prj_device_target # Compiler options for target device
)

# ====================================================
# Driver Library (+ Variant)
# ====================================================
set(LIB memory_flash_littlefs)
add_library(${LIB} STATIC
  lfs_hooks.cpp
)
target_link_libraries(${LIB} PRIVATE ${LINK_LIBS})
export(TARGETS ${LIB} FILE "${PROJECT_BINARY_DIR}/Aurora/${LIB}.cmake")

