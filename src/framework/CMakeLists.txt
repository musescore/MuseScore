#add_subdirectory(global)
add_subdirectory(ui)
add_subdirectory(uicomponents)
add_subdirectory(fonts)
add_subdirectory(actions)

if (BUILD_SHORTCUTS_MODULE)
    add_subdirectory(shortcuts)
endif (BUILD_SHORTCUTS_MODULE)

if (BUILD_NETWORK_MODULE)
    add_subdirectory(network)
endif (BUILD_NETWORK_MODULE)

if (BUILD_SYSTEM_MODULE)
    add_subdirectory(system)
endif (BUILD_SYSTEM_MODULE)

if (BUILD_AUDIO_MODULE)
    add_subdirectory(audio)
endif (BUILD_AUDIO_MODULE)

add_subdirectory(midi)
add_subdirectory(midi_old)

if (BUILD_TELEMETRY_MODULE)
      add_subdirectory(telemetry)
endif (BUILD_TELEMETRY_MODULE)

if (BUILD_UNIT_TESTS)
    add_subdirectory(global/tests)
    add_subdirectory(system/tests)
endif(BUILD_UNIT_TESTS)

if (BUILD_VST)
    add_subdirectory(vst)
endif(BUILD_VST)
