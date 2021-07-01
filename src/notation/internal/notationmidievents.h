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

#ifndef MU_NOTATION_NOTATIONMIDEVENTS_H
#define MU_NOTATION_NOTATIONMIDEVENTS_H

#include <map>
#include <unordered_map>

#include "async/asyncable.h"
#include "async/notification.h"
#include "libmscore/rendermidi.h"

#include "igetscore.h"
#include "inotationmidievents.h"
#include "inotationconfiguration.h"

namespace mu::notation {
class NotationMidiEvents : public INotationMidiEvents, public async::Asyncable
{
    INJECT(notation, INotationConfiguration, configuration)

public:
    explicit NotationMidiEvents(IGetScore* getScore, async::Notification notationChanged);

    void init() override;
    midi::Events retrieveEvents(const midi::channel_t midiChannel, const midi::tick_t fromTick, const midi::tick_t toTick) const override;
    midi::Events retrieveEventsForElement(const Element* element, const midi::channel_t midiChannel) const override;
    std::vector<midi::Event> retrieveSetupEvents(const std::list<InstrumentChannel*> instrChannel) const override;

private:
    struct TicksRange {
        midi::tick_t start = 0;
        midi::tick_t end = 0;

        bool operator <(const TicksRange& other) const
        {
            return start < other.start;
        }

        bool contains(const TicksRange& other) const
        {
            return other.start >= start && other.end <= end;
        }

        bool containsTick(const midi::tick_t tick) const
        {
            return tick >= start && tick <= end;
        }
    };

    using TickRangeMap = std::map<midi::tick_t, TicksRange>;

    struct RenderRangeMap {
        TickRangeMap unrenderedGaps(const midi::tick_t from, const midi::tick_t to) const;

        void insertOrReplace(const midi::tick_t key, TicksRange&& newRange);
        void clear();

    private:
        mutable TickRangeMap m_renderedRanges;
    };

    Ms::Score* score() const;
    Ms::MasterScore* masterScore() const;

    void loadEvents(const midi::tick_t fromTick, const midi::tick_t toTick) const;
    midi::Events eventsFromRange(const midi::channel_t midiChannel, const midi::tick_t fromTick, const midi::tick_t toTick) const;
    bool hasCachedEvents(const midi::channel_t midiChannel) const;

    Ms::EventMap renderMsEvents(const midi::tick_t fromTick, const midi::tick_t toTick) const;
    midi::Events convertMsEvents(Ms::EventMap&& eventMap) const;

    midi::Events eventsFromNote(const Element* noteElement, const midi::channel_t midiChannel) const;
    midi::Events eventsFromChord(const Element* chordElement, const midi::channel_t midiChannel) const;
    midi::Events eventsFromHarmony(const Element* harmonyElement, const midi::channel_t midiChannel) const;

    mutable RenderRangeMap m_renderRanges;
    mutable std::unordered_map<midi::channel_t, midi::Events> m_eventsCache = {};

    std::unique_ptr<Ms::MidiRenderer> m_midiRenderImpl = nullptr;
    IGetScore* m_getScore = nullptr;
};
}

#endif // MU_NOTATION_NOTATIONMIDEVENTS_H
