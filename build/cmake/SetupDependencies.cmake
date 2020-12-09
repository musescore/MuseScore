
include(${CMAKE_CURRENT_LIST_DIR}/GetPlatformInfo.cmake)

if (OS_IS_WIN)

    if (BUILD_64)
        set(DEPENDENCIES_DIR "${PROJECT_SOURCE_DIR}/dependencies/libx64")
    else (BUILD_64)
        set(DEPENDENCIES_DIR "${PROJECT_SOURCE_DIR}/dependencies/libx86")
    endif (BUILD_64)

    link_directories(${DEPENDENCIES_DIR})

    include_directories(${PROJECT_SOURCE_DIR}/dependencies/include)

endif(OS_IS_WIN)

if (OS_IS_MAC)

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

endif(OS_IS_MAC)
