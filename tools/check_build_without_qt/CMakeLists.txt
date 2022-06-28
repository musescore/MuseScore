cmake_minimum_required(VERSION 3.14)

project(engraving_app LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

#set(CMAKE_AUTOUIC ON)
#set(CMAKE_AUTOMOC ON)
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
#find_package(Qt5Core ${QT_MIN_VERSION} REQUIRED)
#find_package(Qt5Gui ${QT_MIN_VERSION} REQUIRED)

set(QT_LIBRARIES
#    Qt5::Core
#    Qt5::Gui
)

include_directories(
#    ${Qt5Core_INCLUDE_DIRS}
#    ${Qt5Gui_INCLUDE_DIRS}
    ${CMAKE_CURRENT_LIST_DIR}
    ${MU_ROOT}/thirdparty/dtl
)

set(THIRDPARTY_DIR ${MU_ROOT}/thirdparty)

add_definitions(
    -DNO_QT_SUPPORT
    -DENGRAVING_NO_INTERACTIVE
    -DENGRAVING_NO_QSVGRENDER
    -DENGRAVING_NO_QTEXTDOCUMENT
    -DENGRAVING_NO_ACCESSIBILITY
    #-DHAW_LOGGER_QT_SUPPORT
)

set(NO_QT_SUPPORT ON)
set(ENGRAVING_NO_INTERNAL ON)
set(ENGRAVING_NO_ACCESSIBILITY ON)
add_subdirectory(${MU_ROOT}/src/engraving engraving)

set(NO_GLOBAL_INTERNAL ON)
add_subdirectory(${MU_ROOT}/src/framework/global global)


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
    engraving
)
