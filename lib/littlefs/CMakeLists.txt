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
# Public Include Target
# ====================================================
set(LIB lfs_inc)
add_library(${LIB} INTERFACE)
target_include_directories(${LIB} INTERFACE "./lfs")
export(TARGETS ${LIB} FILE "${PROJECT_BINARY_DIR}/LittleFS/lfs-inc.cmake")


# ====================================================
# Driver Library (+ Variant)
# ====================================================
set(LIB lfs_core)
add_library(${LIB} STATIC
  lfs/lfs_util.c
  lfs/lfs.c
)
target_link_libraries(${LIB} PRIVATE ${LINK_LIBS})
export(TARGETS ${LIB} FILE "${PROJECT_BINARY_DIR}/LittleFS/${LIB}.cmake")


set(LIB1 lfs_rambd)
add_library(${LIB1} STATIC
  lfs/bd/lfs_rambd.c
)
target_link_libraries(${LIB1} PRIVATE ${LINK_LIBS})
export(TARGETS ${LIB1} FILE "${PROJECT_BINARY_DIR}/LittleFS/${LIB1}.cmake")