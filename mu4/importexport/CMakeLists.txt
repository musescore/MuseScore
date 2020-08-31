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

set(MODULE importexport)

include(${CMAKE_CURRENT_LIST_DIR}/internal/musicxml/musicxml.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/internal/midiimport/midiimport.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/internal/ove/ove.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/internal/guitarpro/guitarpro.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/internal/musedata/musedata.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/internal/bb/bb.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/internal/capella/capella.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/internal/bww/bww.cmake)

set(MODULE_SRC
    ${MUSICXML_SRC}
    ${MIDIIMPORT_SRC}
    ${OVE_SRC}
    ${GUITARPRO_SRC}
    ${MUSEDATA_SRC}
    ${BB_SRC}
    ${CAPELLA_SRC}
    ${BWW_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/importexportmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/importexportmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/iimportexportconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/importexportconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/importexportconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/musicxmlreader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/musicxmlreader.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationmidireader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationmidireader.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/musedatareader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/musedatareader.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationbbreader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationbbreader.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/capellareader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/capellareader.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/overeader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/overeader.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationbwwreader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationbwwreader.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarproreader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarproreader.h
    )

add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/beatroot beatroot) # for midiimport
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/rtf2html rtf2html) # for capella

set(MODULE_LINK
    midi_old    # for midiimport
    beatroot    # for midiimport
    rtf2html    # for capella
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)

