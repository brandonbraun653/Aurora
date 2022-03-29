include("${COMMON_TOOL_ROOT}/cmake/utility/embedded.cmake")

gen_static_lib_variants(
  TARGET
    aurora_memory_eeprom
  SOURCES
    eeprom_generic_driver.cpp
  PRV_LIBRARIES
    chimera_intf_inc
    aurora_intf_inc
  EXPORT_DIR
    "${PROJECT_BINARY_DIR}/Aurora"
)
