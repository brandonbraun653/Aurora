include("${COMMON_TOOL_ROOT}/cmake/utility/embedded.cmake")

gen_intf_lib(
  TARGET
    nanopb_intf_inc
  INTF_INCLUDES
    "./nanopb"
  EXPORT_DIR
    "${PROJECT_BINARY_DIR}/Aurora/nanopb"
)

gen_static_lib_variants(
  TARGET
    lib_nanopb
  SOURCES
    nanopb/pb_common.c
    nanopb/pb_decode.c
    nanopb/pb_encode.c
  PRV_LIBRARIES
    nanopb_intf_inc
  EXPORT_DIR
    "${PROJECT_BINARY_DIR}/Aurora/nanopb"
)
