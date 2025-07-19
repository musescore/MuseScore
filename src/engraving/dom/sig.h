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

#pragma once

#include <map>
#include <cassert>

#include "global/allocator.h"
#include "../types/types.h"

namespace mu::engraving {
int ticks_beat(int n);

//-------------------------------------------------------------------
//   BeatType
//-------------------------------------------------------------------

enum class BeatType : unsigned char {
    DOWNBEAT,                 // 1st beat of measure (rtick == 0)
    COMPOUND_STRESSED,        // e.g. eighth-note number 7 in 12/8
    SIMPLE_STRESSED,          // e.g. beat 3 in 4/4
    COMPOUND_UNSTRESSED,      // e.g. eighth-note numbers 4 or 10 in 12/8
    SIMPLE_UNSTRESSED,        // "offbeat" e.g. beat 2 and 4 in 4/4 (i.e. the denominator unit)
    COMPOUND_SUBBEAT,         // e.g. any other eighth-note in 12/8 (i.e. the denominator unit)
    SUBBEAT                   // does not fall on a beat
};

//-------------------------------------------------------------------
//   Time Signature Fraction   (n/d - numerator/denominator)
//-------------------------------------------------------------------

class TimeSigFrac : public Fraction
{
    OBJECT_ALLOCATOR(engraving, TimeSigFrac)
public:
    using Fraction::Fraction;
    constexpr TimeSigFrac(int n = 0, int d = 1)
        : Fraction(n, d) {}
    TimeSigFrac(const Fraction& f)
        : TimeSigFrac(f.numerator(), f.denominator()) {}

    // isCompound? Note: 3/8, 3/16, ... are NOT considered compound.
    bool isCompound() const { return numerator() > 3 /*&& denominator() >= 8*/ && numerator() % 3 == 0; }

    // isBeatedCompound? Note: Conductors will beat the simple unit at slow tempos (<60 compound units per minute)
    // However, the meter is still considered to be compound (at least for our purposes).
    bool isBeatedCompound(double tempo) const { return tempo2beatsPerMinute(tempo) >= 60.0; }

    int dUnitTicks()        const;
    int ticksPerMeasure()   const { return numerator() * dUnitTicks(); }

    int dUnitsPerBeat()     const { return isCompound() ? 3 : 1; }
    int beatTicks()         const { return dUnitTicks() * dUnitsPerBeat(); }
    int beatsPerMeasure()   const { return numerator() / dUnitsPerBeat(); }

    int subbeatTicks(int level)   const;
    int maxSubbeatLevel()         const;

    bool isTriple()         const { return beatsPerMeasure() % 3 == 0; }
    bool isDuple() const { assert(!isTriple()); return beatsPerMeasure() % 2 == 0; }   // note: always test isTriple() first

    // MuseScore stores tempos in quarter-notes-per-second, so conversions to conventional beats-per-minute format are provided here:
    double tempo2beatsPerMinute(double tempo)   const { return tempo * denominator() * 15.0 / dUnitsPerBeat(); }
    double beatsPerMinute2tempo(double bpm)     const { return bpm * dUnitsPerBeat() / (15.0 * denominator()); }

    BeatType rtick2beatType(int rtick)  const;
    int rtick2subbeatLevel(int rtick)   const;   // returns negative value if not on a well-defined subbeat

    BeatType strongestBeatInRange(int rtick1, int rtick2, int* dUnitsCrossed = 0, int* subbeatTick = 0, bool saveLast = false) const;   // range is exclusive
    int strongestSubbeatLevelInRange(int rtick1, int rtick2, int* subbeatTick = 0) const;   // range is exclusive

    int ticksPastDUnit(int rtick)       const { return rtick % dUnitTicks(); }                   // returns 0 if rtick is exactly on a dUnit
    int ticksToNextDUnit(int rtick)     const { return dUnitTicks() - ticksPastDUnit(rtick); }   // returns dUnitTicks() if rtick is on a dUnit

    int ticksPastBeat(int rtick)        const { return rtick % beatTicks(); }                    // returns 0 if rtick is exactly on a beat
    int ticksToNextBeat(int rtick)      const { return beatTicks() - ticksPastBeat(rtick); }     // returns beatTicks() if rtick is on a beat

    int ticksPastSubbeat(int rtick, int level)      const { return rtick % subbeatTicks(level); }
    int ticksToNextSubbeat(int rtick, int level)    const { return subbeatTicks(level) - ticksPastSubbeat(rtick, level); }
};

//-------------------------------------------------------------------
//   Time Signature Event
//    Incomplete measures as for example pickup measures have
//    a nominal duration different from actual duration.
//-------------------------------------------------------------------

class SigEvent
{
public:
    constexpr SigEvent()
        : m_bar(0) {}                          ///< default SigEvent is invalid
    SigEvent(const Fraction& s, int bar = 0)
        : m_timesig(s), m_nominal(s), m_bar(bar) {}
    SigEvent(const Fraction& s, const Fraction& ss, int bar = 0)
        : m_timesig(s), m_nominal(ss), m_bar(bar) {}

    bool operator==(const SigEvent& e) const;
    bool valid() const { return m_timesig.isValid(); }
    String print() const { return m_timesig.toString(); }
    TimeSigFrac timesig() const { return m_timesig; }
    void setTimesig(const TimeSigFrac& f) { m_timesig = f; }
    TimeSigFrac nominal() const { return m_nominal; }
    void setNominal(const TimeSigFrac& f) { m_nominal = f; }
    int bar() const { return m_bar; }
    void setBar(int val) { m_bar = val; }

private:

    TimeSigFrac m_timesig;
    TimeSigFrac m_nominal;
    int m_bar = 0;                 ///< precomputed value
};

//---------------------------------------------------------
//   SigList
//---------------------------------------------------------

class TimeSigMap : public std::map<int, SigEvent>
{
    OBJECT_ALLOCATOR(engraving, TimeSigMap)

public:
    TimeSigMap() {}

    static const TimeSigFrac DEFAULT_TIME_SIGNATURE;

    void add(int tick, const Fraction&);
    void add(int tick, const SigEvent& ev);

    void del(int tick);

    void clearRange(int tick1, int tick2);

    void dump() const;

    const SigEvent& timesig(int tick) const;
    const SigEvent& timesig(const Fraction& f) const { return timesig(f.ticks()); }

    void tickValues(int t, int* bar, int* beat, int* tick) const;
    int bar2tick(int bar, int beat) const;
    String pos(int t) const;

    unsigned raster(unsigned tick, int raster) const;
    unsigned raster1(unsigned tick, int raster) const;      // round down
    unsigned raster2(unsigned tick, int raster) const;      // round up
    int rasterStep(unsigned tick, int raster) const;

    void normalize();
};
}
