/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "durationtype.h"
#include "mscore.h"
#include "note.h"
#include "sig.h"
#include "measure.h"

#include "log.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   dots
//---------------------------------------------------------

static int getDots(int base, int rest, int* dots)
{
    if (base < 16) {
        return rest;
    }
    *dots = 0;
    if (rest >= base / 2) {
        *dots = *dots + 1;
        rest -= base / 2;
    }
    if (rest >= base / 4) {
        *dots = *dots + 1;
        rest -= base / 4;
    }
    if (rest >= base / 8) {
        *dots = *dots + 1;
        rest -= base / 8;
    }
    if (rest >= base / 16) {
        *dots = *dots + 1;
        rest -= base / 16;
    }
    if (*dots > MAX_DOTS) {
        *dots = MAX_DOTS;
    }
    return rest;
}

//---------------------------------------------------------
//   setDots
//---------------------------------------------------------

void TDuration::setDots(int v)
{
    if (v > MAX_DOTS) {
        v = MAX_DOTS;
    }
    if (v < 0) {
        v = 0;
    }
    m_dots = v;
}

//---------------------------------------------------------
//   setVal
//---------------------------------------------------------

void TDuration::setVal(int ticks)
{
    if (ticks == 0) {
        m_val = DurationType::V_MEASURE;
    } else {
        TDuration dt;
        for (int i = 0; i < int(DurationType::V_ZERO); ++i) {
            dt.setType(DurationType(i));
            int t = dt.ticks().ticks();
            if (ticks / t) {
                int remain = ticks % t;
                const int SMALLEST_DOT_DIVISOR = 1 << MAX_DOTS;
                if ((t - remain) < (t / SMALLEST_DOT_DIVISOR)) {
                    m_val = DurationType(i - 1);
                    return;
                }
                m_val = DurationType(i);
                getDots(t, remain, &m_dots);
                return;
            }
        }
        LOGD("2: no duration type for ticks %d", ticks);
        m_val = DurationType::V_QUARTER;           // fallback default value
    }
}

//---------------------------------------------------------
//   ticks
//---------------------------------------------------------

Fraction TDuration::ticks() const
{
    Fraction t;
    switch (m_val) {
    case DurationType::V_QUARTER: t = Fraction(1, 4);
        break;
    case DurationType::V_1024TH:  t = Fraction(1, 1024);
        break;
    case DurationType::V_512TH:   t = Fraction(1, 512);
        break;
    case DurationType::V_256TH:   t = Fraction(1, 256);
        break;
    case DurationType::V_128TH:   t = Fraction(1, 128);
        break;
    case DurationType::V_64TH:    t = Fraction(1, 64);
        break;
    case DurationType::V_32ND:    t = Fraction(1, 32);
        break;
    case DurationType::V_16TH:    t = Fraction(1, 16);
        break;
    case DurationType::V_EIGHTH:  t = Fraction(1, 8);
        break;
    case DurationType::V_HALF:    t = Fraction(1, 2);
        break;
    case DurationType::V_WHOLE:   t = Fraction(1, 1);
        break;
    case DurationType::V_BREVE:   t = Fraction(2, 1);
        break;
    case DurationType::V_LONG:    t = Fraction(4, 1);
        break;
    case DurationType::V_ZERO:
    case DurationType::V_MEASURE:
        return Fraction(0, 1);
    default:
    case DurationType::V_INVALID:
        return Fraction(-1, 1);
    }
    Fraction tmp = t;
    for (int i = 0; i < m_dots; ++i) {
        tmp *= Fraction(1, 2);
        t += tmp;
    }
    return t;
}

//---------------------------------------------------------
//   headType
//---------------------------------------------------------

