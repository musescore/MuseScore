
if (NOT BUILD_SHORTCUTS_MODULE)
    add_subdirectory(shortcuts)
endif (NOT BUILD_SHORTCUTS_MODULE)

if (NOT BUILD_NETWORK_MODULE)
add_subdirectory(network)
endif (NOT BUILD_NETWORK_MODULE)

if (NOT BUILD_SYSTEM_MODULE)
    add_subdirectory(system)
endif (NOT BUILD_SYSTEM_MODULE)

if (NOT BUILD_AUDIO_MODULE)
    add_subdirectory(audio)
endif (NOT BUILD_AUDIO_MODULE)
