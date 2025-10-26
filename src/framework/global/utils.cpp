/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

using namespace muse;

[[maybe_unused]]
static constexpr const char* noteNamesWithOctaves[] = {
    // octave -1
    QT_TRANSLATE_NOOP("global/pitchName", "C-1"),
    QT_TRANSLATE_NOOP("global/pitchName", "C♯-1"),
    QT_TRANSLATE_NOOP("global/pitchName", "D♭-1"),
    QT_TRANSLATE_NOOP("global/pitchName", "D-1"),
    QT_TRANSLATE_NOOP("global/pitchName", "D♯-1"),
    QT_TRANSLATE_NOOP("global/pitchName", "E♭-1"),
    QT_TRANSLATE_NOOP("global/pitchName", "E-1"),
    QT_TRANSLATE_NOOP("global/pitchName", "E♯-1"),
    QT_TRANSLATE_NOOP("global/pitchName", "F♭-1"),
    QT_TRANSLATE_NOOP("global/pitchName", "F-1"),
    QT_TRANSLATE_NOOP("global/pitchName", "F♯-1"),
    QT_TRANSLATE_NOOP("global/pitchName", "G♭-1"),
    QT_TRANSLATE_NOOP("global/pitchName", "G-1"),
    QT_TRANSLATE_NOOP("global/pitchName", "G♯-1"),
    QT_TRANSLATE_NOOP("global/pitchName", "A♭-1"),
    QT_TRANSLATE_NOOP("global/pitchName", "A-1"),
    QT_TRANSLATE_NOOP("global/pitchName", "A♯-1"),
    QT_TRANSLATE_NOOP("global/pitchName", "B♭-1"),
    QT_TRANSLATE_NOOP("global/pitchName", "B-1"),
    QT_TRANSLATE_NOOP("global/pitchName", "B♯-1"),
    // octave 0
    QT_TRANSLATE_NOOP("global/pitchName", "C♭0"),
    QT_TRANSLATE_NOOP("global/pitchName", "C0"),
    QT_TRANSLATE_NOOP("global/pitchName", "C♯0"),
    QT_TRANSLATE_NOOP("global/pitchName", "D♭0"),
    QT_TRANSLATE_NOOP("global/pitchName", "D0"),
    QT_TRANSLATE_NOOP("global/pitchName", "D♯0"),
    QT_TRANSLATE_NOOP("global/pitchName", "E♭0"),
    QT_TRANSLATE_NOOP("global/pitchName", "E0"),
    QT_TRANSLATE_NOOP("global/pitchName", "E♯0"),
    QT_TRANSLATE_NOOP("global/pitchName", "F♭0"),
    QT_TRANSLATE_NOOP("global/pitchName", "F0"),
    QT_TRANSLATE_NOOP("global/pitchName", "F♯0"),
    QT_TRANSLATE_NOOP("global/pitchName", "G♭0"),
    QT_TRANSLATE_NOOP("global/pitchName", "G0"),
    QT_TRANSLATE_NOOP("global/pitchName", "G♯0"),
    QT_TRANSLATE_NOOP("global/pitchName", "A♭0"),
    QT_TRANSLATE_NOOP("global/pitchName", "A0"),
    QT_TRANSLATE_NOOP("global/pitchName", "A♯0"),
    QT_TRANSLATE_NOOP("global/pitchName", "B♭0"),
    QT_TRANSLATE_NOOP("global/pitchName", "B0"),
    QT_TRANSLATE_NOOP("global/pitchName", "B♯0"),
    // octave 1
    QT_TRANSLATE_NOOP("global/pitchName", "C♭1"),
    QT_TRANSLATE_NOOP("global/pitchName", "C1"),
    QT_TRANSLATE_NOOP("global/pitchName", "C♯1"),
    QT_TRANSLATE_NOOP("global/pitchName", "D♭1"),
    QT_TRANSLATE_NOOP("global/pitchName", "D1"),
    QT_TRANSLATE_NOOP("global/pitchName", "D♯1"),
    QT_TRANSLATE_NOOP("global/pitchName", "E♭1"),
    QT_TRANSLATE_NOOP("global/pitchName", "E1"),
    QT_TRANSLATE_NOOP("global/pitchName", "E♯1"),
    QT_TRANSLATE_NOOP("global/pitchName", "F♭1"),
    QT_TRANSLATE_NOOP("global/pitchName", "F1"),
    QT_TRANSLATE_NOOP("global/pitchName", "F♯1"),
    QT_TRANSLATE_NOOP("global/pitchName", "G♭1"),
    QT_TRANSLATE_NOOP("global/pitchName", "G1"),
    QT_TRANSLATE_NOOP("global/pitchName", "G♯1"),
    QT_TRANSLATE_NOOP("global/pitchName", "A♭1"),
    QT_TRANSLATE_NOOP("global/pitchName", "A1"),
    QT_TRANSLATE_NOOP("global/pitchName", "A♯1"),
    QT_TRANSLATE_NOOP("global/pitchName", "B♭1"),
    QT_TRANSLATE_NOOP("global/pitchName", "B1"),
    QT_TRANSLATE_NOOP("global/pitchName", "B♯1"),
    // octave 2
    QT_TRANSLATE_NOOP("global/pitchName", "C♭2"),
    QT_TRANSLATE_NOOP("global/pitchName", "C2"),
    QT_TRANSLATE_NOOP("global/pitchName", "C♯2"),
    QT_TRANSLATE_NOOP("global/pitchName", "D♭2"),
    QT_TRANSLATE_NOOP("global/pitchName", "D2"),
    QT_TRANSLATE_NOOP("global/pitchName", "D♯2"),
    QT_TRANSLATE_NOOP("global/pitchName", "E♭2"),
    QT_TRANSLATE_NOOP("global/pitchName", "E2"),
    QT_TRANSLATE_NOOP("global/pitchName", "E♯2"),
    QT_TRANSLATE_NOOP("global/pitchName", "F♭2"),
    QT_TRANSLATE_NOOP("global/pitchName", "F2"),
    QT_TRANSLATE_NOOP("global/pitchName", "F♯2"),
    QT_TRANSLATE_NOOP("global/pitchName", "G♭2"),
    QT_TRANSLATE_NOOP("global/pitchName", "G2"),
    QT_TRANSLATE_NOOP("global/pitchName", "G♯2"),
    QT_TRANSLATE_NOOP("global/pitchName", "A♭2"),
    QT_TRANSLATE_NOOP("global/pitchName", "A2"),
    QT_TRANSLATE_NOOP("global/pitchName", "A♯2"),
    QT_TRANSLATE_NOOP("global/pitchName", "B♭2"),
    QT_TRANSLATE_NOOP("global/pitchName", "B2"),
    QT_TRANSLATE_NOOP("global/pitchName", "B♯2"),
    // octave 3
    QT_TRANSLATE_NOOP("global/pitchName", "C♭3"),
    QT_TRANSLATE_NOOP("global/pitchName", "C3"),
    QT_TRANSLATE_NOOP("global/pitchName", "C♯3"),
    QT_TRANSLATE_NOOP("global/pitchName", "D♭3"),
    QT_TRANSLATE_NOOP("global/pitchName", "D3"),
    QT_TRANSLATE_NOOP("global/pitchName", "D♯3"),
    QT_TRANSLATE_NOOP("global/pitchName", "E♭3"),
    QT_TRANSLATE_NOOP("global/pitchName", "E3"),
    QT_TRANSLATE_NOOP("global/pitchName", "E♯3"),
    QT_TRANSLATE_NOOP("global/pitchName", "F♭3"),
    QT_TRANSLATE_NOOP("global/pitchName", "F3"),
    QT_TRANSLATE_NOOP("global/pitchName", "F♯3"),
    QT_TRANSLATE_NOOP("global/pitchName", "G♭3"),
    QT_TRANSLATE_NOOP("global/pitchName", "G3"),
    QT_TRANSLATE_NOOP("global/pitchName", "G♯3"),
    QT_TRANSLATE_NOOP("global/pitchName", "A♭3"),
    QT_TRANSLATE_NOOP("global/pitchName", "A3"),
    QT_TRANSLATE_NOOP("global/pitchName", "A♯3"),
    QT_TRANSLATE_NOOP("global/pitchName", "B♭3"),
    QT_TRANSLATE_NOOP("global/pitchName", "B3"),
    QT_TRANSLATE_NOOP("global/pitchName", "B♯3"),
    // octave 4
    QT_TRANSLATE_NOOP("global/pitchName", "C♭4"),
    QT_TRANSLATE_NOOP("global/pitchName", "C4"),
    QT_TRANSLATE_NOOP("global/pitchName", "C♯4"),
    QT_TRANSLATE_NOOP("global/pitchName", "D♭4"),
    QT_TRANSLATE_NOOP("global/pitchName", "D4"),
    QT_TRANSLATE_NOOP("global/pitchName", "D♯4"),
    QT_TRANSLATE_NOOP("global/pitchName", "E♭4"),
    QT_TRANSLATE_NOOP("global/pitchName", "E4"),
    QT_TRANSLATE_NOOP("global/pitchName", "E♯4"),
    QT_TRANSLATE_NOOP("global/pitchName", "F♭4"),
    QT_TRANSLATE_NOOP("global/pitchName", "F4"),
    QT_TRANSLATE_NOOP("global/pitchName", "F♯4"),
    QT_TRANSLATE_NOOP("global/pitchName", "G♭4"),
    QT_TRANSLATE_NOOP("global/pitchName", "G4"),
    QT_TRANSLATE_NOOP("global/pitchName", "G♯4"),
    QT_TRANSLATE_NOOP("global/pitchName", "A♭4"),
    QT_TRANSLATE_NOOP("global/pitchName", "A4"),
    QT_TRANSLATE_NOOP("global/pitchName", "A♯4"),
    QT_TRANSLATE_NOOP("global/pitchName", "B♭4"),
    QT_TRANSLATE_NOOP("global/pitchName", "B4"),
    QT_TRANSLATE_NOOP("global/pitchName", "B♯4"),
    // octave 5
    QT_TRANSLATE_NOOP("global/pitchName", "C♭5"),
    QT_TRANSLATE_NOOP("global/pitchName", "C5"),
    QT_TRANSLATE_NOOP("global/pitchName", "C♯5"),
    QT_TRANSLATE_NOOP("global/pitchName", "D♭5"),
    QT_TRANSLATE_NOOP("global/pitchName", "D5"),
    QT_TRANSLATE_NOOP("global/pitchName", "D♯5"),
    QT_TRANSLATE_NOOP("global/pitchName", "E♭5"),
    QT_TRANSLATE_NOOP("global/pitchName", "E5"),
    QT_TRANSLATE_NOOP("global/pitchName", "E♯5"),
    QT_TRANSLATE_NOOP("global/pitchName", "F♭5"),
    QT_TRANSLATE_NOOP("global/pitchName", "F5"),
    QT_TRANSLATE_NOOP("global/pitchName", "F♯5"),
    QT_TRANSLATE_NOOP("global/pitchName", "G♭5"),
    QT_TRANSLATE_NOOP("global/pitchName", "G5"),
    QT_TRANSLATE_NOOP("global/pitchName", "G♯5"),
    QT_TRANSLATE_NOOP("global/pitchName", "A♭5"),
    QT_TRANSLATE_NOOP("global/pitchName", "A5"),
    QT_TRANSLATE_NOOP("global/pitchName", "A♯5"),
    QT_TRANSLATE_NOOP("global/pitchName", "B♭5"),
    QT_TRANSLATE_NOOP("global/pitchName", "B5"),
    QT_TRANSLATE_NOOP("global/pitchName", "B♯5"),
    // octave 6
    QT_TRANSLATE_NOOP("global/pitchName", "C♭6"),
    QT_TRANSLATE_NOOP("global/pitchName", "C6"),
    QT_TRANSLATE_NOOP("global/pitchName", "C♯6"),
    QT_TRANSLATE_NOOP("global/pitchName", "D♭6"),
    QT_TRANSLATE_NOOP("global/pitchName", "D6"),
    QT_TRANSLATE_NOOP("global/pitchName", "D♯6"),
    QT_TRANSLATE_NOOP("global/pitchName", "E♭6"),
    QT_TRANSLATE_NOOP("global/pitchName", "E6"),
    QT_TRANSLATE_NOOP("global/pitchName", "E♯6"),
    QT_TRANSLATE_NOOP("global/pitchName", "F♭6"),
    QT_TRANSLATE_NOOP("global/pitchName", "F6"),
    QT_TRANSLATE_NOOP("global/pitchName", "F♯6"),
    QT_TRANSLATE_NOOP("global/pitchName", "G♭6"),
    QT_TRANSLATE_NOOP("global/pitchName", "G6"),
    QT_TRANSLATE_NOOP("global/pitchName", "G♯6"),
    QT_TRANSLATE_NOOP("global/pitchName", "A♭6"),
    QT_TRANSLATE_NOOP("global/pitchName", "A6"),
    QT_TRANSLATE_NOOP("global/pitchName", "A♯6"),
    QT_TRANSLATE_NOOP("global/pitchName", "B♭6"),
    QT_TRANSLATE_NOOP("global/pitchName", "B6"),
    QT_TRANSLATE_NOOP("global/pitchName", "B♯6"),
    // octave 7
    QT_TRANSLATE_NOOP("global/pitchName", "C♭7"),
    QT_TRANSLATE_NOOP("global/pitchName", "C7"),
    QT_TRANSLATE_NOOP("global/pitchName", "C♯7"),
    QT_TRANSLATE_NOOP("global/pitchName", "D♭7"),
    QT_TRANSLATE_NOOP("global/pitchName", "D7"),
    QT_TRANSLATE_NOOP("global/pitchName", "D♯7"),
    QT_TRANSLATE_NOOP("global/pitchName", "E♭7"),
    QT_TRANSLATE_NOOP("global/pitchName", "E7"),
    QT_TRANSLATE_NOOP("global/pitchName", "E♯7"),
    QT_TRANSLATE_NOOP("global/pitchName", "F♭7"),
    QT_TRANSLATE_NOOP("global/pitchName", "F7"),
    QT_TRANSLATE_NOOP("global/pitchName", "F♯7"),
    QT_TRANSLATE_NOOP("global/pitchName", "G♭7"),
    QT_TRANSLATE_NOOP("global/pitchName", "G7"),
    QT_TRANSLATE_NOOP("global/pitchName", "G♯7"),
    QT_TRANSLATE_NOOP("global/pitchName", "A♭7"),
    QT_TRANSLATE_NOOP("global/pitchName", "A7"),
    QT_TRANSLATE_NOOP("global/pitchName", "A♯7"),
    QT_TRANSLATE_NOOP("global/pitchName", "B♭7"),
    QT_TRANSLATE_NOOP("global/pitchName", "B7"),
    QT_TRANSLATE_NOOP("global/pitchName", "B♯7"),
    // octave 8
    QT_TRANSLATE_NOOP("global/pitchName", "C♭8"),
    QT_TRANSLATE_NOOP("global/pitchName", "C8"),
    QT_TRANSLATE_NOOP("global/pitchName", "C♯8"),
    QT_TRANSLATE_NOOP("global/pitchName", "D♭8"),
    QT_TRANSLATE_NOOP("global/pitchName", "D8"),
    QT_TRANSLATE_NOOP("global/pitchName", "D♯8"),
    QT_TRANSLATE_NOOP("global/pitchName", "E♭8"),
    QT_TRANSLATE_NOOP("global/pitchName", "E8"),
    QT_TRANSLATE_NOOP("global/pitchName", "E♯8"),
    QT_TRANSLATE_NOOP("global/pitchName", "F♭8"),
    QT_TRANSLATE_NOOP("global/pitchName", "F8"),
    QT_TRANSLATE_NOOP("global/pitchName", "F♯8"),
    QT_TRANSLATE_NOOP("global/pitchName", "G♭8"),
    QT_TRANSLATE_NOOP("global/pitchName", "G8"),
    QT_TRANSLATE_NOOP("global/pitchName", "G♯8"),
    QT_TRANSLATE_NOOP("global/pitchName", "A♭8"),
    QT_TRANSLATE_NOOP("global/pitchName", "A8"),
    QT_TRANSLATE_NOOP("global/pitchName", "A♯8"),
    QT_TRANSLATE_NOOP("global/pitchName", "B♭8"),
    QT_TRANSLATE_NOOP("global/pitchName", "B8"),
    QT_TRANSLATE_NOOP("global/pitchName", "B♯8"),
    // octave 9
    QT_TRANSLATE_NOOP("global/pitchName", "C♭9"),
    QT_TRANSLATE_NOOP("global/pitchName", "C9"),
    QT_TRANSLATE_NOOP("global/pitchName", "C♯9"),
    QT_TRANSLATE_NOOP("global/pitchName", "D♭9"),
    QT_TRANSLATE_NOOP("global/pitchName", "D9"),
    QT_TRANSLATE_NOOP("global/pitchName", "D♯9"),
    QT_TRANSLATE_NOOP("global/pitchName", "E♭9"),
    QT_TRANSLATE_NOOP("global/pitchName", "E9"),
    QT_TRANSLATE_NOOP("global/pitchName", "E♯9"),
    QT_TRANSLATE_NOOP("global/pitchName", "F♭9"),
    QT_TRANSLATE_NOOP("global/pitchName", "F9"),
    QT_TRANSLATE_NOOP("global/pitchName", "F♯9"),
    QT_TRANSLATE_NOOP("global/pitchName", "G♭9"),
    QT_TRANSLATE_NOOP("global/pitchName", "G9"),
};

