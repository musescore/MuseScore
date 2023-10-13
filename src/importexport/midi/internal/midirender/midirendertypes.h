/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MU_IMPORTEXPORT_MIDIRENDERTYPES_H
#define MU_IMPORTEXPORT_MIDIRENDERTYPES_H

#include <unordered_map>
#include <map>
#include "log.h"

namespace mu::engraving {
class Chord;

class EngravingItem;

class Note;
}

namespace mu::iex::midi {
struct MidiRenderEvent {
    uint8_t type = 0;
    uint8_t channel = 0;     // ???
    uint8_t a = 0;
    uint8_t b = 0;
};
using midirender_channels_t = std::array<std::multimap<size_t, MidiRenderEvent>, 16>;     // midi can handle only 16 channels
struct MidiRenderOutData {
    std::unordered_map<uint64_t, midirender_channels_t> tracks{};
};

using notes_mmap_t = std::multimap<int, mu::engraving::Note*>;
struct MRPart {
    notes_mmap_t regularNotes;
    notes_mmap_t palmMuteNotes;
    notes_mmap_t graceNotesBefore;
    notes_mmap_t graceNotesAfter;
};

struct Meta {
    std::vector<mu::engraving::EngravingItem*> dependentItems;
    int pitch = 0;
    int noteOn = 0;
    int noteOff = 0;
    bool isGraceBefore = false;
    bool isGraceAfter = false;
    bool isPalmMute = false;
};

class RenderContext
{
    std::unordered_map<uint64_t, MRPart> m_parts;
public:
    virtual ~RenderContext() = default;

    std::unordered_map<uint64_t, MRPart>& parts()
    {
        return m_parts;
    }

    [[nodiscard]] const std::unordered_map<uint64_t, MRPart>& parts() const
    {
        return m_parts;
    }

    MRPart& part(uint64_t id)
    {
        IF_ASSERT_FAILED(parts().find(id) != parts().end()) {
            m_parts.insert({ 1, {} });
            return parts().at(1);
        }
        return parts().at(id);
    }
};
}

#endif //MU_IMPORTEXPORT_MIDIRENDERTYPES_H
