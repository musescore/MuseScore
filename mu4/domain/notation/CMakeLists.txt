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

find_package(Qt5 COMPONENTS Svg REQUIRED)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/notationdomainmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/notationdomainmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/inotation.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationreader.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationreadersregister.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationcreator.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationinputstate.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationselection.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationinteraction.h
    ${CMAKE_CURRENT_LIST_DIR}/notationtypes.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/notationerrors.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationplayback.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationactions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationactions.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notation.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationcreator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationcreator.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/scorecallbacks.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/scorecallbacks.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationinputstate.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationinputstate.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationselection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationselection.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationactioncontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationactioncontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/igetscore.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationinteraction.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationinteraction.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/mscznotationreader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/mscznotationreader.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationreadersregister.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationreadersregister.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/msczmetareader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/msczmetareader.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationplayback.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationplayback.h
    )

set(FREETYPE_LIB )
if (USE_SYSTEM_FREETYPE)
     set(FREETYPE_LIB freetype)
else (USE_SYSTEM_FREETYPE)
     set(FREETYPE_LIB mscore_freetype)
endif (USE_SYSTEM_FREETYPE)

set(MODULE_LINK
    Qt5::Svg
    ${FREETYPE_LIB}
    qzip
    z
    libmscore
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)

