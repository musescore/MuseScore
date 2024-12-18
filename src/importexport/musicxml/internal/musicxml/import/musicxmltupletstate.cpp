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
#include "musicxmltupletstate.h"
#include "dom/tuplet.h"
#include "internal/musicxml/shared/musicxmltypes.h"

using namespace mu::engraving;
using namespace mu::iex::musicxml;

//---------------------------------------------------------
//   determineTupletFractionAndFullDuration
//---------------------------------------------------------

/**
 Split duration into two factors where fullDuration is note sized
 (i.e. the denominator is a power of 2), 1/2 < fraction <= 1/1
 and fraction * fullDuration equals duration.
 */

void MusicXmlTupletState::determineTupletFractionAndFullDuration(const Fraction duration, Fraction& fraction, Fraction& fullDuration)
{
    fraction = duration;
    fullDuration = Fraction(1, 1);
    // move denominator's powers of 2 from fraction to fullDuration
    while (fraction.denominator() % 2 == 0) {
        fraction *= 2;
        fraction.reduce();
        fullDuration *= Fraction(1, 2);
    }
    // move numerator's powers of 2 from fraction to fullDuration
    while (fraction.numerator() % 2 == 0) {
        fraction *= Fraction(1, 2);
        fraction.reduce();
        fullDuration *= 2;
        fullDuration.reduce();
    }
    // make sure 1/2 < fraction <= 1/1
    while (fraction <= Fraction(1, 2)) {
        fullDuration *= Fraction(1, 2);
        fraction *= 2;
    }
    fullDuration.reduce();
    fraction.reduce();

    /*
    Examples (note result when denominator is not a power of two):
    3:2 tuplet of 1/4 results in fraction 1/1 and fullDuration 1/2
    2:3 tuplet of 1/4 results in fraction 3/1 and fullDuration 1/4
    4:3 tuplet of 1/4 results in fraction 3/1 and fullDuration 1/4
    3:4 tuplet of 1/4 results in fraction 1/1 and fullDuration 1/1

     Bring back fraction in 1/2 .. 1/1 range.
     */

    if (fraction > Fraction(1, 1) && fraction.denominator() == 1) {
        fullDuration *= fraction;
        fullDuration.reduce();
        fraction = Fraction(1, 1);
    }

    /*
    LOGD("duration %s fraction %s fullDuration %s",
           muPrintable(duration.toString()),
           muPrintable(fraction.toString()),
           muPrintable(fullDuration.toString())
           );
    */
}

//---------------------------------------------------------
//   missingTupletDuration
//---------------------------------------------------------

Fraction MusicXmlTupletState::missingTupletDuration(const Fraction duration)
{
    Fraction tupletFraction;
    Fraction tupletFullDuration;

    determineTupletFractionAndFullDuration(duration, tupletFraction, tupletFullDuration);
    Fraction missing = (Fraction(1, 1) - tupletFraction) * tupletFullDuration;

    return missing;
}

//---------------------------------------------------------
//   isTupletFilled
//---------------------------------------------------------

/**
 Determine if the tuplet is completely filled,
 because either (1) it is at least the same duration
 as the specified number of the specified normal type notes
 or (2) the duration adds up to a normal note duration.

 Example (1): a 3:2 tuplet with a 1/4 and a 1/8 note
 is filled if normal type is 1/8,
 it is not filled if normal type is 1/4.

 Example (2): a 3:2 tuplet with a 1/4 and a 1/8 note is filled.
 */

bool MusicXmlTupletState::isTupletFilled(const TDuration normalType, const Fraction timeMod)
{
    UNUSED(timeMod);
    bool res = false;
    /*
    const auto normalNotes = state.m_normalNotes;
    LOGD("duration %s normalType %s timeMod %s normalNotes %d actualNotes %d",
           muPrintable(state.m_duration.toString()),
           muPrintable(normalType.fraction().toString()),
           muPrintable(timeMod.toString()),
           normalNotes,
           actualNotes
           );
    */

    if (normalType.isValid()) {
        int matchedNormalType  = int(normalType.type());
        int matchedNormalCount = actualNotes;
        // match the types
        matchTypeAndCount(matchedNormalType, matchedNormalCount);
        // ... result scenario (1)
        res = smallestNoteCount >= matchedNormalCount;
        /*
        LOGD("normalType valid tupletType %d tupletCount %d matchedNormalType %d matchedNormalCount %d res %d",
               tupletType,
               tupletCount,
               matchedNormalType,
               matchedNormalCount,
               res
               );
         */
    } else {
        // ... result scenario (2)
        res = smallestNoteCount >= actualNotes;
        /*
        LOGD("normalType not valid tupletCount %d actualNotes %d res %d",
               tupletCount,
               actualNotes,
               res
               );
         */
    }
    return res;
}

//---------------------------------------------------------
//   smallestTypeAndCount
//---------------------------------------------------------

/**
 Determine the smallest note type and the number of those
 present in a ChordRest.
 For a note without dots the type equals the note type
 and count is one.
 For a single dotted note the type equals half the note type
 and count is three.
 A double dotted note is similar.
 Note: code assumes when duration().type() is incremented,
 the note length is divided by two, checked by tupletAssert().
 */

void MusicXmlTupletState::smallestTypeAndCount(const TDuration durType, int& type, int& count)
{
    type = int(durType.type());
    count = 1;
    switch (durType.dots()) {
    case 0:
        // nothing to do
        break;
    case 1:
        type += 1;                 // next-smaller type
        count = 3;
        break;
    case 2:
        type += 2;                 // next-next-smaller type
        count = 7;
        break;
    default:
        LOGD("smallestTypeAndCount() does not support more than 2 dots");
    }
}

