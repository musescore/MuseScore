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

static const char* noteHeadNamesLower[] = {
    QT_TRANSLATE_NOOP("global", "c"),
    QT_TRANSLATE_NOOP("global", "c♯"),
    QT_TRANSLATE_NOOP("global", "d"),
    QT_TRANSLATE_NOOP("global", "d♯"),
    QT_TRANSLATE_NOOP("global", "e"),
    QT_TRANSLATE_NOOP("global", "f"),
    QT_TRANSLATE_NOOP("global", "f♯"),
    QT_TRANSLATE_NOOP("global", "g"),
    QT_TRANSLATE_NOOP("global", "g♯"),
    QT_TRANSLATE_NOOP("global", "a"),
    QT_TRANSLATE_NOOP("global", "a♯"),
    QT_TRANSLATE_NOOP("global", "b")
};

static const char* noteHeadNamesUpper[] = {
    QT_TRANSLATE_NOOP("global", "C"),
    QT_TRANSLATE_NOOP("global", "C♯"),
    QT_TRANSLATE_NOOP("global", "D"),
    QT_TRANSLATE_NOOP("global", "D♯"),
    QT_TRANSLATE_NOOP("global", "E"),
    QT_TRANSLATE_NOOP("global", "F"),
    QT_TRANSLATE_NOOP("global", "F♯"),
    QT_TRANSLATE_NOOP("global", "G"),
    QT_TRANSLATE_NOOP("global", "G♯"),
    QT_TRANSLATE_NOOP("global", "A"),
    QT_TRANSLATE_NOOP("global", "A♯"),
    QT_TRANSLATE_NOOP("global", "B")
};

std::string mu::pitchToString(int pitch)
{
    if (pitch < 0 || pitch > 127) {
        return std::string();
    }

    int octave = (pitch / 12) - 1;
    int i = pitch % 12;

    std::string result;
    if (octave < 0) {
        result = trc("global", noteHeadNamesUpper[i]);
    } else {
        result = trc("global", noteHeadNamesLower[i]) + std::to_string(octave);
    }

    return result;
}
