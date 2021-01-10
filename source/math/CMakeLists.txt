include("${COMMON_TOOL_ROOT}/cmake/utility/embedded.cmake")

# ====================================================
# Common
# ====================================================
set(LINK_LIBS
  chimera_intf_inc       # Chimera public headers
  aurora_intf_inc        # Aurora public headers
)

# ====================================================
# Driver Library
# ====================================================
function(build_library variant)
  set(LIB aurora_math${variant})
  add_library(${LIB} STATIC
    conversion.cpp
    math.cpp
  )
  target_link_libraries(${LIB} PRIVATE ${LINK_LIBS} prj_build_target${variant} prj_device_target)
  export(TARGETS ${LIB} FILE "${PROJECT_BINARY_DIR}/Aurora/${LIB}.cmake")
endfunction()

add_target_variants(build_library)
