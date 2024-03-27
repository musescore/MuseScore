/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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

#ifndef MUSE_MUSESAMPLER_MUSESAMPLERTYPES_H
#define MUSE_MUSESAMPLER_MUSESAMPLERTYPES_H

#include "types/string.h"

namespace muse::musesampler {
enum class ClefType {
    None,
    Treble,
    Bass,
    Alto,
    Tenor,
    Percussion,
    HigherOctaveTreble,
    LowerOctaveTreble,
    HigherOctaveBass,
    LowerOctaveBass,
    Baritone,
    Mezzosoprano,
    Soprano,
    FrenchViolin,
};

enum class StaffType {
    Standard,
    Grand,
};

struct Instrument {
    String id;
    String soundId;
    String musicXmlId;
    String name;
    String abbreviation;
    String category;
    String vendor;
    size_t staffLines = 0;
    StaffType staffType = StaffType::Standard;
    ClefType clefType = ClefType::None;
};
}

#endif // MUSE_MUSESAMPLER_MUSESAMPLERTYPES_H
