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

#include "masternotationmididata.h"

#include "engraving/libmscore/repeatlist.h"
#include "engraving/libmscore/tempo.h"

#include "log.h"

#include "notationerrors.h"

using namespace mu;
using namespace mu::notation;
using namespace mu::midi;

MasterNotationMidiData::MasterNotationMidiData(IGetScore* getScore, async::Notification notationChanged)
    : m_getScore(getScore)
{
    notationChanged.onNotify(this, [this]() {
        m_renderRanges.clear();
        m_eventsCache.clear();
    });
}

MasterNotationMidiData::~MasterNotationMidiData()
{
    m_parts = nullptr;
}

void MasterNotationMidiData::init(INotationPartsPtr parts)
{
    IF_ASSERT_FAILED(score() && parts) {
        return;
    }

    m_parts = std::move(parts);
    m_midiRenderImpl = std::unique_ptr<Ms::MidiRenderer>(new Ms::MidiRenderer(score()));

    m_midiDataMap.clear();

    for (const Part* part : m_parts->partList()) {
        m_midiDataMap.insert({ part->id(), buildMidiData(part) });
    }

    m_parts->partList().onItemAdded(this, [this](const Part* part) {
        m_midiDataMap.insert({ part->id(), buildMidiData(part) });
    });

    m_parts->partList().onItemRemoved(this, [this](const Part* part) {
        m_midiDataMap.erase(part->id());
    });
}

MidiData MasterNotationMidiData::trackMidiData(const ID& partId) const
{
    auto search = m_midiDataMap.find(partId);

    if (search != m_midiDataMap.end()) {
        return search->second;
    }

    return MidiData();
}

Ret MasterNotationMidiData::triggerElementMidiData(const Element* element)
{
    if (element->isNote()) {
        const Ms::Note* note = Ms::toNote(element);
        IF_ASSERT_FAILED(note) {
            return make_ret(Err::UnableToPlaybackElement);
        }
        return playNoteMidiData(note);
    } else if (element->isChord()) {
        const Ms::Chord* chord = Ms::toChord(element);
        IF_ASSERT_FAILED(chord) {
            return make_ret(Err::UnableToPlaybackElement);
        }
        return playChordMidiData(chord);
    } else if (element->isHarmony()) {
        const Ms::Harmony* h = Ms::toHarmony(element);
        IF_ASSERT_FAILED(h) {
            return make_ret(Err::UnableToPlaybackElement);
        }
        return playHarmonyMidiData(h);
    }

    NOT_SUPPORTED << element->name();
    return make_ret(Err::UnableToPlaybackElement);
}

Events MasterNotationMidiData::retrieveEvents(const channel_t midiChannel, const tick_t fromTick, const tick_t toTick) const
{
    for (const auto& gap : m_renderRanges.unrenderedGaps(fromTick, toTick)) {
        loadEvents(gap.second.start, gap.second.end);
    }

    return eventsFromRange(midiChannel, fromTick, toTick);
}

Events MasterNotationMidiData::retrieveEventsForElement(const Element* element, const channel_t midiChannel) const
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

std::vector<Event> MasterNotationMidiData::retrieveSetupEvents(const std::list<InstrumentChannel*> instrChannels) const
{
    std::vector<midi::Event> result;

    for (const InstrumentChannel* instrChannel : instrChannels) {
        Event e;

        e.setMessageType(Event::MessageType::ChannelVoice10);
        e.setOpcode(Event::Opcode::ProgramChange);
        e.setProgram(instrChannel->program());
        e.setBank(instrChannel->bank());

        channel_t channel = instrChannel->channel();
        //! TODO Modification of templates is required
        if (!(channel < 16)) {
            channel = 15;
        }

        e.setChannel(channel);

        result.emplace_back(std::move(e));
    }

    return result;
}

Events MasterNotationMidiData::eventsFromRange(const channel_t midiChannel, const tick_t fromTick, const tick_t toTick) const
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

Ms::Score* MasterNotationMidiData::score() const
{
    return m_getScore->score();
}