NoteHeadType TDuration::headType() const
{
    NoteHeadType headType = NoteHeadType::HEAD_WHOLE;
    switch (m_val) {
    case DurationType::V_1024TH:
    case DurationType::V_512TH:
    case DurationType::V_256TH:
    case DurationType::V_128TH:
    case DurationType::V_64TH:
    case DurationType::V_32ND:
    case DurationType::V_16TH:
    case DurationType::V_EIGHTH:
    case DurationType::V_QUARTER:
        headType = NoteHeadType::HEAD_QUARTER;
        break;
    case DurationType::V_HALF:
        headType = NoteHeadType::HEAD_HALF;
        break;
    case DurationType::V_MEASURE:
    case DurationType::V_WHOLE:
        headType = NoteHeadType::HEAD_WHOLE;
        break;
    case DurationType::V_BREVE:
        headType = NoteHeadType::HEAD_BREVIS;
        break;
    case DurationType::V_LONG:
        headType = NoteHeadType::HEAD_BREVIS;
        break;
    default:
    case DurationType::V_INVALID:
    case DurationType::V_ZERO:
        headType = NoteHeadType::HEAD_QUARTER;
        break;
    }
    return headType;
}

//---------------------------------------------------------
//   hooks
//---------------------------------------------------------

int TDuration::hooks() const
{
    static constexpr int table[] = {
        // V_LONG, V_BREVE, V_WHOLE, V_HALF, V_QUARTER, V_EIGHTH, V_16TH,
        0,      0,       0,       0,      0,         1,        2,
        // V_32ND, V_64TH, V_128TH, V_256TH, V_512TH, V_1024TH,
        3,      4,       5,       6,      7,       8,
        // V_ZERO, V_MEASURE, V_INVALID
        0,      0,       0
    };
    return table[int(m_val)];
}

//---------------------------------------------------------
//   hasStem
//---------------------------------------------------------

bool TDuration::hasStem() const
{
    switch (m_val) {
    case DurationType::V_1024TH:
    case DurationType::V_512TH:
    case DurationType::V_256TH:
    case DurationType::V_128TH:
    case DurationType::V_64TH:
    case DurationType::V_32ND:
    case DurationType::V_16TH:
    case DurationType::V_EIGHTH:
    case DurationType::V_QUARTER:
    case DurationType::V_HALF:
    case DurationType::V_LONG:
        return true;
    default:
        return false;
    }
}

//---------------------------------------------------------
//   shiftType
//    If stepDotted = false, duration type will inc/dec by
//    nSteps with _dots remaining same.
//
//    If stepDotted = true, duration will round toward zero
//    to next single-dotted or undotted duration and then
//    will included dotted durations when stepping
//---------------------------------------------------------

void TDuration::shiftType(int nSteps, bool stepDotted)
{
    if (m_val == DurationType::V_MEASURE || m_val == DurationType::V_INVALID || m_val == DurationType::V_ZERO) {
        setType(DurationType::V_INVALID);
    } else {
        int newValue;
        int newDots;
        if (stepDotted) {
            // figure out the new duration in terms of the number of single dotted or undotted steps from DurationType::V_LONG
            int roundDownSingleDots = (m_dots > 0) ? -1 : 0;
            int newValAsNumSingleDotSteps = int(m_val) * 2 + roundDownSingleDots - nSteps;

            // convert that new duration back into terms of DurationType integer value and number of dots
            newDots  = newValAsNumSingleDotSteps % 2;       // odd means there is a dot
            newValue = newValAsNumSingleDotSteps / 2 + newDots;       // if new duration has a dot, then that
        } else {
            newDots = m_dots;
            newValue = int(m_val) - nSteps;
        }

        if ((newValue < int(DurationType::V_LONG)) || (newValue > int(DurationType::V_1024TH))
            || ((newValue >= int(DurationType::V_1024TH)) && (newDots >= 1))
            || ((newValue >= int(DurationType::V_512TH)) && (newDots >= 2))
            || ((newValue >= int(DurationType::V_256TH)) && (newDots >= 3))
            || ((newValue >= int(DurationType::V_128TH)) && (newDots >= 4))) {
            setType(DurationType::V_INVALID);
        } else {
            setType(DurationType(newValue));
            setDots(newDots);
        }
    }
}

//---------------------------------------------------------
//   operator<
//---------------------------------------------------------

