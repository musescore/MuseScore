cmake_minimum_required(VERSION 3.14)

project(engraving_app LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON)
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

find_package(Qt6 6.2.4 REQUIRED COMPONENTS Core Gui)

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}
    ${MU_ROOT}/thirdparty/dtl
)

set(THIRDPARTY_DIR ${MU_ROOT}/thirdparty)

add_compile_definitions(
    NO_QT_SUPPORT
    NO_ENGRAVING_INTERACTIVE
    NO_ENGRAVING_QSVGRENDER
)

set(NO_ENGRAVING_INTERNAL ON)
add_subdirectory(${MU_ROOT}/src/engraving engraving)

set(NO_GLOBAL_INTERNAL ON)
add_subdirectory(${MU_ROOT}/src/framework/global global)

# include(${MU_ROOT}/src/framework/global/modularity/modularity.cmake)
add_executable(engraving_app
    # ${MODULARITY_SRC}
    # ${MU_ROOT}/src/framework/global/translation.cpp
    # ${MU_ROOT}/src/framework/global/translation.h
    log.h
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
    global
    engraving
    Qt6::Core
    Qt6::Gui
)
