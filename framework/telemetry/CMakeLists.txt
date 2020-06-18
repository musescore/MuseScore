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
set(LIBRARY_TYPE STATIC)
message(STATUS "Configuring " ${MODULE})

include_directories(
      ${PROJECT_SOURCE_DIR}/thirdparty
      ${PROJECT_SOURCE_DIR}/mscore
      ${PROJECT_SOURCE_DIR}/framework/global
      ${PROJECT_BINARY_DIR}
      )

file(GLOB_RECURSE telemetry_SOURCES RELATIVE ${CMAKE_CURRENT_LIST_DIR} *.cpp)
file(GLOB_RECURSE telemetry_HEADERS RELATIVE ${CMAKE_CURRENT_LIST_DIR} *.h)

if (Qt5Widgets_VERSION VERSION_GREATER_EQUAL "5.11.0")
      find_package(Qt5QuickCompiler)
      qtquick_compiler_add_resources(RCC_SOURCES telemetry_resources.qrc)
else (Qt5Widgets_VERSION VERSION_GREATER_EQUAL "5.11.0")
      QT5_ADD_RESOURCES(RCC_SOURCES telemetry_resources.qrc)
endif (Qt5Widgets_VERSION VERSION_GREATER_EQUAL "5.11.0")


add_library(${MODULE} ${LIBRARY_TYPE}
    ${telemetry_SOURCES}
    ${telemetry_HEADERS}
    ${RCC_SOURCES}
)

target_link_libraries(${MODULE} global google_analytics)

if (MSVC)
  set_target_properties (${MODULE} PROPERTIES
    COMPILE_FLAGS "/wd4127 /wd26439 /wd5999" # ToDo for Qt 5.15: "... /wd26444 /wd26451 /wd26495 /wd26498 /wd26812" ??
  )
endif (MSVC)
