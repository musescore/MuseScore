/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#ifndef MUSE_AUDIO_SOUNDFONTTYPES_H
#define MUSE_AUDIO_SOUNDFONTTYPES_H

#include <map>
#include <string>
#include <vector>

#include "global/io/path.h"

#include "midi/miditypes.h"

namespace muse::audio::synth {
using SoundFontPath = io::path_t;
using SoundFontPaths = std::vector<SoundFontPath>;

inline bool isSoundFont(const io::path_t& filePath)
{
    std::string ext = io::suffix(filePath);
    return ext == "sf2" || ext == "sf3" || ext == "dls";
}

struct SoundFontPreset
{
    midi::Program program;
    std::string name;
};

struct SoundFontMeta
{
    SoundFontPath path;
    std::vector<SoundFontPreset> presets;
};

using SoundFontsMap = std::map<SoundFontPath, SoundFontMeta>;

inline const String SOUNDFONT_NAME_ATTRIBUTE(u"soundFontName");
inline const String PRESET_NAME_ATTRIBUTE(u"presetName");
inline const String PRESET_BANK_ATTRIBUTE(u"presetBank");
inline const String PRESET_PROGRAM_ATTRIBUTE(u"presetProgram");
}

#endif // MUSE_AUDIO_SOUNDFONTTYPES_H
