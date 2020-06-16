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

set(MODULE notation_scene)

set(MODULE_QRC
    ${CMAKE_CURRENT_LIST_DIR}/view/notation_view.qrc
    )

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/view/qml)

include(${CMAKE_CURRENT_LIST_DIR}/view/notationview.cmake)

set(MODULE_SRC
    ${NOTATIONVIEW_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/notationscenemodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/notationscenemodule.h
    ${CMAKE_CURRENT_LIST_DIR}/toolbar/notationtoolbarmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/toolbar/notationtoolbarmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/iscenenotationconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/scenenotationconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/scenenotationconfiguration.h
    )

set(MODULE_LINK
    notation
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)

#include(${CMAKE_CURRENT_LIST_DIR}/../../../framework/utests_base/utests_base.cmake)


#set(TESTS_SRC
#    ${TESTS_BASE_SRC}
#    ${CMAKE_CURRENT_LIST_DIR}/notationview_tests.cpp
#)


#find_package(Qt5 COMPONENTS Core Gui Quick REQUIRED)

#add_definitions(

#)

#include_directories(
#    ${CMAKE_CURRENT_LIST_DIR}
#    ${TESTS_BASE_INC}
#)

#set(TESTS_LIBS
#    ${TESTS_BASE_LIBS}
#)

#add_executable(tests_scene_notation ${TESTS_SRC})

#target_link_libraries(tests_scene_notation ${TESTS_LIBS} Qt5::Core Qt5::Gui Qt5::Quick)




