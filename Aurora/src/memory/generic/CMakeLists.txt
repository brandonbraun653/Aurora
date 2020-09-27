# ====================================================
# Common
# ====================================================
set(LINK_LIBS
  chimera_inc       # Chimera public headers
  aurora_inc        # Aurora public headers
  prj_device_target # Compiler options for target device
)

# ====================================================
# Driver Library (+ Variant)
# ====================================================
set(LIB aurora_memory_generic)
add_library(${LIB} STATIC
  generic_utils.cpp
)
target_link_libraries(${LIB} PRIVATE ${LINK_LIBS})
export(TARGETS ${LIB} FILE "${PROJECT_BINARY_DIR}/Aurora/${LIB}.cmake")
