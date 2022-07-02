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
#ifndef MU_AUDIO_SYNTHTYPES_H
#define MU_AUDIO_SYNTHTYPES_H

#include <string>
#include <map>

#include "io/path.h"
#include "types/uri.h"

namespace mu::audio::synth {
using SoundFontPath = io::path_t;
using SoundFontPaths = std::vector<SoundFontPath>;

inline bool isSoundFont(const io::path_t& filePath)
{
    std::string ext = io::suffix(filePath);
    return ext == "sf2" || ext == "sf3";
}

enum class SynthType {
    Undefined = 0,
    Fluid,
    VSTi
};

struct SynthUri : public UriQuery
{
    SynthType type = SynthType::Undefined;

    SynthUri() = default;
    SynthUri(const SynthType& type)
        : UriQuery("musescore://audio/" + synthTypeToString(type)), type(type) {}

private:
    std::string synthTypeToString(const SynthType& synthType)
    {
        switch (synthType) {
        case SynthType::Fluid: {
            static std::string fluid("fluid");
            return fluid;
        }
        case SynthType::VSTi: {
            static std::string vsti("vsti");
            return vsti;
        }
        default:
            static std::string undefined("undefined");
            return undefined;
        }
    }
};

using SynthUriList = std::vector<SynthUri>;

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