static constexpr const char* sharpNotes[] = {
    QT_TRANSLATE_NOOP("global/noteName", "C"),
    QT_TRANSLATE_NOOP("global/noteName", "C♯"),
    QT_TRANSLATE_NOOP("global/noteName", "D"),
    QT_TRANSLATE_NOOP("global/noteName", "D♯"),
    QT_TRANSLATE_NOOP("global/noteName", "E"),
    QT_TRANSLATE_NOOP("global/noteName", "F"),
    QT_TRANSLATE_NOOP("global/noteName", "F♯"),
    QT_TRANSLATE_NOOP("global/noteName", "G"),
    QT_TRANSLATE_NOOP("global/noteName", "G♯"),
    QT_TRANSLATE_NOOP("global/noteName", "A"),
    QT_TRANSLATE_NOOP("global/noteName", "A♯"),
    QT_TRANSLATE_NOOP("global/noteName", "B")
};

static constexpr const char* flatNotes[] = {
    QT_TRANSLATE_NOOP("global/noteName", "C"),
    QT_TRANSLATE_NOOP("global/noteName", "D♭"),
    QT_TRANSLATE_NOOP("global/noteName", "D"),
    QT_TRANSLATE_NOOP("global/noteName", "E♭"),
    QT_TRANSLATE_NOOP("global/noteName", "E"),
    QT_TRANSLATE_NOOP("global/noteName", "F"),
    QT_TRANSLATE_NOOP("global/noteName", "G♭"),
    QT_TRANSLATE_NOOP("global/noteName", "G"),
    QT_TRANSLATE_NOOP("global/noteName", "A♭"),
    QT_TRANSLATE_NOOP("global/noteName", "A"),
    QT_TRANSLATE_NOOP("global/noteName", "B♭"),
    QT_TRANSLATE_NOOP("global/noteName", "B")
};

std::string muse::pitchToString(int pitch, bool addoctave, bool useFlats /* = false */)
{
    if (pitch < 0 || pitch > 127) {
        return std::string();
    }

    auto source = useFlats ? flatNotes : sharpNotes;

    int i = pitch % 12;
    if (addoctave) {
        int octave = (pitch / 12) - 1;
        std::string key = std::string(source[i]) + std::to_string(octave);
        return muse::trc("global/pitchName", key.c_str());
    }
    return muse::trc("global/noteName", source[i]);
}
