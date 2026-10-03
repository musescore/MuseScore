/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include "stringtuningsutils.h"

#include "engraving/dom/pitchspelling.h"
#include "engraving/dom/stringtunings.h"
#include "engraving/dom/utils.h"

using namespace mu;
using namespace muse;

namespace {
String noteInputAccidentalsToAscii(const String& value)
{
    String result;
    result.reserve(value.size());

    // muse::String does not expose iterators, so walk the UTF-16 buffer by index.
    for (size_t i = 0; i < value.size(); ++i) {
        const Char ch = value.at(i);
        if (ch == u'♭') {
            result += u'b';
        } else if (ch == u'♯') {
            result += u'#';
        } else if (ch != u' ') {
            result += ch;
        }
    }

    return result;
}

String normalizeLocalizedNoteInput(String value, mu::engraving::NoteSpellingType spelling)
{
    if (spelling != mu::engraving::NoteSpellingType::FRENCH) {
        return value;
    }

    value.replace(u'é', u'e');
    value.replace(u'É', u'E');
    value.remove(u'\u0301');

    return value;
}
}

QString mu::notation::stringTuningPitchToString(int pitch, bool useFlats, engraving::NoteSpellingType spelling)
{
    return engraving::stringTuningPitchName(pitch, useFlats, spelling, true).toQString();
}

int mu::notation::stringTuningInputToPitch(const String& input, engraving::NoteSpellingType spelling, bool* useFlat)
{
    using namespace mu::engraving;

    const String normalizedValue = normalizeLocalizedNoteInput(noteInputAccidentalsToAscii(input).trimmed(), spelling);
    if (normalizedValue.empty()) {
        return INVALID_PITCH;
    }

    size_t octaveDigitsStart = normalizedValue.size();
    while (octaveDigitsStart > 0 && normalizedValue.at(octaveDigitsStart - 1).isDigit()) {
        --octaveDigitsStart;
    }

    if (octaveDigitsStart == normalizedValue.size()) {
        return INVALID_PITCH;
    }

    size_t octaveStart = octaveDigitsStart;
    if (octaveStart > 0 && normalizedValue.at(octaveStart - 1) == u'-') {
        --octaveStart;
    }

    const String octaveString = normalizedValue.mid(octaveStart);
    const int octave = octaveString.toInt();
    if (octave < -1 || octave > 9) {
        return INVALID_PITCH;
    }

    const String noteName = normalizedValue.left(octaveStart);
    if (noteName.empty()) {
        return INVALID_PITCH;
    }

    NoteCaseType noteCase = NoteCaseType::AUTO;
    size_t parsedLength = 0;
    const int tpc = convertNote(noteName, spelling, noteCase, parsedLength);
    if (!tpcIsValid(tpc) || parsedLength != noteName.size()) {
        return INVALID_PITCH;
    }

    const int pitchClass = (tpc2pitch(tpc) + PITCH_DELTA_OCTAVE) % PITCH_DELTA_OCTAVE;
    const int pitch = (octave + 1) * PITCH_DELTA_OCTAVE + pitchClass;

    if (!pitchIsValid(pitch)) {
        return INVALID_PITCH;
    }

    if (useFlat) {
        *useFlat = static_cast<int>(tpc2alter(tpc)) < 0;
    }

    return pitch;
}
