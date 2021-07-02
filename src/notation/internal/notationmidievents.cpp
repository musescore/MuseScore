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

#include "notationmidievents.h"

#include "log.h"

#include "notationerrors.h"

using namespace mu;
using namespace mu::notation;
using namespace mu::midi;

NotationMidiEvents::NotationMidiEvents(IGetScore* getScore, async::Notification notationChanged)
    : m_getScore(getScore)
{
    notationChanged.onNotify(this, [this]() {
        m_renderRanges.clear();
        m_eventsCache.clear();
    });
}

void NotationMidiEvents::init()
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    m_midiRenderImpl = std::unique_ptr<Ms::MidiRenderer>(new Ms::MidiRenderer(score()));
}

Events NotationMidiEvents::retrieveEvents(const channel_t midiChannel, const tick_t fromTick, const tick_t toTick) const
{
    for (const auto& gap : m_renderRanges.unrenderedGaps(fromTick, toTick)) {
        loadEvents(gap.second.start, gap.second.end);
    }

    return eventsFromRange(midiChannel, fromTick, toTick);
}

Events NotationMidiEvents::retrieveEventsForElement(const Element* element, const channel_t midiChannel) const
{
    if (element->isNote()) {
        return eventsFromNote(element, midiChannel);
    } else if (element->isChord()) {
        return eventsFromChord(element, midiChannel);
    } else if (element->isHarmony()) {
        return eventsFromHarmony(element, midiChannel);
    }

    NOT_SUPPORTED << element->name();
    return {};
}

std::vector<Event> NotationMidiEvents::retrieveSetupEvents(const std::list<InstrumentChannel*> instrChannels) const
{
    std::vector<midi::Event> result;

    for (const InstrumentChannel* instrChannel : instrChannels) {
        Event e;

        e.setMessageType(Event::MessageType::ChannelVoice10);
        e.setOpcode(Event::Opcode::ProgramChange);
        e.setProgram(instrChannel->program());
        e.setBank(instrChannel->bank());
        e.setChannel(instrChannel->channel());

        result.emplace_back(std::move(e));
    }

    return result;
}

Events NotationMidiEvents::eventsFromRange(const channel_t midiChannel, const tick_t fromTick, const tick_t toTick) const
{
    auto search = m_eventsCache.find(midiChannel);

    if (search == m_eventsCache.end() || fromTick >= toTick) {
        return {};
    }

    Events result;

    for (const auto& pair : search->second) {
        if (pair.first < fromTick || pair.first > toTick) {
            continue;
        }

        result.insert(pair);
    }

    return result;
}

Ms::Score* NotationMidiEvents::score() const
{
    return m_getScore->score();
}

Ms::MasterScore* NotationMidiEvents::masterScore() const
{
    return score() ? score()->masterScore() : nullptr;
}

void NotationMidiEvents::loadEvents(const tick_t fromTick, const tick_t toTick) const
{
    Ms::EventMap msevents = renderMsEvents(fromTick, toTick);

    if (msevents.empty()) {
        return;
    }

    TicksRange renderedTickRange;
    renderedTickRange.start = fromTick;
    renderedTickRange.end = toTick;

    m_renderRanges.insertOrReplace(renderedTickRange.start, std::move(renderedTickRange));

    for (auto& pair : convertMsEvents(std::move(msevents))) {
        for (Event& event : pair.second) {
            Events& chunk = m_eventsCache[event.channel()];
            std::vector<Event>& events = chunk[pair.first];
            events.push_back(std::move(event));
        }
    }
}

bool NotationMidiEvents::hasCachedEvents(const channel_t midiChannel) const
{
    return m_eventsCache.find(midiChannel) != m_eventsCache.end();
}

Ms::EventMap NotationMidiEvents::renderMsEvents(const tick_t fromTick, const tick_t toTick) const
{
    Ms::EventMap msevents;

    std::vector<Ms::MidiRenderer::Chunk> mschunks = m_midiRenderImpl->chunksFromRange(fromTick, toTick);
    if (mschunks.empty()) {
        return msevents;
    }

    masterScore()->setExpandRepeats(configuration()->isPlayRepeatsEnabled());

    Ms::MidiRenderer::Context ctx;
    ctx.metronome = configuration()->isMetronomeEnabled();
    ctx.renderHarmony = true;

    for (const auto& mschunk : mschunks) {
        m_midiRenderImpl->renderChunk(mschunk, &msevents, ctx);
    }

    return msevents;
}

Events NotationMidiEvents::convertMsEvents(Ms::EventMap&& eventMap) const
{
    Events result;

    std::set<tick_t> tickKeySet;
    for (const auto& pair : eventMap) {
        tickKeySet.insert(pair.first);
    }

    for (const tick_t tick : tickKeySet) {
        std::vector<midi::Event> events;

        auto it = eventMap.equal_range(tick);

        for (auto itr = it.first; itr != it.second; ++itr) {
            const Ms::NPlayEvent ev = itr->second;

            midi::EventType etype = static_cast<midi::EventType>(ev.type());
            static const std::set<EventType> SKIP_EVENTS
                = { EventType::ME_INVALID, EventType::ME_EOT, EventType::ME_TICK1, EventType::ME_TICK2 };
            if (SKIP_EVENTS.find(etype) != SKIP_EVENTS.end()) {
                continue;
            }
            midi::Event e
            {
                static_cast<channel_t>(ev.channel()),
                etype,
                static_cast<uint8_t>(ev.dataA()),
                static_cast<uint8_t>(ev.dataB())
            };

            events.push_back(std::move(e));
        }

        result.insert({ tick, std::move(events) });
    }

    return result;
}

