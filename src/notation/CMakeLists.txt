# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore BVBA and others
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

set(MODULE notation)

set(MODULE_QRC notationscene.qrc)
set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

find_package(Qt5 COMPONENTS Svg REQUIRED)

include(${CMAKE_CURRENT_LIST_DIR}/view/widgets/widgets.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/view/styledialog/styledialog.cmake)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/notationmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/notationmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/imasternotation.h
    ${CMAKE_CURRENT_LIST_DIR}/iexcerptnotation.h
    ${CMAKE_CURRENT_LIST_DIR}/inotation.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationpainting.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationviewstate.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationnoteinput.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationselection.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationinteraction.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/inotationundostack.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationstyle.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationaccessibility.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationmidiinput.h
    ${CMAKE_CURRENT_LIST_DIR}/notationtypes.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/notationerrors.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationplayback.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationelements.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationparts.h
    ${CMAKE_CURRENT_LIST_DIR}/iinstrumentsrepository.h
    ${CMAKE_CURRENT_LIST_DIR}/iselectinstrumentscenario.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationcreator.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/notationuiactions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationuiactions.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/igetscore.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/masternotation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/masternotation.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/excerptnotation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/excerptnotation.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notation.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationpainting.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationpainting.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationviewstate.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationviewstate.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationundostack.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationundostack.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationstyle.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationstyle.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/scorecallbacks.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/scorecallbacks.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationnoteinput.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationnoteinput.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationselection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationselection.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationselectionrange.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationselectionrange.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationelements.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationelements.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationactioncontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationactioncontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationinteraction.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationinteraction.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationaccessibility.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/positionswriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/positionswriter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/mscnotationwriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/mscnotationwriter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationplayback.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationplayback.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/midiinputoutputcontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/midiinputoutputcontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationmidiinput.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationmidiinput.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationparts.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationparts.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/masternotationparts.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/masternotationparts.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/searchcommandsparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/searchcommandsparser.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/inotationselectionrange.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/instrumentsrepository.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/instrumentsrepository.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationcreator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationcreator.h

    ${CMAKE_CURRENT_LIST_DIR}/view/abstractnotationpaintview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/abstractnotationpaintview.h
    ${CMAKE_CURRENT_LIST_DIR}/view/notationpaintview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/notationpaintview.h
    ${CMAKE_CURRENT_LIST_DIR}/view/notationviewinputcontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/notationviewinputcontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/view/playbackcursor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/playbackcursor.h
    ${CMAKE_CURRENT_LIST_DIR}/view/noteinputcursor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/noteinputcursor.h
    ${CMAKE_CURRENT_LIST_DIR}/view/loopmarker.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/loopmarker.h
    ${CMAKE_CURRENT_LIST_DIR}/view/notationswitchlistmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/notationswitchlistmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/partlistmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/partlistmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/searchpopupmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/searchpopupmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/noteinputbarmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/noteinputbarmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/noteinputbarcustomisemodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/noteinputbarcustomisemodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/notationtoolbarmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/notationtoolbarmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/notationcontextmenumodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/notationcontextmenumodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/notationnavigator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/notationnavigator.h
    ${CMAKE_CURRENT_LIST_DIR}/view/noteinputbarcustomiseitem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/noteinputbarcustomiseitem.h
    ${CMAKE_CURRENT_LIST_DIR}/view/continuouspanel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/continuouspanel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/undoredomodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/undoredomodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/noteflagstypeselectormodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/noteflagstypeselectormodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/abstractelementpopupmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/abstractelementpopupmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/harppedalpopupmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/harppedalpopupmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/caposettingsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/caposettingsmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/selectionfiltermodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/selectionfiltermodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/editgridsizedialogmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/editgridsizedialogmodel.h
    
    ${CMAKE_CURRENT_LIST_DIR}/view/pianokeyboard/pianokeyboardtypes.h
    ${CMAKE_CURRENT_LIST_DIR}/view/pianokeyboard/pianokeyboardcontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/pianokeyboard/pianokeyboardcontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/view/pianokeyboard/pianokeyboardview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/pianokeyboard/pianokeyboardview.h
    ${CMAKE_CURRENT_LIST_DIR}/view/pianokeyboard/pianokeyboardpanelcontextmenumodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/pianokeyboard/pianokeyboardpanelcontextmenumodel.h

    ${WIDGETS_SRC}
    ${STYLEDIALOG_SRC}
    )

set(MODULE_UI
    ${WIDGETS_UI}
    )

set(MODULE_LINK
    Qt5::Svg
    draw
    engraving
    commonscene
    uicomponents
    ui
    )

if (NOT MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 9.0)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-copy")
endif (NOT MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 9.0)

include(${PROJECT_SOURCE_DIR}/build/module.cmake)
