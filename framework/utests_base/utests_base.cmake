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
# set(MODULE_TEST_SRC ...)           - set sources and headers files
# set(MODULE_TEST_LINK ...)          - set libraries for link

# After all the settings you need to do:
# include(${PROJECT_SOURCE_DIR}/framework/utests_base/utests_base.cmake)


message(STATUS "Configuring " ${MODULE_TEST})

set(_all_h_file "${PROJECT_SOURCE_DIR}/all.h")

# --- gtest ---
#define_property(TARGET PROPERTY OUTPUT_XML
#    BRIEF_DOCS "List XML files outputed by google test."
#    FULL_DOCS "List XML files outputed by google test."
#)

#cmake_policy(SET CMP0079 NEW)
#add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/googletest googletest)
#enable_testing()
get_property(gmock_LIBS GLOBAL PROPERTY gmock_LIBS)
get_property(gmock_INCLUDE_DIRS GLOBAL PROPERTY gmock_INCLUDE_DIRS)
# -------------

set(TESTS_BASE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)

set(TESTS_BASE_INC
    ${gmock_INCLUDE_DIRS}
    ${CMAKE_CURRENT_LIST_DIR}
)

set(TESTS_BASE_LIBS
   gmock
   global
)

include_directories(
    ${TESTS_BASE_INC}
    ${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/framework
    ${PROJECT_SOURCE_DIR}/framework/global
    ${PROJECT_SOURCE_DIR}/mu4
    ${MODULE_TEST_INCLUDE}
)

add_executable(${MODULE_TEST}
    ${_all_h_file}
    ${TESTS_BASE_SRC}
    ${MODULE_TEST_SRC}
    )

find_package(Qt5 COMPONENTS Core Gui Quick REQUIRED)

target_link_libraries(${MODULE_TEST}
    Qt5::Core Qt5::Gui Qt5::Quick
    ${TESTS_BASE_LIBS}
    ${MODULE_TEST_LINK}
    )

if (NOT MSVC)
    set_target_properties (${MODULE_TEST} PROPERTIES COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch")
else (NOT MSVC)
    set_target_properties (${MODULE_TEST} PROPERTIES COMPILE_FLAGS "${PCH_INCLUDE}" )
endif (NOT MSVC)

xcode_pch(${MODULE_TEST} all)

# Use MSVC pre-compiled headers
vstudio_pch( ${MODULE_TEST} )

# MSVC does not depend on mops1 & mops2 for PCH
if (NOT MSVC)
    ADD_DEPENDENCIES(${MODULE_TEST} mops1)
    ADD_DEPENDENCIES(${MODULE_TEST} mops2)
endif (NOT MSVC)

