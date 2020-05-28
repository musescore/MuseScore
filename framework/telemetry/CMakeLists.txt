#=============================================================================
#  MuseScore
#  Music Composition & Notation
#
#  Copyright (C) 2019 MuseScore BVBA and others
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

set(MODULE telemetry)

set(MODULE_QRC telemetry_resources.qrc)

set(MODULE_INCLUDE
    ${PROJECT_SOURCE_DIR}/thirdparty
    ${PROJECT_SOURCE_DIR}/mscore
    )

file(GLOB_RECURSE telemetry_SOURCES RELATIVE ${CMAKE_CURRENT_LIST_DIR} *.cpp)
file(GLOB_RECURSE telemetry_HEADERS RELATIVE ${CMAKE_CURRENT_LIST_DIR} *.h)

set(MODULE_SRC
    ${telemetry_SOURCES}
    ${telemetry_HEADERS}
    )

set(MODULE_LINK
    google_analytics
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)

if (MSVC)
set_target_properties (${MODULE} PROPERTIES
    COMPILE_FLAGS "/wd4127"
)
endif (MSVC)



