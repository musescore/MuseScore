

add_subdirectory(global)
add_subdirectory(ui)
add_subdirectory(uicomponents)
add_subdirectory(actions)
add_subdirectory(shortcuts)

if (BUILD_TELEMETRY_MODULE)
      add_subdirectory(telemetry)
      add_subdirectory(../thirdparty/google_analytics google_analytics)
endif (BUILD_TELEMETRY_MODULE)
