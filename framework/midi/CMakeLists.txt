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

set(MODULE mu4_midi)

include(${PROJECT_SOURCE_DIR}/thirdparty/fluidsynth/fluidsynth.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/internal/zerberus/zerberus.cmake)


include(${PROJECT_SOURCE_DIR}/build/platform.cmake)
if (PLATFORM_WINDOWS)
    set(DRIVER_SRC
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/winmidioutport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/winmidioutport.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/winmidiinport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/winmidiinport.h
    )
elseif(PLATFORM_LINUX)
    set(DRIVER_SRC
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/lin/alsamidioutport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/lin/alsamidioutport.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/lin/alsamidiinport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/lin/alsamidiinport.h
    )
elseif(PLATFORM_OSX)
    set(DRIVER_SRC
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/osx/coremidioutport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/osx/coremidioutport.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/osx/coremidiinport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/osx/coremidiinport.h
    )
endif()


set(MODULE_SRC
    ${FLUIDSYNTH_SRC}
    ${ZERBERUS_SRC}
    ${DRIVER_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/midimodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/midimodule.h
    ${CMAKE_CURRENT_LIST_DIR}/isynthesizer.h
    ${CMAKE_CURRENT_LIST_DIR}/isynthesizersregister.h
    ${CMAKE_CURRENT_LIST_DIR}/isequencer.h
    ${CMAKE_CURRENT_LIST_DIR}/isoundfontsprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/imidiconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/imidiinport.h
    ${CMAKE_CURRENT_LIST_DIR}/imidioutport.h
    ${CMAKE_CURRENT_LIST_DIR}/imidiportdatasender.h
    ${CMAKE_CURRENT_LIST_DIR}/midievent.h
    ${CMAKE_CURRENT_LIST_DIR}/miditypes.h
    ${CMAKE_CURRENT_LIST_DIR}/midierrors.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/fluidsynth.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/fluidsynth.h
    ${CMAKE_CURRENT_LIST_DIR}/view/synthssettingsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/synthssettingsmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/sequencer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/sequencer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizercontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizercontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/zerberussynth.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/zerberussynth.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizersregister.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizersregister.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/soundfontsprovider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/soundfontsprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/midiconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/midiconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/dummymidioutport.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/dummymidioutport.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/dummymidiinport.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/dummymidiinport.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/midiportdatasender.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/midiportdatasender.h
    ${CMAKE_CURRENT_LIST_DIR}/devtools/midiportdevmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/devtools/midiportdevmodel.h
    )

set (MODULE_INCLUDE
    ${FLUIDSYNTH_INC}
    )

set (MODULE_DEF
    ${FLUIDSYNTH_DEF}
    )


if (MSVC OR MINGW)
    set(MODULE_LINK audiofile sndfiledll)
else (MSVC OR MINGW)
    set(MODULE_LINK audiofile ${SNDFILE_LIB})
endif (MSVC OR MINGW)


set (MODULE_HAS_C_CODE 1)

include(${PROJECT_SOURCE_DIR}/build/module.cmake)


