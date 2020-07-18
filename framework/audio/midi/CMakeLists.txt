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

set(MODULE audio_midi)

include(${CMAKE_CURRENT_LIST_DIR}/3rdparty/fluidlite.cmake)

set(MODULE_SRC
    ${FLUIDSYNTH_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/midimodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/midimodule.h
    ${CMAKE_CURRENT_LIST_DIR}/isynthesizer.h
    ${CMAKE_CURRENT_LIST_DIR}/isequencer.h
    ${CMAKE_CURRENT_LIST_DIR}/isoundfontfileprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/miditypes.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/fluidlitesynth.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/fluidlitesynth.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/sequencer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/sequencer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/sffproviderlocalfile.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/sffproviderlocalfile.h
    )

set (MODULE_INCLUDE
    ${FLUIDSYNTH_INC}
    )

set (MODULE_HAS_C_CODE 1)

include(${PROJECT_SOURCE_DIR}/build/module.cmake)


