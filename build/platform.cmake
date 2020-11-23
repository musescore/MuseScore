

if (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Windows")
    set(PLATFORM_IS_WINDOWS 1)
elseif(${CMAKE_HOST_SYSTEM_NAME} MATCHES "Linux")
    set(PLATFORM_IS_LINUX 1)
elseif(${CMAKE_HOST_SYSTEM_NAME} MATCHES "Darwin")
    set(PLATFORM_IS_MACOS 1)
else()
    message(FATAL_ERROR "Unknown platform")
endif()
