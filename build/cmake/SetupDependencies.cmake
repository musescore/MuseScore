
include(${CMAKE_CURRENT_LIST_DIR}/GetPlatformInfo.cmake)

if (PLATFORM_IS_WINDOWS)

    if (BUILD_64)
        set(DEPENDENCIES_DIR "${PROJECT_SOURCE_DIR}/dependencies/libx64")
    else (BUILD_64)
        set(DEPENDENCIES_DIR "${PROJECT_SOURCE_DIR}/dependencies/libx86")
    endif (BUILD_64)

    link_directories(${DEPENDENCIES_DIR})

endif(PLATFORM_IS_WINDOWS)

if (PLATFORM_IS_MACOS)

    find_library(AudioToolboxFW        NAMES AudioToolbox)
    find_library(AudioUnitFW           NAMES AudioUnit)
    find_library(CoreAudioFW           NAMES CoreAudio)
    find_library(CoreMidiFW            NAMES CoreMIDI)
    find_library(SystemConfigurationFW NAMES SystemConfiguration)
    find_library(CoreServicesFW        NAMES CoreServices)
    find_library(AppKit                NAMES AppKit)

    set(OsxFrameworks
        ${AudioToolboxFW}
        ${AudioUnitFW}
        ${CoreAudioFW}
        ${CoreMidiFW}
        ${SystemConfigurationFW}
        ${CoreServicesFW}
        ${AppKit}
        )

endif(PLATFORM_IS_MACOS)
