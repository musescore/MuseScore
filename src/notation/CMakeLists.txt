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

set(MODULE notation)

set(MODULE_QRC notationscene.qrc)
set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

find_package(Qt5 COMPONENTS Svg REQUIRED)

include(${CMAKE_CURRENT_LIST_DIR}/view/widgets/widgets.cmake)

set(MODULE_SRC
    ${WIDGETS_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/notationmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/notationmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/imasternotation.h
    ${CMAKE_CURRENT_LIST_DIR}/iexcerptnotation.h
    ${CMAKE_CURRENT_LIST_DIR}/inotation.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationreader.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationreadersregister.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationwriter.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationwritersregister.h
    ${CMAKE_CURRENT_LIST_DIR}/abstractnotationwriter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/abstractnotationwriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inotationcreator.h
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
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationuiactions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationuiactions.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/igetscore.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/masternotation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/masternotation.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/excerptnotation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/excerptnotation.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notation.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationundostack.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationundostack.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationstyle.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationstyle.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationcreator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationcreator.h
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
    ${CMAKE_CURRENT_LIST_DIR}/internal/mscznotationreader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/mscznotationreader.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationreadersregister.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationreadersregister.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationwritersregister.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationwritersregister.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/msczmetareader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/msczmetareader.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationplayback.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationplayback.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/midiinputcontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/midiinputcontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationmidiinput.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationmidiinput.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationparts.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationparts.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/masternotationparts.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/masternotationparts.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/searchcommandsparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/searchcommandsparser.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/inotationselectionrange.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/instrumentsconverter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/instrumentsconverter.h
    ${CMAKE_CURRENT_LIST_DIR}/view/inotationcontextmenu.h
    ${CMAKE_CURRENT_LIST_DIR}/view/notationpaintview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/notationpaintview.h
    ${CMAKE_CURRENT_LIST_DIR}/view/notationviewinputcontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/notationviewinputcontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/view/zoomcontrolmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/zoomcontrolmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/concertpitchcontrolmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/concertpitchcontrolmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/viewmodecontrolmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/viewmodecontrolmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/notationaccessibilitymodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/notationaccessibilitymodel.h
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
    ${CMAKE_CURRENT_LIST_DIR}/view/notationcontextmenu.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/notationcontextmenu.h
    ${CMAKE_CURRENT_LIST_DIR}/view/notationnavigator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/notationnavigator.h
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/abstractnoteinputbaritem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/abstractnoteinputbaritem.h
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/actionnoteinputbaritem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/actionnoteinputbaritem.h
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/undoredomodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/undoredomodel.h
    )

set(MODULE_UI
    ${WIDGETS_UI}
    )

set(FREETYPE_LIB )
if (USE_SYSTEM_FREETYPE)
    set(FREETYPE_LIB freetype)
else (USE_SYSTEM_FREETYPE)
    set(FREETYPE_LIB mscore_freetype)
endif (USE_SYSTEM_FREETYPE)

set(Z_LIB )
if (MSVC)
    include(FindStaticLibrary)
    set(Z_LIB zlibstat)
elseif (CC_IS_EMSCRIPTEN)
    #zlib included in main linker
else ()
    set(Z_LIB z)
endif ()

set(MODULE_LINK
    Qt5::Svg
    ${FREETYPE_LIB}
    qzip
    ${Z_LIB}
    libmscore
    commonscene
    uicomponents
    ui
    )

if (NOT MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 9.0)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-copy")
endif (NOT MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 9.0)

include(${PROJECT_SOURCE_DIR}/build/module.cmake)
