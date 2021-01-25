
if (NOT BUILD_NETWORK_MODULE)
add_subdirectory(network)
endif (NOT BUILD_NETWORK_MODULE)

if (NOT BUILD_SYSTEM_MODULE)
    add_subdirectory(system)
endif (NOT BUILD_SYSTEM_MODULE)
