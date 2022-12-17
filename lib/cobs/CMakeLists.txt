include("${COMMON_TOOL_ROOT}/cmake/utility/embedded.cmake")

gen_intf_lib(
  TARGET
    cobs_intf_inc
  INTF_INCLUDES
    "./cobs"
  EXPORT_DIR
    "${PROJECT_BINARY_DIR}/Aurora/cobs"
)

gen_static_lib_variants(
  TARGET
    lib_cobs
  SOURCES
    cobs/cobs.c
    cobs/cobsr.c
  PRV_LIBRARIES
    cobs_intf_inc
  EXPORT_DIR
    "${PROJECT_BINARY_DIR}/Aurora/cobs"
)
