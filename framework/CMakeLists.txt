

# --- gtest ---
define_property(TARGET PROPERTY OUTPUT_XML
    BRIEF_DOCS "List XML files outputed by google test."
    FULL_DOCS "List XML files outputed by google test."
)
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/googletest googletest)
# ---

add_subdirectory(global)
add_subdirectory(global/tests)
add_subdirectory(ui)
add_subdirectory(uicomponents)
add_subdirectory(actions)
add_subdirectory(shortcuts)

if (BUILD_TELEMETRY_MODULE)
      add_subdirectory(telemetry)
      add_subdirectory(../thirdparty/google_analytics google_analytics)
endif (BUILD_TELEMETRY_MODULE)
