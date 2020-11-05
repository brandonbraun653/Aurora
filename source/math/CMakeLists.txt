# ====================================================
# Common
# ====================================================
set(LINK_LIBS
  chimera_inc       # Chimera public headers
  aurora_inc        # Aurora public headers
  lfs_inc           # Little FS public headers
  aurora_build_options
)

# ====================================================
# Driver Library
# ====================================================
set(LIB aurora_math)
add_library(${LIB} STATIC
  conversion.cpp
  math.cpp
)
target_link_libraries(${LIB} PRIVATE ${LINK_LIBS})
export(TARGETS ${LIB} FILE "${PROJECT_BINARY_DIR}/Aurora/${LIB}.cmake")