//! NOTE Copied from MuseScore::play(Element* e, int pitch)
Events NotationMidiEvents::eventsFromNote(const Element* noteElement, const channel_t midiChannel) const
{
    const Ms::Note* note = Ms::toNote(noteElement);
    IF_ASSERT_FAILED(note) {
        return {};
    }

    Events result;

    auto event = midi::Event(midi::Event::Opcode::NoteOn);
    event.setChannel(midiChannel);
    event.setNote(note->ppitch());
    event.setVelocityFraction(0.63f); //as 80 for 127 scale
    result.insert({ 0, { event } });

    event.setOpcode(midi::Event::Opcode::NoteOff);
    event.setVelocity(0);
    result.insert({ static_cast<tick_t>(Ms::MScore::defaultPlayDuration), { std::move(event) } });

    return result;
}

//! NOTE Copied from MuseScore::play(Element* e, int pitch)
Events NotationMidiEvents::eventsFromChord(const Element* chordElement, const channel_t midiChannel) const
{
    const Ms::Chord* chord = Ms::toChord(chordElement);
    IF_ASSERT_FAILED(chord) {
        return {};
    }

    Events result;

    std::vector<midi::Event> noteOnEvents(chord->notes().size());
    std::vector<midi::Event> noteOffEvents(chord->notes().size());

    for (Ms::Note* n : chord->notes()) {
        int pitch = n->ppitch();

        auto event = midi::Event(midi::Event::Opcode::NoteOn);
        event.setChannel(midiChannel);
        event.setNote(pitch);
        event.setVelocityFraction(0.63f); //as 80 for 127 scale
        noteOnEvents.push_back(event);

        event.setOpcode(midi::Event::Opcode::NoteOff);
        event.setVelocity(0);
        noteOffEvents.push_back(event);
    }

    result.insert({ 0, std::move(noteOnEvents) });
    result.insert({ static_cast<tick_t>(Ms::MScore::defaultPlayDuration), std::move(noteOffEvents) });

    return result;
}

//! NOTE Copied from MuseScore::play(Element* e, int pitch)
Events NotationMidiEvents::eventsFromHarmony(const Element* harmonyElement, const channel_t midiChannel) const
{
    const Harmony* harmony = Ms::toHarmony(harmonyElement);
    IF_ASSERT_FAILED(harmony) {
        return {};
    }

    Events result;

    const Ms::RealizedHarmony& r = harmony->getRealizedHarmony();
    QList<int> pitches = r.pitches();

    auto noteOn = midi::Event(midi::Event::Opcode::NoteOn);
    noteOn.setChannel(midiChannel);
    noteOn.setVelocityFraction(0.63f); //as 80 for 127 scale

    auto noteOff = midi::Event(midi::Event::Opcode::NoteOff);
    noteOff.setChannel(midiChannel);

    std::vector<midi::Event> noteOnEvents(pitches.size());
    std::vector<midi::Event> noteOffEvents(pitches.size());

    for (int pitch : pitches) {
        noteOn.setNote(pitch);
        noteOnEvents.push_back(noteOn);

        noteOff.setNote(pitch);
        noteOffEvents.push_back(noteOff);
    }

    result.insert({ 0, std::move(noteOnEvents) });
    result.insert({ static_cast<tick_t>(Ms::MScore::defaultPlayDuration), std::move(noteOffEvents) });

    return result;
}

NotationMidiEvents::TickRangeMap NotationMidiEvents::RenderRangeMap::unrenderedGaps(const tick_t from, const tick_t to) const
{
    TickRangeMap result;

    TicksRange searchRange = { from, to };

    std::vector<TicksRange> unrenderedGaps;

    if (m_renderedRanges.empty()) {
        result.emplace(from, std::move(searchRange));
        return result;
    }

    for (const auto& range : m_renderedRanges) {
        if (range.second.contains(searchRange)) {
            return {};
        } else if (range.second.containsTick(searchRange.start)) {
            searchRange.start = range.second.end;
        } else if (range.second.containsTick(searchRange.end)) {
            searchRange.end = range.second.start;
        }
    }

    result.emplace(from, std::move(searchRange));
    return result;
}

void NotationMidiEvents::RenderRangeMap::insertOrReplace(const tick_t key, NotationMidiEvents::TicksRange&& newRange)
{
    auto it = m_renderedRanges.begin();

    while (it != m_renderedRanges.end()) {
        if (newRange.contains(it->second)) {
            it = m_renderedRanges.erase(it);
        } else {
            ++it;
        }
    }

    m_renderedRanges.emplace(key, newRange);
}

void NotationMidiEvents::RenderRangeMap::clear()
{
    m_renderedRanges.clear();
}
