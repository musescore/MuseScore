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

#include <memory>

#include "libmscore/instrument.h"
#include "libmscore/measure.h"
#include "libmscore/synthesizerstate.h"
#include "pitchwheelrenderer.h"

namespace mu::engraving {
class EventMap;
class MasterScore;
class Staff;
class SynthesizerState;

//---------------------------------------------------------
//   MidiRenderer
///   MIDI renderer for a score
//---------------------------------------------------------

class MidiRenderer
{
public:
    //! @brief helper structure to find channel in a case eachStringHasChannel = true
    struct ChannelLookup {
        std::map<int, std::map<int, int> > channelsMap;
        uint32_t maxChannel = 0;
        uint32_t getChannel(uint32_t instrumentChannel, int32_t string);
    };

    struct Context
    {
        SynthesizerState synthState;
        bool metronome = true;
        bool eachStringHasChannel = false;//!to better display the guitar instrument, each string has its own channel
        std::shared_ptr<ChannelLookup> channels = std::make_shared<ChannelLookup>();

        Context() {}
    };

    explicit MidiRenderer(Score* s);

    void renderScore(EventMap* events, const Context& ctx);

    static const int ARTICULATION_CONV_FACTOR { 100000 };

private:

    void renderStaff(EventMap* events, const Staff* sctx, PitchWheelRenderer& pitchWheelRenderer);

    void renderSpanners(EventMap* events, PitchWheelRenderer& pitchWheelRenderer);
    void doRenderSpanners(EventMap* events, Spanner* s, uint32_t channel, PitchWheelRenderer& pitchWheelRenderer);

    void renderMetronome(EventMap* events);
    void renderMetronome(EventMap* events, Measure const* m);

    void collectMeasureEvents(EventMap* events, Measure const* m, const Staff* sctx, int tickOffset,
                              PitchWheelRenderer& pitchWheelRenderer);
    void doCollectMeasureEvents(EventMap* events, Measure const* m, const Staff* sctx, int tickOffset,
                                PitchWheelRenderer& pitchWheelRenderer);
    void collectGraceBeforeChordEvents(Chord* chord, EventMap* events, double veloMultiplier, Staff* st, int tickOffset,
                                       PitchWheelRenderer& pitchWheelRenderer);

    uint32_t getChannel(const Instrument* instr, const Note* note);

    Score* score = nullptr;

    Context _context;
};
} // namespace mu::engraving

#endif
