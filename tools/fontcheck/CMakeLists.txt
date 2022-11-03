cmake_minimum_required(VERSION 3.5)

project(fontcheck LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(MU_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)
set(PROJECT_ROOT_DIR ${MU_ROOT})

set(CMAKE_MODULE_PATH
    ${MU_ROOT}/build
    ${MU_ROOT}/build/cmake
    ${CMAKE_MODULE_PATH}
    )

set(QT_SUPPORT ON)
include(SetupBuildEnvironment)
include(GetPlatformInfo)

set(QT_MIN_VERSION    "5.15.0")
find_package(Qt5Core ${QT_MIN_VERSION} REQUIRED)
find_package(Qt5Gui ${QT_MIN_VERSION} REQUIRED)
find_package(Qt5Widgets ${QT_MIN_VERSION} REQUIRED)

set(QT_LIBRARIES
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${MU_ROOT}/thirdparty/dtl
    ${MU_ROOT}/thirdparty/freetype/include
)

set(THIRDPARTY_DIR ${MU_ROOT}/thirdparty)

add_definitions(-DDRAW_NO_QSVGRENDER)

add_subdirectory(${MU_ROOT}/src/framework/global global)
add_subdirectory(${MU_ROOT}/src/framework/draw draw)
add_subdirectory(${MU_ROOT}/src/framework/fonts fonts)
add_subdirectory(${MU_ROOT}/src/engraving engraving)
add_subdirectory(${MU_ROOT}/thirdparty/freetype freetype)

add_executable(fontcheck
    main.cpp
    fontface.cpp
    fontface.h
)

set_target_properties(engraving PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(global PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(fontcheck PROPERTIES COMPILE_FLAGS "-fPIC")

target_include_directories(fontcheck PUBLIC
    ${MU_ROOT}/src/framework
    ${MU_ROOT}/src/framework/global
    ${MU_ROOT}/src
    ${MU_ROOT}/src/engraving
)

target_link_libraries(fontcheck
    ${QT_LIBRARIES}
    global
    draw
    fonts
    engraving
)
