//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_AUDIO_SYNTHTYPES_H
#define MU_AUDIO_SYNTHTYPES_H

#include <string>
#include <map>

#include "midi/miditypes.h"

namespace mu::audio::synth {
static const unsigned int AUDIO_CHANNELS = 2;

using SynthName = midi::SynthName;
using SynthMap = midi::SynthMap;

enum class SoundFontFormat {
    Undefined = 0,
    SF2,
    SF3,
    SFZ,
    Embedded
};
using SoundFontFormats = std::set<SoundFontFormat>;

struct SynthesizerState {
    enum class ValID {
        UndefinedID = -1,
        SoundFontID = 0,
    };

    struct Val {
        ValID id = ValID::UndefinedID;
        std::string val;
        Val() = default;
        Val(ValID id, const std::string& val)
            : id(id), val(val) {}

        bool operator ==(const Val& other) const { return other.id == id && other.val == val; }
        bool operator !=(const Val& other) const { return !operator ==(other); }
    };

    struct Group {
        std::string name;
        std::vector<Val> vals;

        bool isValid() const { return !name.empty(); }

        bool operator ==(const Group& other) const { return other.name == name && other.vals == vals; }
        bool operator !=(const Group& other) const { return !operator ==(other); }
    };

    std::map<std::string, Group> groups;

    bool isNull() const { return groups.empty(); }
};
}

#endif // MU_AUDIO_SYNTHTYPES_H
