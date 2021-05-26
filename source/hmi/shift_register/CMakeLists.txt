include("${COMMON_TOOL_ROOT}/cmake/utility/embedded.cmake")

gen_static_lib_variants(
  TARGET
    aurora_hmi_shift_register
  SOURCES
    hmi_sr_input.cpp
  PRV_LIBRARIES
    aurora_intf_inc
    chimera_intf_inc
  EXPORT_DIR
    "${PROJECT_BINARY_DIR}/Aurora"
)
