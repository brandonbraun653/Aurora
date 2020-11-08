# ====================================================
# Common
# ====================================================
set(LINK_LIBS
  chimera_inc       # Chimera public headers
  aurora_inc        # Aurora public headers
  aurora_build_options
)

# ====================================================
# Public Include Target
# ====================================================
set(LIB etl_inc)
add_library(${LIB} INTERFACE)
target_include_directories(${LIB} INTERFACE "./etl/include")
export(TARGETS ${LIB} FILE "${PROJECT_BINARY_DIR}/EmbeddedTemplateLibrary/etl-inc.cmake")