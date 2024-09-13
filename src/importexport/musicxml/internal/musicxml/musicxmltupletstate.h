/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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
#ifndef MUSICXMLTUPLETSTATE_H
#define MUSICXMLTUPLETSTATE_H

#include "musicxmltypes.h"
#include "dom/durationtype.h"

namespace mu::engraving {
enum class MxmlTupletFlag : char {
    NONE = 0,
    STOP_PREVIOUS = 1,
    START_NEW = 2,
    ADD_CHORD = 4,
    STOP_CURRENT = 8
};
typedef muse::Flags<MxmlTupletFlag> MxmlTupletFlags;

class MxmlTupletState
{
public:
    MxmlTupletFlags determineTupletAction(const Fraction noteDuration, const Fraction timeMod, const MxmlStartStop tupletStartStop,
                                          const TDuration normalType, Fraction& missingPreviousDuration, Fraction& missingCurrentDuration);
private:
    void addDurationToTuplet(const Fraction duration, const Fraction timeMod);
    void smallestTypeAndCount(const TDuration durType, int& type, int& count);
    void matchTypeAndCount(int& noteType, int& noteCount);
    bool isTupletFilled(const TDuration normalType, const Fraction timeMod);

    bool inTuplet = false;
    bool implicit = false;
    int actualNotes = 1;
    int normalNotes = 1;
    Fraction duration { 0, 1 };
    int smallestNoteType = 0;   // smallest note type in the tuplet
    int smallestNoteCount = 0;   // number of smallest notes in the tuplet
};
using MxmlTupletStates = std::map<String, MxmlTupletState>;
}

#endif // MUSICXMLTUPLETSTATE_H
