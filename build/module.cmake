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
# set(MODULE somename)
# set(MODULE_INCLUDE ...)
# set(MODULE_QRC somename.qrc)
# set(MODULE_QML_IMPORT ...)
# set(MODULE_SRC ...)
# set(MODULE_LINK ...)
# include(${PROJECT_SOURCE_DIR}/build/module.cmake)

message(STATUS "Configuring " ${MODULE})

set(LIBRARY_TYPE STATIC)
set(_all_h_file "${PROJECT_SOURCE_DIR}/all.h")

include_directories(
    ${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/framework
    ${PROJECT_SOURCE_DIR}/framework/global
    ${PROJECT_SOURCE_DIR}/mu4
    ${MODULE_INCLUDE}
)

if (NOT ${MODULE_QRC} STREQUAL "")
    qt5_add_resources(RCC_SOURCES ${MODULE_QRC})
endif()

if (NOT ${MODULE_QML_IMPORT} STREQUAL "")
    set(QML_IMPORT_PATH "${QML_IMPORT_PATH};${MODULE_QML_IMPORT}" CACHE STRING "QtCreator extra import paths for QML modules" FORCE)
endif()

add_library(${MODULE} ${LIBRARY_TYPE}
    ${_all_h_file}
    ${RCC_SOURCES}
    ${MODULE_SRC}
)

if(NOT TARGET global)
    set(MODULE_LINK global ${MODULE_LINK})
endif()

target_link_libraries(${MODULE}
    ${MODULE_LINK}
    )

if (NOT MSVC)
    set_target_properties (${MODULE} PROPERTIES COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch")
else (NOT MSVC)
    set_target_properties (${MODULE} PROPERTIES COMPILE_FLAGS "${PCH_INCLUDE}" )
endif (NOT MSVC)

xcode_pch(${MODULE} all)

# Use MSVC pre-compiled headers
vstudio_pch( ${MODULE} )

# MSVC does not depend on mops1 & mops2 for PCH
if (NOT MSVC)
    ADD_DEPENDENCIES(${MODULE} mops1)
    ADD_DEPENDENCIES(${MODULE} mops2)
endif (NOT MSVC)
