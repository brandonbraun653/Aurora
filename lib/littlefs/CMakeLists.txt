# ====================================================
# Common
# ====================================================
set(LINK_LIBS
  chimera_intf_inc       # Chimera public headers
  aurora_intf_inc        # Aurora public headers
  lfs_inc           # Little FS public headers
)

# ====================================================
# Public Include Target
# ====================================================
set(LIB lfs_inc)
add_library(${LIB} INTERFACE)
target_include_directories(${LIB} INTERFACE "./lfs")
export(TARGETS ${LIB} FILE "${PROJECT_BINARY_DIR}/LittleFS/lfs-inc.cmake")


# ====================================================
# Driver Library
# ====================================================
function(build_library variant)
  set(LIB lfs_core${variant})
  add_library(${LIB} STATIC
    lfs/lfs_util.c
    lfs/lfs.c
    lfs/bd/lfs_rambd.c
  )
  target_link_libraries(${LIB} PUBLIC lfs_inc)
  target_link_libraries(${LIB} PRIVATE ${LINK_LIBS} prj_build_target${variant} prj_device_target)
  export(TARGETS ${LIB} FILE "${PROJECT_BINARY_DIR}/LittleFS/${LIB}.cmake")
endfunction()

add_target_variants(build_library)
