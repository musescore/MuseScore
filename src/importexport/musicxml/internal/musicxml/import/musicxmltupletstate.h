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
#pragma once

#include "global/types/flags.h"

#include "types/fraction.h"

namespace mu::engraving {
class TDuration;
}

namespace mu::iex::musicxml {
enum class MusicXmlStartStop : char;
enum class MusicXmlTupletFlag : char {
    NONE = 0,
    STOP_PREVIOUS = 1,
    START_NEW = 2,
    ADD_CHORD = 4,
    STOP_CURRENT = 8
};
typedef muse::Flags<MusicXmlTupletFlag> MusicXmlTupletFlags;

class MusicXmlTupletState
{
public:
    MusicXmlTupletFlags determineTupletAction(const engraving::Fraction noteDuration, const engraving::Fraction timeMod,
                                              const MusicXmlStartStop tupletStartStop, const engraving::TDuration normalType,
                                              engraving::Fraction& missingPreviousDuration, engraving::Fraction& missingCurrentDuration);
    static void determineTupletFractionAndFullDuration(const engraving::Fraction duration, engraving::Fraction& fraction,
                                                       engraving::Fraction& fullDuration);
    static engraving::Fraction missingTupletDuration(const engraving::Fraction duration);
private:
    void addDurationToTuplet(const engraving::Fraction duration, const engraving::Fraction timeMod);
    void smallestTypeAndCount(const engraving::TDuration durType, int& type, int& count);
    void matchTypeAndCount(int& noteType, int& noteCount);
    bool isTupletFilled(const engraving::TDuration normalType, const engraving::Fraction timeMod);

    bool inTuplet = false;
    bool implicit = false;
    int actualNotes = 1;
    int normalNotes = 1;
    engraving::Fraction duration { 0, 1 };
    int smallestNoteType = 0;   // smallest note type in the tuplet
    int smallestNoteCount = 0;   // number of smallest notes in the tuplet
};
using MusicXmlTupletStates = std::map<muse::String, MusicXmlTupletState>;
}
