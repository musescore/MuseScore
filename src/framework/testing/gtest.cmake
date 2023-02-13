#=============================================================================
#  MuseScore
#  Music Composition & Notation
#
#  Copyright (C) 2020 MuseScore BVBA and others
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#=============================================================================

## Setup
# set(MODULE_TEST somename)          - set module (target) name
# set(MODULE_TEST_INCLUDE ...)       - set include (by default see below include_directories)
# set(MODULE_TEST_DEF ...)           - set definitions
# set(MODULE_TEST_SRC ...)           - set sources and headers files
# set(MODULE_TEST_LINK ...)          - set libraries for link
# set(MODULE_TEST_DATA_ROOT ...)     - set test data root path

# After all the settings you need to do:
# include(${PROJECT_SOURCE_DIR}/framework/testing/gtest.cmake)

message(STATUS "Configuring ${MODULE_TEST}")

get_property(gmock_LIBS GLOBAL PROPERTY gmock_LIBS)
get_property(gmock_INCLUDE_DIRS GLOBAL PROPERTY gmock_INCLUDE_DIRS)

add_executable(${MODULE_TEST}
    ${CMAKE_CURRENT_LIST_DIR}/gmain.cpp
    ${CMAKE_CURRENT_LIST_DIR}/environment.cpp
    ${CMAKE_CURRENT_LIST_DIR}/environment.h
    ${MODULE_TEST_SRC}
    )

target_include_directories(${MODULE_TEST} PRIVATE
    ${PROJECT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/src/framework
    ${PROJECT_SOURCE_DIR}/src/framework/global
    ${PROJECT_SOURCE_DIR}/src
    ${MODULE_TEST_INCLUDE}
)

target_compile_definitions(${MODULE_TEST} PRIVATE
    ${MODULE_TEST_DEF}
    ${MODULE_TEST}_DATA_ROOT="${MODULE_TEST_DATA_ROOT}"
)

find_package(Qt5 COMPONENTS Core Gui REQUIRED)

target_link_libraries(${MODULE_TEST}
    Qt5::Core
    Qt5::Gui
    gmock
    global
    ${MODULE_TEST_LINK}
    )

add_test(NAME ${MODULE_TEST} COMMAND ${MODULE_TEST})
