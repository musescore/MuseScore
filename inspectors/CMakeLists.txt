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
get_filename_component(DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
message("Configuring " ${DIR_NAME})

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED CONFIG)
add_definitions(
      ${Qt5Core_DEFINITIONS}
      ${Qt5Gui_DEFINITIONS}
      ${Qt5Widgets_DEFINITIONS}
      )

include_directories(
      ${Qt5Core_INCLUDE_DIRS}
      ${Qt5Gui_INCLUDE_DIRS}
      ${Qt5Widgets_INCLUDE_DIRS}
      ${PROJECT_SOURCE_DIR}/global
      ${PROJECT_SOURCE_DIR}/framework/global
      ${PROJECT_SOURCE_DIR}/framework
      ${PROJECT_SOURCE_DIR}/libmscore
      ${PROJECT_SOURCE_DIR}/thirdparty
      ${PROJECT_SOURCE_DIR}/mscore
      ${PROJECT_SOURCE_DIR}/telemetry
      ${PROJECT_BINARY_DIR}
      )

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(_all_h_file "${PROJECT_BINARY_DIR}/all.h")
set(LIBRARY_TYPE STATIC)

file(GLOB_RECURSE inspectors_SOURCES RELATIVE ${CMAKE_CURRENT_LIST_DIR} *.cpp)
file(GLOB_RECURSE inspectors_HEADERS RELATIVE ${CMAKE_CURRENT_LIST_DIR} *.h)
file(GLOB_RECURSE inspectors_ui_files RELATIVE ${CMAKE_CURRENT_LIST_DIR} *.ui)

qt5_wrap_ui(inspectors_UI ${inspectors_ui_files})
qt5_add_resources(RCC_SOURCES inspectors_resources.qrc)

add_library(inspectors ${LIBRARY_TYPE} ${inspectors_SOURCES} ${inspectors_HEADERS} ${inspectors_UI} ${RCC_SOURCES})

if (NOT MSVC)
   set_target_properties (
      inspectors
      PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch"
      )
else (NOT MSVC)
   set_target_properties (
      inspectors
      PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE}"
      )
endif (NOT MSVC)

xcode_pch(inspectors all)

# Use MSVC pre-compiled headers
vstudio_pch( inspectors )

target_link_libraries(inspectors global global_old)
