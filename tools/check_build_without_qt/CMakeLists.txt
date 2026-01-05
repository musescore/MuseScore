cmake_minimum_required(VERSION 3.16)

project(engraving_app LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(MU_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)
set(MUSE_FRAMEWORK_PATH ${MU_ROOT})

set(CMAKE_MODULE_PATH
    ${MU_ROOT}/buildscripts
    ${MU_ROOT}/buildscripts/cmake
    ${MU_ROOT}/src/framework/cmake
    ${CMAKE_MODULE_PATH}
)

include(SetupBuildEnvironment)
include(GetPlatformInfo)

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}
    ${MU_ROOT}/thirdparty/dtl
)

set(THIRDPARTY_DIR ${MU_ROOT}/thirdparty)

set(MUSE_QT_SUPPORT OFF)
include(${MU_ROOT}/src/framework/cmake/MuseSetupConfiguration.cmake)

set(GLOBAL_NO_INTERNAL ON)
add_subdirectory(${MU_ROOT}/src/framework/global global)

set(DRAW_NO_INTERNAL ON)
add_subdirectory(${MU_ROOT}/src/framework/draw draw)

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

target_include_directories(engraving_app PUBLIC
    ${MU_ROOT}
    ${MU_ROOT}/src
    ${MU_ROOT}/src/framework
    ${MU_ROOT}/src/framework/global
)

target_link_libraries(engraving_app
    muse::global
    muse::draw
    engraving
    iex_guitarpro
    iex_musicxml
)
