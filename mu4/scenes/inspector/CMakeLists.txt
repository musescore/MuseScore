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
get_filename_component(MODULE ${CMAKE_CURRENT_SOURCE_DIR} NAME)
message("Configuring " ${MODULE})

set(MODULE_QRC ${CMAKE_CURRENT_LIST_DIR}/view/inspector_resources.qrc )

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/view/qml)

file(GLOB_RECURSE inspector_SOURCES RELATIVE ${CMAKE_CURRENT_LIST_DIR} *.cpp)
file(GLOB_RECURSE inspector_HEADERS RELATIVE ${CMAKE_CURRENT_LIST_DIR} *.h)
file(GLOB_RECURSE inspector_ui_files RELATIVE ${CMAKE_CURRENT_LIST_DIR} *.ui)

set(MODULE_SRC
    ${inspector_SOURCES}
    ${inspector_HEADERS}
    )

set(MODULE_UI ${inspector_UI} )

include_directories(
      ${Qt5Core_INCLUDE_DIRS}
      ${Qt5Gui_INCLUDE_DIRS}
      ${Qt5Widgets_INCLUDE_DIRS}
      ${PROJECT_SOURCE_DIR}/global
      ${PROJECT_SOURCE_DIR}/framework/global
      ${PROJECT_SOURCE_DIR}/framework
      ${PROJECT_SOURCE_DIR}/libmscore
      ${PROJECT_SOURCE_DIR}/mscore
      ${PROJECT_BINARY_DIR}
      )

set(MODULE_LINK
    global
    global_old
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)
