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
get_filename_component(DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
message("Configuring " ${DIR_NAME})

find_package(Qt5 COMPONENTS Core REQUIRED CONFIG)
add_definitions(
    ${Qt5Core_DEFINITIONS}
)
include_directories(
    ${Qt5Core_INCLUDE_DIRS}
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(LIBRARY_TYPE STATIC)

file(GLOB_RECURSE global_SOURCES RELATIVE ${CMAKE_CURRENT_LIST_DIR} *.cpp)
file(GLOB_RECURSE global_HEADERS RELATIVE ${CMAKE_CURRENT_LIST_DIR} *.h)

add_library(global ${LIBRARY_TYPE} ${global_SOURCES} ${global_HEADERS})