//---------------------------------------------------------
//   matchTypeAndCount
//---------------------------------------------------------

/**
 Given two note types and counts, if the types are not equal,
 make them equal by successively doubling the count of the
 largest type.
 */

void MusicXmlTupletState::matchTypeAndCount(int& noteType, int& noteCount)
{
    while (smallestNoteType < noteType) {
        smallestNoteType++;
        smallestNoteCount *= 2;
    }
    while (noteType < smallestNoteType) {
        noteType++;
        noteCount *= 2;
    }
}

//---------------------------------------------------------
//   addDurationToTuplet
//---------------------------------------------------------

/**
 Add duration to tuplet duration
 Determine type and number of smallest notes in the tuplet
 */

void MusicXmlTupletState::addDurationToTuplet(const Fraction dur, const Fraction timeMod)
{
    /*
    LOGD("1 duration %s timeMod %s -> state.tupletType %d state.tupletCount %d state.actualNotes %d state.normalNotes %d",
           muPrintable(duration.print()),
           muPrintable(timeMod.print()),
           m_tupletType,
           m_tupletCount,
           m_actualNotes,
           m_normalNotes
           );
    */
    if (duration <= Fraction(0, 1)) {
        // first note: init variables
        actualNotes = timeMod.denominator();
        normalNotes = timeMod.numerator();
        smallestTypeAndCount(dur / timeMod, smallestNoteType, smallestNoteCount);
    } else {
        int noteType = 0;
        int noteCount = 0;
        smallestTypeAndCount(dur / timeMod, noteType, noteCount);
        // match the types
        matchTypeAndCount(noteType, noteCount);
        smallestNoteCount += noteCount;
    }
    duration += dur;
    /*
    LOGD("2 duration %s -> state.tupletType %d state.tupletCount %d state.actualNotes %d state.normalNotes %d",
           muPrintable(duration.print()),
           m_tupletType,
           m_tupletCount,
           m_actualNotes,
           m_normalNotes
           );
    */
}

//---------------------------------------------------------
//   determineTupletAction
//---------------------------------------------------------

/**
 Update tuplet state using parse result tupletDesc.
 Tuplets with <actual-notes> and <normal-notes> but without <tuplet>
 are handled correctly.
 TODO Nested tuplets are not (yet) supported.
 */

MusicXmlTupletFlags MusicXmlTupletState::determineTupletAction(const Fraction noteDuration,
                                                               const Fraction timeMod,
                                                               const MusicXmlStartStop tupletStartStop,
                                                               const TDuration normalType,
                                                               Fraction& missingPreviousDuration,
                                                               Fraction& missingCurrentDuration)
{
    const int actNotes = timeMod.denominator();
    const int norNotes = timeMod.numerator();
    MusicXmlTupletFlags res = MusicXmlTupletFlag::NONE;

    // check for unexpected termination of previous tuplet
    if (inTuplet && timeMod == Fraction(1, 1)) {
        // recover by simply stopping the current tuplet first
        if (!isTupletFilled(normalType, timeMod)) {
            missingPreviousDuration = missingTupletDuration(duration);
            //LOGD("tuplet incomplete, missing %s", muPrintable(missingPreviousDuration.print()));
        }
        *this = {};
        res |= MusicXmlTupletFlag::STOP_PREVIOUS;
    }

    // check for obvious errors
    if (inTuplet && tupletStartStop == MusicXmlStartStop::START) {
        LOGD("tuplet already started");
        // recover by simply stopping the current tuplet first
        if (!isTupletFilled(normalType, timeMod)) {
            missingPreviousDuration = missingTupletDuration(duration);
            //LOGD("tuplet incomplete, missing %s", muPrintable(missingPreviousDuration.print()));
        }
        *this = {};
        res |= MusicXmlTupletFlag::STOP_PREVIOUS;
    }
    if (tupletStartStop == MusicXmlStartStop::STOP && !inTuplet) {
        LOGD("tuplet stop but no tuplet started");           // TODO
        // recovery handled later (automatically, no special case needed)
    }

    // Tuplet are either started by the tuplet start
    // or when the time modification is first found.
    if (!inTuplet) {
        if (tupletStartStop == MusicXmlStartStop::START
            || (!inTuplet && (actNotes != 1 || norNotes != 1))) {
            if (tupletStartStop != MusicXmlStartStop::START) {
                implicit = true;
            } else {
                implicit = false;
            }
            // create a new tuplet
            inTuplet = true;
            res |= MusicXmlTupletFlag::START_NEW;
        }
    }

    // Add chord to the current tuplet.
    // Must also check for actual/normal notes to prevent
    // adding one chord too much if tuplet stop is missing.
    if (inTuplet && !(actNotes == 1 && norNotes == 1)) {
        addDurationToTuplet(noteDuration, timeMod);
        res |= MusicXmlTupletFlag::ADD_CHORD;
    }

    // Tuplets are stopped by the tuplet stop
    // or when the tuplet is filled completely
    // (either with knowledge of the normal type
    // or as a last resort calculated based on
    // actual and normal notes plus total duration)
    // or when the time-modification is not found.

    if (inTuplet) {
        if (tupletStartStop == MusicXmlStartStop::STOP
            || (implicit && isTupletFilled(normalType, timeMod))
            || (actNotes == 1 && norNotes == 1)) {           // incorrect ??? check scenario incomplete tuplet w/o start
            if (actNotes > norNotes && !isTupletFilled(normalType, timeMod)) {
                missingCurrentDuration = missingTupletDuration(duration);
                LOGD("current tuplet incomplete, missing %s", muPrintable(missingCurrentDuration.toString()));
            }

            *this = {};
            res |= MusicXmlTupletFlag::STOP_CURRENT;
        }
    }

    return res;
}
