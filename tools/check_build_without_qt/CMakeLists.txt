cmake_minimum_required(VERSION 3.14)

project(engraving_app LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MU_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)
set(PROJECT_ROOT_DIR ${MU_ROOT})

set(CMAKE_MODULE_PATH
    ${MU_ROOT}/build
    ${MU_ROOT}/build/cmake
    ${CMAKE_MODULE_PATH}
    )

include(SetupBuildEnvironment)
include(GetPlatformInfo)

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}
    ${MU_ROOT}/thirdparty/dtl
)

set(THIRDPARTY_DIR ${MU_ROOT}/thirdparty)

add_definitions(
    -DNO_QT_SUPPORT
    -DGLOBAL_NO_INTERNAL

    -DMUSESCORE_REVISION="abc123"
    -DMUSESCORE_BUILD_NUMBER="123456"
    -DMUSESCORE_VERSION="4.0.0"
    -DMUSESCORE_VERSION_LABEL="dev"
    -DMUSESCORE_INSTALL_SUFFIX=""
    -DMUSESCORE_INSTALL_PREFIX=""
    -DMUSESCORE_INSTALL_NAME=""
)

set(GLOBAL_NO_INTERNAL ON)
add_subdirectory(${MU_ROOT}/src/framework/global global)

set(DRAW_NO_INTERNAL ON)
add_subdirectory(${MU_ROOT}/src/framework/draw draw)

set(NO_QT_SUPPORT ON)
set(ENGRAVING_NO_INTERNAL ON)
set(ENGRAVING_NO_ACCESSIBILITY ON)
add_subdirectory(${MU_ROOT}/src/engraving engraving)

add_subdirectory(${MU_ROOT}/src/importexport/guitarpro guitarpro)


add_executable(engraving_app
    fontproviderstub.cpp
    fontproviderstub.h
    main.cpp
)

set_target_properties(engraving PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(global PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(engraving_app PROPERTIES COMPILE_FLAGS "-fPIC")

target_include_directories(engraving_app PUBLIC
    ${MU_ROOT}/src/framework
    ${MU_ROOT}/src/framework/global
    ${MU_ROOT}/src
    ${MU_ROOT}/src/engraving
)

target_link_libraries(engraving_app
    ${QT_LIBRARIES}
    global
    draw
    engraving
    iex_guitarpro
)
