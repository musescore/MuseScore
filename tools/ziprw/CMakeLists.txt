cmake_minimum_required(VERSION 3.16)

project(ziprw LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MU_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)
set(MUSE_FRAMEWORK_PATH ${MU_ROOT})

set(CMAKE_MODULE_PATH
        ${MU_ROOT}/buildscripts
        ${MU_ROOT}/buildscripts/cmake
        ${CMAKE_MODULE_PATH}
)

include(SetupBuildEnvironment)
include(GetPlatformInfo)

set(QT_MIN_VERSION "6.2.4")
include(SetupQt6)

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}
    ${MU_ROOT}/src/framework
)

add_compile_definitions(
    GLOBAL_NO_INTERNAL
)

configure_file(${MU_ROOT}/src/framework/cmake/muse_framework_config.h.in muse_framework_config.h )

include(DeclareModuleSetup)

set(GLOBAL_NO_INTERNAL ON)
add_subdirectory(${MU_ROOT}/src/framework/global global)

set(ENGRAVING_NO_INTERNAL ON)
set(ENGRAVING_NO_ACCESSIBILITY ON)

add_executable(ziprw
    ${MU_ROOT}/src/framework/global/io/internal/filesystem.cpp
    ${MU_ROOT}/src/framework/global/io/internal/filesystem.h
    main.cpp
)

set_target_properties(muse_global PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(ziprw PROPERTIES COMPILE_FLAGS "-fPIC")

target_include_directories(ziprw PUBLIC
    ${MU_ROOT}/src/framework
    ${MU_ROOT}/src
)

target_link_libraries(ziprw PRIVATE
    ${QT_LIBRARIES}
    muse_global
)
