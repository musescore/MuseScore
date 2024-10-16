cmake_minimum_required(VERSION 3.16)

project(engraving_app LANGUAGES CXX)

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

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}
    ${MU_ROOT}/thirdparty/dtl
)

set(THIRDPARTY_DIR ${MU_ROOT}/thirdparty)

add_compile_definitions(
    NO_QT_SUPPORT
    GLOBAL_NO_INTERNAL
)

configure_file(${MU_ROOT}/src/framework/cmake/muse_framework_config.h.in muse_framework_config.h )

include(DeclareModuleSetup)

set(GLOBAL_NO_INTERNAL ON)
add_subdirectory(${MU_ROOT}/src/framework/global global)

set(DRAW_NO_INTERNAL ON)
add_subdirectory(${MU_ROOT}/src/framework/draw draw)

set(NO_QT_SUPPORT ON)
set(ENGRAVING_NO_INTERNAL ON)
set(ENGRAVING_NO_ACCESSIBILITY ON)
add_subdirectory(${MU_ROOT}/src/engraving engraving)

add_subdirectory(${MU_ROOT}/src/importexport/guitarpro guitarpro)

set(MUSICXML_NO_INTERNAL ON)
add_subdirectory(${MU_ROOT}/src/importexport/musicxml musicxml)

add_executable(engraving_app
    fontproviderstub.cpp
    fontproviderstub.h
    main.cpp
)

set_target_properties(engraving PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(muse_global PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(engraving_app PROPERTIES COMPILE_FLAGS "-fPIC")

target_include_directories(engraving_app PUBLIC
    ${MU_ROOT}/src/framework
    ${MU_ROOT}/src/framework/global
    ${MU_ROOT}/src
    ${MU_ROOT}/src/engraving
)

target_link_libraries(engraving_app
    ${QT_LIBRARIES}
    muse::global
    muse::draw
    engraving
    iex_guitarpro
)
