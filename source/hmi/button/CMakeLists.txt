# ====================================================
# Common
# ====================================================
set(LINK_LIBS
  chimera_inc       # Chimera public headers
  aurora_inc        # Aurora public headers
  aurora_build_options
)

# ====================================================
# Driver Library
# ====================================================
set(LIB aurora_hmi_button)
add_library(${LIB} STATIC
  hmi_button_driver.cpp
)
target_link_libraries(${LIB} PRIVATE ${LINK_LIBS})
export(TARGETS ${LIB} FILE "${PROJECT_BINARY_DIR}/Aurora/${LIB}.cmake")
