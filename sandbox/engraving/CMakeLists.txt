cmake_minimum_required(VERSION 3.14)

project(engraving_app LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

#set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTORCC ON)

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

set(QT_MIN_VERSION    "5.15.0")
find_package(Qt5Core ${QT_MIN_VERSION} REQUIRED)
find_package(Qt5Gui ${QT_MIN_VERSION} REQUIRED)
#find_package(Qt5Svg ${QT_MIN_VERSION} REQUIRED)
#find_package(Qt5Widgets ${QT_MIN_VERSION} REQUIRED)

set(QT_LIBRARIES
    Qt5::Core
    Qt5::Gui
#    Qt5::Svg
#    Qt5::Widgets
)

include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
#    ${Qt5Svg_INCLUDE_DIRS}
#    ${Qt5Widgets_INCLUDE_DIRS}
    ${CMAKE_CURRENT_LIST_DIR}
    ${MU_ROOT}/thirdparty/dtl
)

add_subdirectory(${MU_ROOT}/thirdparty/qzip qzip)
add_subdirectory(${MU_ROOT}/src/framework/midi_old midi_old)

add_definitions(
    -DNO_ENGRAVING_INTERACTIVE
    -DNO_ENGRAVING_QSVGRENDER
)

set(NO_ENGRAVING_INTERNAL ON)
add_subdirectory(${MU_ROOT}/src/engraving engraving)

include(${MU_ROOT}/src/framework/global/modularity/modularity.cmake)

add_executable(engraving_app
    ${MODULARITY_SRC}
    ${MU_ROOT}/src/framework/global/translation.cpp
    ${MU_ROOT}/src/framework/global/translation.h
    log.h
    fontproviderstub.cpp
    fontproviderstub.h
    main.cpp
)

set_target_properties(qzip PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(midi_old PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(engraving PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(engraving_app PROPERTIES COMPILE_FLAGS "-fPIC")

target_include_directories(engraving_app PUBLIC
    ${MU_ROOT}/src/framework
    ${MU_ROOT}/src/framework/global
    ${MU_ROOT}/src
    ${MU_ROOT}/src/engraving
)

target_link_libraries(engraving_app
    ${QT_LIBRARIES}
    engraving
)