bool TDuration::operator<(const TDuration& t) const
{
    if (t.m_val < m_val) {
        return true;
    }
    if (t.m_val == m_val) {
        if (m_dots < t.m_dots) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   operator>=
//---------------------------------------------------------

bool TDuration::operator>=(const TDuration& t) const
{
    if (t.m_val > m_val) {
        return true;
    }
    if (t.m_val == m_val) {
        if (m_dots >= t.m_dots) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   operator<=
//---------------------------------------------------------

bool TDuration::operator<=(const TDuration& t) const
{
    if (t.m_val < m_val) {
        return true;
    }
    if (t.m_val == m_val) {
        if (m_dots <= t.m_dots) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   operator>
//---------------------------------------------------------

bool TDuration::operator>(const TDuration& t) const
{
    if (t.m_val > m_val) {
        return true;
    }
    if (t.m_val == m_val) {
        if (m_dots > t.m_dots) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   fraction
//---------------------------------------------------------

Fraction TDuration::fraction() const
{
    int z = 1;
    unsigned n;
    switch (m_val) {
    case DurationType::V_1024TH:    n = 1024;
        break;
    case DurationType::V_512TH:     n = 512;
        break;
    case DurationType::V_256TH:     n = 256;
        break;
    case DurationType::V_128TH:     n = 128;
        break;
    case DurationType::V_64TH:      n = 64;
        break;
    case DurationType::V_32ND:      n = 32;
        break;
    case DurationType::V_16TH:      n = 16;
        break;
    case DurationType::V_EIGHTH:    n = 8;
        break;
    case DurationType::V_QUARTER:   n = 4;
        break;
    case DurationType::V_HALF:      n = 2;
        break;
    case DurationType::V_WHOLE:     n = 1;
        break;
    case DurationType::V_BREVE:     z = 2;
        n = 1;
        break;
    case DurationType::V_LONG:      z = 4;
        n = 1;
        break;
    case DurationType::V_ZERO:      z = 0;
        n = 1;
        break;
    default:          z = 0;
        n = 0;
        break;                                        // zero+invalid fraction
    }

    //dots multiplier is (2^(n + 1) - 1)/(2^n) where n is the number of dots
    int dotN = (1 << (static_cast<char>(m_dots) + 1)) - 1;
    int dotD = 1 << static_cast<char>(m_dots);

    return Fraction(z * dotN, n * dotD);
}

// Longest TDuration that fits into Fraction. Must fit exactly if truncate = false.
TDuration::TDuration(const Fraction& l, bool truncate, int maxDots, DurationType maxType)
{
#ifdef NDEBUG
    UNUSED(truncate);
#endif
    setType(maxType);   // use maxType to avoid testing all types if you know that l is smaller than a certain DurationType
    setDots(maxDots);
    truncateToFraction(l, maxDots);
    assert(truncate || (fraction() - l).numerator() == 0);   // check for exact fit
}

//---------------------------------------------------------
//   truncateToFraction
//---------------------------------------------------------

void TDuration::truncateToFraction(const Fraction& l, int maxDots)
{
    // try to fit in l by reducing number of duration dots
    if (setDotsToFitFraction(l, m_dots)) {
        return;
    }

    // that wasn't enough so now change type too
    for (shiftType(-1); isValid(); shiftType(-1)) {
        if (setDotsToFitFraction(l, maxDots)) {
            return;       // duration fits in l
        }
    }
}

//---------------------------------------------------------
//   setDotsToFitFraction
//---------------------------------------------------------

bool TDuration::setDotsToFitFraction(const Fraction& l, int maxDots)
{
    for (; maxDots >= 0; maxDots--) {
        m_dots = maxDots;     // ensures _dots >= 0 if function returns false.
        if ((fraction() - l).numerator() <= 0) {
            return true;       // duration fits in l
        }
    }

    return false;   // doesn't fit by changing dots alone (type needs to be changed too)
}

//---------------------------------------------------------
//   operator -=
//---------------------------------------------------------

TDuration& TDuration::operator-=(const TDuration& t)
{
    Fraction f1 = fraction() - t.fraction();
    TDuration d(f1);
    m_val  = d.m_val;
    m_dots = d.m_dots;
    return *this;
}

//---------------------------------------------------------
//   operator +=
//---------------------------------------------------------

TDuration& TDuration::operator+=(const TDuration& t)
{
    Fraction f1 = fraction() + t.fraction();
    TDuration d(f1);
    m_val  = d.m_val;
    m_dots = d.m_dots;
    return *this;
}

//---------------------------------------------------------
//   toDurationList
//---------------------------------------------------------

std::vector<TDuration> toDurationList(Fraction l, bool useDots, int maxDots, bool printRestRemains)
{
    std::vector<TDuration> dList;
    dList.reserve(8);

    if (!useDots) {
        maxDots = 0;
    }

    for (TDuration dd(l, true, maxDots); dd.isValid() && l.numerator() > 0; dd = TDuration(l, true, maxDots, dd.type())) {
        dList.push_back(dd);
        l -= dd.fraction();
    }

    if (printRestRemains && l.numerator() != 0) {
        LOGD("toDurationList:: rest remains %d/%d", l.numerator(), l.denominator());
    }

    return dList;
}

//---------------------------------------------------------
//   toRhythmicDurationList
//---------------------------------------------------------

std::vector<TDuration> toRhythmicDurationList(const Fraction& l, bool isRest, Fraction rtickStart,
                                              const TimeSigFrac& nominal, Measure* msr, int maxDots)
{
    std::vector<TDuration> dList;
    dList.reserve(8);

    if (msr->isAnacrusis()) {
        rtickStart += msr->anacrusisOffset();
    } else if (isRest && l == msr->ticks()) {
        TDuration d = TDuration(DurationType::V_MEASURE);
        dList.push_back(d);
        return dList;
    }

    if (nominal.isCompound()) {
        splitCompoundBeatsForList(&dList, l, isRest, rtickStart, nominal, maxDots);
    } else {
        populateRhythmicList(&dList, l, isRest, rtickStart, nominal, maxDots);
    }

    return dList;
}

//---------------------------------------------------------
//   populateRhythmicList
//---------------------------------------------------------

void populateRhythmicList(std::vector<TDuration>* dList, const Fraction& l, bool isRest, const Fraction& rtickStart,
                          const TimeSigFrac& nominal, int maxDots)
{
    Fraction rtickEnd = rtickStart + l;

    bool needToSplit = false;   // do we need to split?
    int rtickSplit = 0;   // tick to split on if we need to

    // CHECK AT SUBBEAT LEVEL

    int startLevel            = nominal.rtick2subbeatLevel(rtickStart.ticks());
    int endLevel              = nominal.rtick2subbeatLevel(rtickEnd.ticks());
    int strongestLevelCrossed = nominal.strongestSubbeatLevelInRange(rtickStart.ticks(), rtickEnd.ticks(), &rtickSplit);   // sets rtickSplit

    if ((startLevel < 0) || (endLevel < 0) || (strongestLevelCrossed < 0)) {
        // Beyond maximum subbeat level so just split into largest possible durations.
        std::vector<TDuration> dList2 = toDurationList(l, maxDots > 0, maxDots, false);
        dList->insert(dList->end(), dList2.begin(), dList2.end());
        return;
    }

    // split if we cross something stronger than where we start and end
    if ((strongestLevelCrossed < startLevel) && (strongestLevelCrossed < endLevel)) {
        needToSplit = true;
    }
    // but don't split for level 1 syncopation (allow eight-note, quarter, quarter... to cross unstressed beats)
    if (startLevel == endLevel && strongestLevelCrossed == startLevel - 1) {
        needToSplit = false;
    }
    // nor for the next simplest case of level 2 syncopation (allow sixteenth-note, eighth, eighth... to cross unstressed beats)
    if (startLevel == endLevel && strongestLevelCrossed == startLevel - 2) {
        // but disallow sixteenth-note, quarter, quarter...
        int ticksToNext = nominal.ticksToNextSubbeat(rtickStart.ticks(), startLevel - 1);
        int ticksPastPrev = nominal.ticksPastSubbeat(rtickStart.ticks(), startLevel - 1);
        needToSplit = ticksToNext != ticksPastPrev;
    }

    if (!needToSplit && strongestLevelCrossed == 0) {
        // NOW CHECK AT DENOMINATOR UNIT LEVEL AND BEAT LEVEL
        BeatType startBeat = nominal.rtick2beatType(rtickStart.ticks());
        BeatType endBeat   = nominal.rtick2beatType(rtickEnd.ticks());

        int dUnitsCrossed = 0;     // number of timeSig denominator units the note/rest crosses
        // if there is a choice of which beat to split on, should we use the first or last?
        bool useLast = startBeat <= BeatType::SIMPLE_UNSTRESSED;     // split on the later beat if starting on a beat

        BeatType strongestBeatCrossed = nominal.strongestBeatInRange(rtickStart.ticks(),
                                                                     rtickEnd.ticks(), &dUnitsCrossed, &rtickSplit, useLast);

        needToSplit = forceRhythmicSplit(isRest, startBeat, endBeat, dUnitsCrossed, strongestBeatCrossed, nominal);
    }

    if (!needToSplit) {
        // CHECK THERE IS A DURATION THAT FITS
        // crossed beats/subbeats were not important so try to avoid splitting
        TDuration d = TDuration(l, true, maxDots);
        if (d.fraction() == l) {
            // we can use a single duration - no need to split!
            dList->push_back(l);
            return;
        }
        // no single TDuration fits so must split anyway
    }

    // Prevent infinite recursion if there is no splitting point other than the start and end ticks
    IF_ASSERT_FAILED(rtickStart.ticks() < rtickSplit && rtickSplit < rtickEnd.ticks()) {
        std::vector<TDuration> dList2 = toDurationList(l, maxDots > 0, maxDots, false);
        dList->insert(dList->end(), dList2.begin(), dList2.end());
        return;
    }

    // Split on the strongest beat or subbeat crossed
    Fraction leftSplit = Fraction::fromTicks(rtickSplit) - rtickStart;
    Fraction rightSplit = l - leftSplit;

    // Recurse to see if we need to split further before adding to list
    populateRhythmicList(dList, leftSplit, isRest, rtickStart, nominal, maxDots);
    populateRhythmicList(dList, rightSplit, isRest, Fraction::fromTicks(rtickSplit), nominal, maxDots);
}

//---------------------------------------------------------
//   splitCompoundBeatsForList
//    Split compound notes/rests where they enter a compound beat.
//---------------------------------------------------------

void splitCompoundBeatsForList(std::vector<TDuration>* dList, const Fraction& l, bool isRest,
                               const Fraction& rtickStart, const TimeSigFrac& nominal, int maxDots)
{
    Fraction rtickEnd = rtickStart + l;

    BeatType startBeat = nominal.rtick2beatType(rtickStart.ticks());
    BeatType endBeat = nominal.rtick2beatType(rtickEnd.ticks());

    if (startBeat > BeatType::COMPOUND_UNSTRESSED) {
        // Not starting on a compound beat so mustn't extend into next compound beat
        int splitTicks = nominal.ticksToNextBeat(rtickStart.ticks());

        if ((rtickEnd - rtickStart).ticks() > splitTicks) {
            // Duration extends into next beat so must split
            Fraction leftSplit = Fraction::fromTicks(splitTicks);
            Fraction rightSplit = l - leftSplit;
            populateRhythmicList(dList, leftSplit, isRest, rtickStart, nominal, maxDots);       // this side is ok to proceed
            splitCompoundBeatsForList(dList, rightSplit, isRest, rtickStart + Fraction::fromTicks(splitTicks), nominal, maxDots);       // not checked yet
            return;
        }
    }

    if (endBeat > BeatType::COMPOUND_UNSTRESSED) {
        // Not ending on a compound beat so mustn't extend into previous compound beat
        int splitTicks = nominal.ticksPastBeat(rtickEnd.ticks());

        if ((rtickEnd - rtickStart).ticks() > splitTicks) {
            // Duration extends into previous beat so must split
            Fraction rightSplit = Fraction::fromTicks(splitTicks);
            Fraction leftSplit = l - rightSplit;
            populateRhythmicList(dList, leftSplit, isRest, rtickStart, nominal, maxDots);       // must add leftSplit to list first
            populateRhythmicList(dList, rightSplit, isRest, rtickEnd - Fraction::fromTicks(splitTicks), nominal, maxDots);
            return;
        }
    }

    // Duration either starts and ends on compound beats, or it remains within a single compound beat
    populateRhythmicList(dList, l, isRest, rtickStart, nominal, maxDots);
}

//---------------------------------------------------------
//   forceRhythmicSplit
//    Where to split notes and force a tie to indicate rhythm.
//
//    The function assumes the following:
//      * Note/rest has already been split at measure boundaries.
//      * Full measure rest was used if possible
//      * Note/rest already split where it enters a compound beat.
//
//    Usage: Set crossedBeat to the strongest BeatType crossed by
//    the note. Split note if function returns true. Repeat with
//    the two new notes, and so on, until function returns false.
//
//    Note: If no single TDuration can fill the gap then the note
//    *has* to be split, regardless of what this function returns.
//    Non-rhythmic splits should still occur on the strongest beat.
//
//    Implementation: When comparing BeatTypes use <, <=, >, >=
//    instead of == and != as appropriate (see sig.h). E.g. to match
//    all full beats: "<= SIMPLE_UNSTRESSED". This will match for
//    all compound and simple full beats, and not any subbeats.
//---------------------------------------------------------

bool forceRhythmicSplit(bool isRest, BeatType startBeat, BeatType endBeat,
                        int dUnitsCrossed, BeatType strongestBeatCrossed, const TimeSigFrac& nominal)
{
    // Assumption: Notes were split at measure boundary before this function was
    // called. (Necessary because timeSig might be different in next measure.)
    assert(strongestBeatCrossed != BeatType::DOWNBEAT);
    // Assumption: compound notes have already been split where they enter a compound beat.
    // (Necessary because the split beat is not always the strongest beat in this case.)
    assert(!nominal.isCompound() || strongestBeatCrossed >= BeatType::COMPOUND_SUBBEAT
           || (startBeat <= BeatType::COMPOUND_UNSTRESSED && endBeat <= BeatType::COMPOUND_UNSTRESSED));

    // SPECIAL CASES

    // nothing can cross a stressed beat in an irregular time signature
    if (strongestBeatCrossed <= BeatType::SIMPLE_STRESSED && !nominal.isTriple() && !nominal.isDuple()) {
        return true;
    }
    if (isRest) {
        // rests must not cross the middle of a bar with numerator == 2 (e.g. 2/4 bar) even though the beat is unstressed
        if (strongestBeatCrossed <= BeatType::SIMPLE_UNSTRESSED && nominal.numerator() == 2) {
            return true;
        }
        // rests must not cross a beat in a triple meter - simple (e.g. 3/4) or compound (e.g. 9/8)
        if (strongestBeatCrossed <= BeatType::SIMPLE_UNSTRESSED && nominal.isTriple()) {
            return true;
        }
    }

    // GENERAL RULES

    if (nominal.isCompound()) {
        return forceRhythmicSplitCompound(isRest, startBeat, endBeat, dUnitsCrossed, strongestBeatCrossed);
    } else {
        return forceRhythmicSplitSimple(isRest, startBeat, endBeat, dUnitsCrossed, strongestBeatCrossed);
    }
}

//---------------------------------------------------------
//   forceRhythmicSplitCompound
//---------------------------------------------------------

bool forceRhythmicSplitCompound(bool isRest, BeatType startBeat, BeatType endBeat, int dUnitsCrossed, BeatType strongestBeatCrossed)
{
    switch (strongestBeatCrossed) {
    case BeatType::COMPOUND_STRESSED:
        // Assumption: compound notes have already been split where they enter a compound beat.
        assert(startBeat <= BeatType::COMPOUND_UNSTRESSED && endBeat <= BeatType::COMPOUND_UNSTRESSED);
        // Notes are guaranteed to and start on a compound beat so we can pretend we have a simple measure.
        return forceRhythmicSplitSimple(isRest, startBeat, endBeat, dUnitsCrossed / 3, BeatType::SIMPLE_STRESSED);
    case BeatType::COMPOUND_UNSTRESSED:
        // Same assumption as before
        assert(startBeat <= BeatType::COMPOUND_UNSTRESSED && endBeat <= BeatType::COMPOUND_UNSTRESSED);
        // No further conditions since note are guaranteed to start and end on a compound beat.
        return false;
    case BeatType::COMPOUND_SUBBEAT:
        // don't split anything that takes up a full compound beat
        if (startBeat <= BeatType::COMPOUND_UNSTRESSED && endBeat <= BeatType::COMPOUND_UNSTRESSED) {
            return false;
        }
        // split rests that don't start on a compound beat
        if (isRest && startBeat > BeatType::COMPOUND_UNSTRESSED) {
            return true;
        }
        // Remaining groupings within compound triplets are the same as for simple triple (3/4, 3/8, etc.)
        return forceRhythmicSplitSimple(isRest, startBeat, endBeat, dUnitsCrossed, BeatType::SIMPLE_UNSTRESSED);
    default:         // BeatType::SUBBEAT
        return forceRhythmicSplitSimple(isRest, startBeat, endBeat, dUnitsCrossed, strongestBeatCrossed);
    }
}

//---------------------------------------------------------
//   forceRhythmicSplitSimple
//    Implementation: This function is also called for compound
//    measures so be careful when comparing BeatTypes. Use <, <=,
//    >, >= instead of == and != when appropriate. (See sig.h)
//---------------------------------------------------------

bool forceRhythmicSplitSimple(bool isRest, BeatType startBeat, BeatType endBeat, int beatsCrossed, BeatType strongestBeatCrossed)
{
    switch (strongestBeatCrossed) {
    case BeatType::SIMPLE_STRESSED:
        // Must split rests on a stressed beat.
        if (isRest) {
            return true;
        }
        // Don't split notes that start or end on a stressed beat. Enables double-dotting in 4/4.
        // (Don't remove this to disable double-dotting, instead set maxDots = 1 elsewhere.)
        if (startBeat <= BeatType::SIMPLE_STRESSED || endBeat <= BeatType::SIMPLE_STRESSED) {
            return false;
        }
        // Don't split notes that both start and end on unstressed beats or stronger.
        if (startBeat <= BeatType::SIMPLE_UNSTRESSED && endBeat <= BeatType::SIMPLE_UNSTRESSED) {
            return false;
        }
        // anything else must split on stressed beat.
        return true;
    case BeatType::SIMPLE_UNSTRESSED:
        // Don't split notes or rests if starting and ending on stressed beat.
        if (startBeat <= BeatType::SIMPLE_STRESSED && endBeat <= BeatType::SIMPLE_STRESSED) {
            return false;
        }
        // Split rests that don't start or end on a beat. Notes may cross 1 unstressed beat.
        if (startBeat == BeatType::SUBBEAT || endBeat == BeatType::SUBBEAT) {
            return isRest || (beatsCrossed > 1);
        }
        return false;
    default:         // BeatType::SUBBEAT
        return false;
    }
}

//---------------------------------------------------------
//   setType
//---------------------------------------------------------

void TDuration::setType(DurationType t)
{
    m_val = t;
    if (m_val == DurationType::V_MEASURE) {
        m_dots = 0;
    }
}

//---------------------------------------------------------
//   isValid
//---------------------------------------------------------

bool TDuration::isValid(Fraction f)
{
    TDuration t;
    t.setType(DurationType::V_LONG);
    t.setDots(4);
    t.truncateToFraction(f, 4);
    return t.isValid() && (t.fraction() - f).numerator() == 0;
}
}