Ms::MasterScore* MasterNotationMidiData::masterScore() const
{
    return score() ? score()->masterScore() : nullptr;
}

MidiData MasterNotationMidiData::buildMidiData(const Ms::Part* part) const
{
    return { buildMidiMapping(part), buildMidiStream(part) };
}

MidiMapping MasterNotationMidiData::buildMidiMapping(const Ms::Part* part) const
{
    midi::MidiMapping mapping;

    mapping.division = Ms::MScore::division;
    mapping.tempo = makeTempoMap();

    for (auto it = part->instruments()->cbegin(); it != part->instruments()->cend(); ++it) {
        const Ms::Instrument* instrument = it->second;

        for (const Ms::Channel* channel : instrument->channel()) {
            mapping.programms.push_back({ static_cast<midi::channel_t>(channel->channel()),
                                          channel->program(),
                                          channel->bank() });
        }
    }

    return mapping;
}

MidiStream MasterNotationMidiData::buildMidiStream(const Ms::Part* part) const
{
    midi::MidiStream stream;

    IF_ASSERT_FAILED(masterScore()->lastMeasure()) {
        return stream;
    }

    stream.lastTick = masterScore()->lastMeasure()->endTick().ticks() - 1;

    for (auto it = part->instruments()->cbegin(); it != part->instruments()->cend(); ++it) {
        const Ms::Instrument* instrument = it->second;

        std::vector<channel_t> midiChannels;
        midiChannels.reserve(instrument->channel().size());

        for (const Ms::Channel* channel : instrument->channel()) {
            channel_t midiChannel = channel->channel();
            midiChannels.push_back(midiChannel);
        }

        std::list<InstrumentChannel*> channelList(instrument->channel().begin(), instrument->channel().end());

        std::vector<Event> setupEvents = retrieveSetupEvents(channelList);
        stream.controlEventsStream.set(std::move(setupEvents));

        stream.eventsRequest.onReceive(this, [this, stream, midiChannels](const tick_t fromTick, const tick_t toTick) mutable {
            if (fromTick >= stream.lastTick || toTick > stream.lastTick) {
                stream.mainStream.send({}, 0);
                return;
            }

            for (const channel_t& midiChannel : midiChannels) {
                Events events = retrieveEvents(static_cast<channel_t>(midiChannel), fromTick, toTick);

                stream.mainStream.send(std::move(events), toTick);
            }
        });
    }

    return stream;
}

TempoMap MasterNotationMidiData::makeTempoMap() const
{
    midi::TempoMap tempos;

    Ms::TempoMap* tempomap = score()->tempomap();
    qreal relTempo = tempomap->relTempo();
    for (const Ms::RepeatSegment* rs : score()->repeatList()) {
        int startTick = rs->tick, endTick = startTick + rs->len();
        int tickOffset = rs->utick - rs->tick;

        auto se = tempomap->lower_bound(startTick);
        auto ee = tempomap->lower_bound(endTick);
        for (auto it = se; it != ee; ++it) {
            //
            // compute midi tempo: microseconds / quarter note
            //
            tempo_t tempo = static_cast<tempo_t>(lrint((1.0 / (it->second.tempo * relTempo)) * 1000000.0));

            tempos.insert({ it->first + tickOffset, tempo });
        }
    }

    return tempos;
}

Ret MasterNotationMidiData::playNoteMidiData(const Ms::Note* note) const
{
    const Ms::Note* masterNote = note;
    if (note->linkList().size() > 1) {
        for (Ms::ScoreElement* se : note->linkList()) {
            if (se->score() == note->masterScore() && se->isNote()) {
                masterNote = Ms::toNote(se);
                break;
            }
        }
    }

    Ms::Fraction tick = masterNote->chord()->tick();
    if (tick < Ms::Fraction(0, 1)) {
        tick = Ms::Fraction(0, 1);
    }
    const Ms::Instrument* instr = masterNote->part()->instrument(tick);
    channel_t midiChannel = instr->channel(masterNote->subchannel())->channel();

    MidiData midiData = trackMidiData(masterNote->part()->id());

    Events events = retrieveEventsForElement(masterNote, midiChannel);
    midiData.stream.backgroundStream.send(std::move(events), Ms::MScore::defaultPlayDuration* 2);

    return make_ret(Ret::Code::Ok);
}

