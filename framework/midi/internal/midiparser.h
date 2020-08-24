//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_MIDI_MIDIPARSER_H
#define MU_MIDI_MIDIPARSER_H

#include <cstdint>

#include "../miditypes.h"

namespace mu {
namespace midi {
class MidiParser
{
public:

    static uint32_t toMessage(const Event& e);
    static Event toEvent(uint32_t msg);
};
}
}

#endif // MU_MIDI_MIDIPARSER_H
