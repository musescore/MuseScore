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

#ifndef __DURATIONTYPE_H__
#define __DURATIONTYPE_H__

#include "note.h"

namespace Ms {
class TimeSigFrac;
enum class BeatType : char;

//---------------------------------------------------------
//   TDuration
//---------------------------------------------------------

class TDuration
{
private:
    DurationType _val = DurationType::V_INVALID;
    int _dots = 0;
    void shiftType(int nSteps, bool stepDotted = false);
    void truncateToFraction(const Fraction& l, int maxDots);
    bool setDotsToFitFraction(const Fraction& l, int maxDots);

public:
    TDuration()
        : _val(DurationType::V_INVALID), _dots(0) {}
    TDuration(const Fraction& l, bool truncate = false, int maxDots = 4, DurationType maxType = DurationType::V_LONG);
    TDuration(DurationType t)
        : _val(t), _dots(0) {}
    TDuration(DurationTypeWithDots t)
        : _val(t.type), _dots(t.dots) {}

    DurationType type() const { return _val; }
    DurationTypeWithDots typeWithDots() const { return DurationTypeWithDots(_val, _dots); }
    bool isValid() const { return _val != DurationType::V_INVALID; }
    bool isZero() const { return _val == DurationType::V_ZERO; }
    bool isMeasure() const { return _val == DurationType::V_MEASURE; }
    void setVal(int tick);
    void setType(DurationType t);

    Fraction ticks() const;
    bool operator==(const TDuration& t) const { return t._val == _val && t._dots == _dots; }
    bool operator==(const DurationType& t) const { return t == _val; }
    bool operator!=(const TDuration& t) const { return t._val != _val || t._dots != _dots; }
    bool operator<(const TDuration& t) const;
    bool operator>(const TDuration& t) const;
    bool operator>=(const TDuration& t) const;
    bool operator<=(const TDuration& t) const;
    TDuration& operator-=(const TDuration& t);
    TDuration operator-(const TDuration& t) const { return TDuration(*this) -= t; }
    TDuration& operator+=(const TDuration& t);
    TDuration operator+(const TDuration& t) const { return TDuration(*this) += t; }

    NoteHeadType headType() const;
    int hooks() const;
    bool hasStem() const;
    TDuration shift(int nSteps) const { TDuration d(type()); d.shiftType(nSteps); return d; }                                // dots are not retained
    TDuration shiftRetainDots(int nSteps, bool stepDotted = false)
    {
        TDuration d(type());
        d.setDots(_dots);
        d.shiftType(nSteps, stepDotted);
        return d;
    }

    int dots() const { return _dots; }
    void setDots(int v);
    Fraction fraction() const;
    static bool isValid(Fraction f);
};

std::vector<TDuration> toDurationList(Fraction l, bool useDots, int maxDots = 4, bool printRestRemains = true);
std::vector<TDuration> toRhythmicDurationList(const Fraction& l, bool isRest, Fraction rtickStart, const TimeSigFrac& nominal, Measure* msr,
                                              int maxDots);

bool forceRhythmicSplit(bool isRest, BeatType startBeat, BeatType endBeat, int beatsCrossed, BeatType strongestBeatCrossed,
                        const TimeSigFrac& nominal);
bool forceRhythmicSplitSimple(bool isRest, BeatType startBeat, BeatType endBeat, int beatsCrossed, BeatType strongestBeatCrossed);
bool forceRhythmicSplitCompound(bool isRest, BeatType startBeat, BeatType endBeat, int beatsCrossed, BeatType strongestBeatCrossed);

void populateRhythmicList(std::vector<TDuration>* dList, const Fraction& l, bool isRest, const Fraction& rtickStart,
                          const TimeSigFrac& nominal, int maxDots);
void splitCompoundBeatsForList(std::vector<TDuration>* dList, const Fraction& l, bool isRest, const Fraction& rtickStart,
                               const TimeSigFrac& nominal, int maxDots);
}     // namespace Ms

Q_DECLARE_METATYPE(Ms::TDuration);

#endif
