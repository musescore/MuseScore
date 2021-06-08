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
#include "utils.h"

#include "translation.h"

using namespace mu;

static const std::vector<std::string> noteHeadNamesLower = {
    trc("global", "c"),
    trc("global", "c♯"),
    trc("global", "d"),
    trc("global", "d♯"),
    trc("global", "e"),
    trc("global", "f"),
    trc("global", "f♯"),
    trc("global", "g"),
    trc("global", "g♯"),
    trc("global", "a"),
    trc("global", "a♯"),
    trc("global", "b")
};

static const std::vector<std::string> noteHeadNamesUpper = {
    trc("global", "C"),
    trc("global", "C♯"),
    trc("global", "D"),
    trc("global", "D♯"),
    trc("global", "E"),
    trc("global", "F"),
    trc("global", "F♯"),
    trc("global", "G"),
    trc("global", "G♯"),
    trc("global", "A"),
    trc("global", "A♯"),
    trc("global", "B")
};

std::string mu::pitchToString(int pitch)
{
    if (pitch < 0 || pitch > 127) {
        return std::string();
    }

    int octave = (pitch / 12) - 1;
    int i = pitch % 12;
    return octave < 0 ? noteHeadNamesUpper[i] : (noteHeadNamesLower[i] + std::to_string(octave));
}
