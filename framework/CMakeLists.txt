
add_subdirectory(global)
add_subdirectory(ui)
add_subdirectory(uicomponents)
add_subdirectory(actions)
add_subdirectory(shortcuts)
add_subdirectory(workspace)

add_subdirectory(audio/engine)

if (BUILD_TELEMETRY_MODULE)
      add_subdirectory(telemetry)
      add_subdirectory(../thirdparty/google_analytics google_analytics)
endif (BUILD_TELEMETRY_MODULE)

if (BUILD_UNIT_TESTS)
    add_subdirectory(global/tests)
endif(BUILD_UNIT_TESTS)
