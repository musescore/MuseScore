
include(${CMAKE_CURRENT_LIST_DIR}/GetPlatformInfo.cmake)

if (OS_IS_WIN)

    if (MUE_COMPILE_BUILD_64)
        set(DEPENDENCIES_DIR "${PROJECT_SOURCE_DIR}/dependencies/libx64")
    else()
        set(DEPENDENCIES_DIR "${PROJECT_SOURCE_DIR}/dependencies/libx86")
    endif()

    link_directories(${DEPENDENCIES_DIR})

    include_directories(${PROJECT_SOURCE_DIR}/dependencies/include)

endif(OS_IS_WIN)
