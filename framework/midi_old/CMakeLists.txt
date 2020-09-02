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

set(MODULE midi_old)

set(MIDI_OLD_DIR ${PROJECT_SOURCE_DIR}/audio/midi)

set(MODULE_SRC
    ${MIDI_OLD_DIR}/event.cpp
    ${MIDI_OLD_DIR}/event.h
    ${MIDI_OLD_DIR}/midifile.cpp
    ${MIDI_OLD_DIR}/midifile.h
    ${MIDI_OLD_DIR}/midiinstrument.cpp
    ${MIDI_OLD_DIR}/midiinstrument.h
    )

set (MODULE_INCLUDE

    )

set (MODULE_DEF

    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)


