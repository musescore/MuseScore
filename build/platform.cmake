

if (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Windows")
    set(PLATFORM_WINDOWS 1)
elseif(${CMAKE_HOST_SYSTEM_NAME} MATCHES "Linux")
    set(PLATFORM_LINUX 1)
elseif(${CMAKE_HOST_SYSTEM_NAME} MATCHES "Darwin")
    set(PLATFORM_OSX 1)
else()
    message(FATAL_ERROR "Unknown platform")
endif()
