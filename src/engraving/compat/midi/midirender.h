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

#ifndef __RENDERMIDI_H__
#define __RENDERMIDI_H__

#include "libmscore/measure.h"
#include "libmscore/synthesizerstate.h"
#include "pitchwheelrenderer.h"

namespace mu::engraving {
class EventMap;
class MasterScore;
class Staff;
class SynthesizerState;

//---------------------------------------------------------
//   RangeMap
///   Helper class to keep track of status of status of
///   certain parts of score or MIDI representation.
//---------------------------------------------------------

class RangeMap
{
    enum class Range {
        BEGIN, END
    };
    std::map<int, Range> status;

public:
    void setOccupied(int tick1, int tick2);
    void setOccupied(std::pair<int, int> range) { setOccupied(range.first, range.second); }

    int occupiedRangeEnd(int tick) const;

    void clear() { status.clear(); }
};

//---------------------------------------------------------
//   MidiRenderer
///   MIDI renderer for a score
//---------------------------------------------------------

class MidiRenderer
{
public:
    struct Context
    {
        SynthesizerState synthState;
        bool metronome = true;

        Context() {}
    };

    explicit MidiRenderer(Score* s);

    void renderScore(EventMap* events, const Context& ctx);

    static const int ARTICULATION_CONV_FACTOR { 100000 };

private:

    Score* score = nullptr;

    void renderStaff(EventMap* events, const Staff* sctx, PitchWheelRenderer& pitchWheelRenderer);

    void renderSpanners(EventMap* events, PitchWheelRenderer& pitchWheelRenderer);

    void renderMetronome(EventMap* events);
    void renderMetronome(EventMap* events, Measure const* m);

    void collectMeasureEvents(EventMap* events, Measure const* m, const Staff* sctx, int tickOffset,
                              PitchWheelRenderer& pitchWheelRenderer);
    void doCollectMeasureEvents(EventMap* events, Measure const* m, const Staff* sctx, int tickOffset,
                                PitchWheelRenderer& pitchWheelRenderer);
};
} // namespace mu::engraving

#endif
