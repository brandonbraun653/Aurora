# ====================================================
# Common
# ====================================================
set(LINK_LIBS
  Boost::boost
  chimera_inc       # Chimera public headers
  aurora_inc        # Aurora public headers
  ulog_inc
  aurora_build_options
)

# ====================================================
# Driver Library
# ====================================================
set(LIB aurora_log_serial)
add_library(${LIB} STATIC
  serial_sink.cpp
)
target_link_libraries(${LIB} PRIVATE ${LINK_LIBS})
target_link_libraries(${LIB} PUBLIC ulog_inc)
export(TARGETS ${LIB} FILE "${PROJECT_BINARY_DIR}/Aurora/${LIB}.cmake")