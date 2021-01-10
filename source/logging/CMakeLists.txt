include("${COMMON_TOOL_ROOT}/cmake/utility/embedded.cmake")

# ====================================================
# Common
# ====================================================
set(LINK_LIBS
  Boost::boost
  chimera_intf_inc       # Chimera public headers
  aurora_intf_inc        # Aurora public headers
  ulog_inc
)

# ====================================================
# Driver Library
# ====================================================
function(build_library variant)
  set(LIB aurora_log_serial${variant})
  add_library(${LIB} STATIC
    serial_sink.cpp
  )
  target_link_libraries(${LIB} PUBLIC ulog_inc)
  target_link_libraries(${LIB} PRIVATE ${LINK_LIBS} prj_build_target${variant} prj_device_target)
  export(TARGETS ${LIB} FILE "${PROJECT_BINARY_DIR}/Aurora/${LIB}.cmake")
endfunction()

add_target_variants(build_library)