Ret MasterNotationMidiData::playChordMidiData(const Ms::Chord* chord) const
{
    IF_ASSERT_FAILED(!chord->notes().empty()) {
        return make_ret(Err::UnableToPlaybackElement);
    }

    Ms::Part* part = chord->part();
    Ms::Fraction tick = chord->segment() ? chord->segment()->tick() : Ms::Fraction(0, 1);
    Ms::Instrument* instr = part->instrument(tick);
    channel_t midiChannel = instr->channel(chord->notes()[0]->subchannel())->channel();

    MidiData midiData = trackMidiData(part->id());

    Events events = retrieveEventsForElement(chord, midiChannel);
    midiData.stream.backgroundStream.send(std::move(events), Ms::MScore::defaultPlayDuration* 2);

    return make_ret(Ret::Code::Ok);
}

Ret MasterNotationMidiData::playHarmonyMidiData(const Ms::Harmony* harmony) const
{
    if (!harmony->isRealizable()) {
        return make_ret(Err::UnableToPlaybackElement);
    }

    const Ms::Channel* hChannel = harmony->part()->harmonyChannel();
    if (!hChannel) {
        return make_ret(Err::UnableToPlaybackElement);
    }

    const Ms::Instrument* instr = harmony->part() ? harmony->part()->instrument() : nullptr;
    if (!instr) {
        return make_ret(Err::UnableToPlaybackElement);
    }

    channel_t midiChannel = hChannel->channel();
    MidiData midiData = trackMidiData(harmony->part()->id());

    Events events = retrieveEventsForElement(harmony, midiChannel);
    midiData.stream.backgroundStream.send(std::move(events), Ms::MScore::defaultPlayDuration* 2);

    return make_ret(Ret::Code::Ok);
}

void MasterNotationMidiData::loadEvents(const tick_t fromTick, const tick_t toTick) const
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

bool MasterNotationMidiData::hasCachedEvents(const channel_t midiChannel) const
{
    return m_eventsCache.find(midiChannel) != m_eventsCache.end();
}

Ms::EventMap MasterNotationMidiData::renderMsEvents(const tick_t fromTick, const tick_t toTick) const
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

Events MasterNotationMidiData::convertMsEvents(Ms::EventMap&& eventMap) const
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

            Ms::EventType etype = static_cast<Ms::EventType>(ev.type());
            static const std::set<Ms::EventType> SKIP_EVENTS
                = { Ms::EventType::ME_INVALID, Ms::EventType::ME_EOT, Ms::EventType::ME_TICK1, Ms::EventType::ME_TICK2 };

            if (SKIP_EVENTS.find(etype) != SKIP_EVENTS.end()) {
                continue;
            }

            midi::Event e = midi::Event::fromMIDI10Package(ev.toPackage());

            events.push_back(std::move(e));
        }

        result.insert({ tick, std::move(events) });
    }

    return result;
}

//! NOTE Copied from MuseScore::play(Element* e, int pitch)
Events MasterNotationMidiData::eventsFromNote(const Element* noteElement, const channel_t midiChannel) const
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
Events MasterNotationMidiData::eventsFromChord(const Element* chordElement, const channel_t midiChannel) const
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
Events MasterNotationMidiData::eventsFromHarmony(const Element* harmonyElement, const channel_t midiChannel) const
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

MasterNotationMidiData::TickRangeMap MasterNotationMidiData::RenderRangeMap::unrenderedGaps(const tick_t from, const tick_t to) const
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

void MasterNotationMidiData::RenderRangeMap::insertOrReplace(const tick_t key, MasterNotationMidiData::TicksRange&& newRange)
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

void MasterNotationMidiData::RenderRangeMap::clear()
{
    m_renderedRanges.clear();
}
