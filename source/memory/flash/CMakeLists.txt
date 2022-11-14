include("${COMMON_TOOL_ROOT}/cmake/utility/embedded.cmake")

# ====================================================
# Add sub-projects
# ====================================================
add_subdirectory("eeprom")
add_subdirectory("nor")

gen_static_lib_variants(
  TARGET
    aurora_memory_flash_device_test
  SOURCES
    device_test.cpp
  PRV_LIBRARIES
    chimera_intf_inc
    aurora_intf_inc
  EXPORT_DIR
    "${PROJECT_BINARY_DIR}/Aurora"
)