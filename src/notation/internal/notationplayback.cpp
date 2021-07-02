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
#include "notationplayback.h"

#include <cmath>

#include "log.h"

#include "libmscore/rendermidi.h"
#include "libmscore/score.h"
#include "libmscore/tempo.h"
#include "libmscore/part.h"
#include "libmscore/instrument.h"
#include "libmscore/repeatlist.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "libmscore/system.h"
#include "libmscore/scorefont.h"
#include "libmscore/page.h"
#include "libmscore/staff.h"
#include "libmscore/chordrest.h"
#include "libmscore/chord.h"
#include "libmscore/harmony.h"
#include "libmscore/tempotext.h"
#include "libmscore/tempo.h"

#include "framework/midi_old/event.h" //! TODO Remove me

#include "notationerrors.h"

using namespace mu;
using namespace mu::notation;
using namespace mu::midi;
using namespace mu::async;

NotationPlayback::NotationPlayback(IGetScore* getScore,
                                   async::Notification notationChanged,
                                   INotationMidiEventsPtr midiDataProvider)
    : m_getScore(getScore), m_midiEventsProvider(std::move(midiDataProvider))
{
    notationChanged.onNotify(this, [this]() {
        updateLoopBoundaries();
    });
}

NotationPlayback::~NotationPlayback()
{
    m_notationParts = nullptr;
}

Ms::Score* NotationPlayback::score() const
{
    return m_getScore->score();
}

Ms::MasterScore* NotationPlayback::masterScore() const
{
    return score() ? score()->masterScore() : nullptr;
}

void NotationPlayback::init(INotationPartsPtr parts)
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    m_notationParts = parts;

    m_midiEventsProvider->init();

    QObject::connect(score(), &Ms::Score::posChanged, [this](Ms::POS pos, int tick) {
        if (Ms::POS::CURRENT == pos) {
            m_playPositionTickChanged.send(tick);
        } else {
            updateLoopBoundaries();
        }
    });

    load();
}

MidiData mu::notation::NotationPlayback::buildMidiData(const Ms::Part* part) const
{
    return { buildMidiMapping(part), buildMidiStream(part) };
}

MidiMapping NotationPlayback::buildMidiMapping(const Ms::Part* part) const
{
    midi::MidiMapping mapping;

    mapping.division = Ms::MScore::division;
    mapping.tempo = makeTempoMap();

    for (auto it = part->instruments()->cbegin(); it != part->instruments()->cend(); ++it) {
        const Ms::Instrument* instrument = it->second;

        for (const Ms::Channel* channel : instrument->channel()) {
            mapping.synthName = channel->synti().toStdString();
            mapping.programms.push_back({ static_cast<midi::channel_t>(channel->channel()),
                                          channel->program(),
                                          channel->bank() });
        }
    }

    return mapping;
}

MidiStream NotationPlayback::buildMidiStream(const Ms::Part* part) const
{
    midi::MidiStream stream;

    IF_ASSERT_FAILED(masterScore()->lastMeasure()) {
        return stream;
    }

    stream.lastTick = masterScore()->lastMeasure()->endTick().ticks() - 1;

    for (auto it = part->instruments()->cbegin(); it != part->instruments()->cend(); ++it) {
        const Ms::Instrument* instrument = it->second;

        std::vector<channel_t> midiChannels(instrument->channel().size());

        for (const Ms::Channel* channel : instrument->channel()) {
            channel_t midiChannel = channel->channel();
            midiChannels.push_back(midiChannel);
        }

        std::list<InstrumentChannel*> channelList(instrument->channel().begin(), instrument->channel().end());

        std::vector<Event> setupEvents = m_midiEventsProvider->retrieveSetupEvents(channelList);
        stream.controlEventsStream.set(std::move(setupEvents));

        stream.eventsRequest.onReceive(this, [this, stream, midiChannels](const tick_t fromTick, const tick_t toTick) mutable {
            if (fromTick >= stream.lastTick || toTick > stream.lastTick) {
                stream.mainStream.send({}, 0);
                return;
            }

            for (const channel_t& midiChannel : midiChannels) {
                Events events = m_midiEventsProvider->retrieveEvents(static_cast<channel_t>(midiChannel), fromTick, toTick);

                stream.mainStream.send(std::move(events), toTick);
            }
        });
    }

    return stream;
}

void NotationPlayback::load()
{
    IF_ASSERT_FAILED(m_notationParts) {
        return;
    }

    if (!score()) {
        return;
    }

    m_instrumentsMidiData.clear();

    for (const Part* part : m_notationParts->partList()) {
        m_instrumentsMidiData.insert({ part->id().toStdString(), buildMidiData(part) });
    }

    m_notationParts->partList().onItemAdded(this, [this](const Part* part) {
        InstrumentTrackId id = part->id().toStdString();
        m_instrumentsMidiData.insert({ id, buildMidiData(part) });
        m_instrumentTrackAdded.send(std::move(id));
    });

    m_notationParts->partList().onItemRemoved(this, [this](const Part* part) {
        InstrumentTrackId id = part->id().toStdString();
        m_instrumentsMidiData.erase(id);
        m_instrumentTrackRemoved.send(std::move(id));
    });
}

MidiData NotationPlayback::instrumentMidiData(const INotationPlayback::InstrumentTrackId& id) const
{
    auto search = m_instrumentsMidiData.find(id);

    if (search != m_instrumentsMidiData.end()) {
        return search->second;
    }

    return MidiData();
}

std::vector<INotationPlayback::InstrumentTrackId> NotationPlayback::instrumentTrackIdList() const
{
    std::vector<INotationPlayback::InstrumentTrackId> result;
    result.reserve(m_instrumentsMidiData.size());

    for (auto&& pair : m_instrumentsMidiData) {
        result.push_back(pair.first);
    }

    return result;
}

Channel<INotationPlayback::InstrumentTrackId> NotationPlayback::instrumentTrackRemoved() const
{
    return m_instrumentTrackRemoved;
}

Channel<INotationPlayback::InstrumentTrackId> NotationPlayback::instrumentTrackAdded() const
{
    return m_instrumentTrackAdded;
}

void NotationPlayback::updateLoopBoundaries()
{
    LoopBoundaries boundaries;
    boundaries.loopInTick = score()->loopInTick().ticks();
    boundaries.loopOutTick = score()->loopOutTick().ticks();

    if (boundaries.isNull()) {
        return;
    }

    boundaries.loopInRect = loopBoundaryRectByTick(LoopBoundaryType::LoopIn, boundaries.loopInTick);
    boundaries.loopOutRect = loopBoundaryRectByTick(LoopBoundaryType::LoopOut, boundaries.loopOutTick);
    boundaries.visible = m_loopBoundaries.val.visible;

    if (m_loopBoundaries.val != boundaries) {
        m_loopBoundaries.set(boundaries);
    }
}

int NotationPlayback::instrumentBank(const Ms::Instrument* instrument) const
{
    //! NOTE Temporary solution
    if (instrument->useDrumset()) {
        return 128;
    }
    return 0;
}

TempoMap NotationPlayback::makeTempoMap() const
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

QTime NotationPlayback::totalPlayTime() const
{
    QTime time(0, 0, 0, 0);

    Ms::Score* score = m_getScore->score();
    if (!score) {
        return time;
    }

    int lastTick = score->repeatList().ticks();
    int secs = score->utick2utime(lastTick);

    return time.addSecs(secs);
}

float NotationPlayback::tickToSec(tick_t tick) const
{
    return score() ? score()->utick2utime(tick) : 0.0;
}

tick_t NotationPlayback::secToTick(float sec) const
{
    return score() ? score()->utime2utick(sec) : 0;
}

//! NOTE Copied from ScoreView::moveCursor(const Fraction& tick)
QRect NotationPlayback::playbackCursorRectByTick(tick_t _tick) const
{
    if (!score()) {
        return QRect();
    }

    Fraction tick = Fraction::fromTicks(_tick);

    Measure* measure = score()->tick2measureMM(tick);
    if (!measure) {
        return QRect();
    }

    Ms::System* system = measure->system();
    if (!system) {
        return QRect();
    }

    qreal x = 0.0;
    Ms::Segment* s = nullptr;
    for (s = measure->first(Ms::SegmentType::ChordRest); s;) {
        Fraction t1 = s->tick();
        int x1 = s->canvasPos().x();
        qreal x2;
        Fraction t2;
        Ms::Segment* ns = s->next(Ms::SegmentType::ChordRest);
        while (ns && !ns->visible()) {
            ns = ns->next(Ms::SegmentType::ChordRest);
        }
        if (ns) {
            t2 = ns->tick();
            x2 = ns->canvasPos().x();
        } else {
            t2 = measure->endTick();
            // measure->width is not good enough because of courtesy keysig, timesig
            Ms::Segment* seg = measure->findSegment(Ms::SegmentType::EndBarLine, measure->tick() + measure->ticks());
            if (seg) {
                x2 = seg->canvasPos().x();
            } else {
                x2 = measure->canvasPos().x() + measure->width();             //safety, should not happen
            }
        }
        if (tick >= t1 && tick < t2) {
            Fraction dt = t2 - t1;
            qreal dx = x2 - x1;
            x = x1 + dx * (tick - t1).ticks() / dt.ticks();
            break;
        }
        s = ns;
    }

    if (!s) {
        return QRect();
    }

    double y = system->staffYpage(0) + system->page()->pos().y();
    double _spatium = score()->spatium();

    qreal mag = _spatium / Ms::SPATIUM20;
    double w  = _spatium * 2.0 + score()->scoreFont()->width(Ms::SymId::noteheadBlack, mag);
    double h  = 6 * _spatium;
    //
    // set cursor height for whole system
    //
    double y2 = 0.0;

    for (int i = 0; i < score()->nstaves(); ++i) {
        Ms::SysStaff* ss = system->staff(i);
        if (!ss->show() || !score()->staff(i)->show()) {
            continue;
        }
        y2 = ss->bbox().bottom();
    }
    h += y2;
    x -= _spatium;
    y -= 3 * _spatium;

    return QRect(x, y, w, h);
}

RetVal<midi::tick_t> NotationPlayback::playPositionTickByElement(const Element* element) const
{
    RetVal<tick_t> result;
    result.ret = make_ret(Err::Undefined);
    result.val = 0;

    IF_ASSERT_FAILED(element) {
        return result;
    }

    if (!score()) {
        return result;
    }

    //! NOTE Copied from void ScoreView::mousePressEvent(QMouseEvent* ev)  case ViewState::PLAY: {
    if (!(element->isNote() || element->isRest())) {
        return result;
    }

    if (element->isNote()) {
        element = element->parent();
    }

    const Ms::ChordRest* cr = Ms::toChordRest(element);

    int ticks = score()->repeatList().tick2utick(cr->tick().ticks());

    return result.make_ok(std::move(ticks));
}

Ret NotationPlayback::playElementMidiData(const Element* element)
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

//! NOTE Copied from MuseScore::play(Element* e, int pitch)
Ret NotationPlayback::playNoteMidiData(const Ms::Note* note) const
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

    MidiData midiData = instrumentMidiData(masterNote->part()->id().toStdString());

    Events events = m_midiEventsProvider->retrieveEventsForElement(masterNote, midiChannel);
    midiData.stream.backgroundStream.send(std::move(events), Ms::MScore::defaultPlayDuration* 2);

    return make_ret(Ret::Code::Ok);
}

Ret NotationPlayback::playChordMidiData(const Ms::Chord* chord) const
{
    Ms::Part* part = chord->part();
    Ms::Fraction tick = chord->segment() ? chord->segment()->tick() : Ms::Fraction(0, 1);
    Ms::Instrument* instr = part->instrument(tick);
    channel_t midiChannel = instr->channel(chord->notes()[0]->subchannel())->channel();

    MidiData midiData = instrumentMidiData(part->id().toStdString());

    Events events = m_midiEventsProvider->retrieveEventsForElement(chord, midiChannel);
    midiData.stream.backgroundStream.send(std::move(events), Ms::MScore::defaultPlayDuration* 2);

    return make_ret(Ret::Code::Ok);
}

Ret NotationPlayback::playHarmonyMidiData(const Ms::Harmony* harmony) const
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
    MidiData midiData = instrumentMidiData(harmony->part()->id().toStdString());

    Events events = m_midiEventsProvider->retrieveEventsForElement(harmony, midiChannel);
    midiData.stream.backgroundStream.send(std::move(events), Ms::MScore::defaultPlayDuration* 2);

    return make_ret(Ret::Code::Ok);
}

void NotationPlayback::addLoopBoundary(LoopBoundaryType boundaryType, tick_t tick)
{
    if (tick == BoundaryTick::FirstScoreTick) {
        tick = score()->firstMeasure()->tick().ticks();
    } else if (tick == BoundaryTick::LastScoreTick) {
        tick = score()->lastMeasure()->endTick().ticks();
    }

    switch (boundaryType) {
    case LoopBoundaryType::LoopIn:
        addLoopIn(tick);
        break;
    case LoopBoundaryType::LoopOut:
        addLoopOut(tick);
        break;
    case LoopBoundaryType::Unknown:
        break;
    }
}

void NotationPlayback::addLoopIn(int _tick)
{
    Fraction tick = Fraction::fromTicks(_tick);

    if (_tick == BoundaryTick::SelectedNoteTick) {
        tick = score()->pos();
    }

    if (tick >= score()->loopOutTick()) { // If In pos >= Out pos, reset Out pos to end of score
        score()->setLoopOutTick(score()->lastMeasure()->endTick());
    }

    score()->setLoopInTick(tick);
}

void NotationPlayback::addLoopOut(int _tick)
{
    Fraction tick = Fraction::fromTicks(_tick);

    if (_tick == BoundaryTick::SelectedNoteTick) {
        tick = score()->pos() + score()->inputState().ticks();
    }

    if (tick <= score()->loopInTick()) { // If Out pos <= In pos, reset In pos to beginning of score
        score()->setLoopInTick(Fraction(0, 1));
    } else {
        if (tick > score()->lastMeasure()->endTick()) {
            tick = score()->lastMeasure()->endTick();
        }
    }

    score()->setLoopOutTick(tick);
}

void NotationPlayback::setLoopBoundariesVisible(bool visible)
{
    if (m_loopBoundaries.val.visible == visible) {
        return;
    }

    m_loopBoundaries.val.visible = visible;
    m_loopBoundaries.set(m_loopBoundaries.val);
}

QRect NotationPlayback::loopBoundaryRectByTick(LoopBoundaryType boundaryType, int _tick) const
{
    Fraction tick = Fraction::fromTicks(_tick);

    // set mark height for whole system
    if (boundaryType == LoopBoundaryType::LoopOut && tick > Fraction(0, 1)) {
        tick -= Fraction::fromTicks(1);
    }

    Measure* measure = score()->tick2measureMM(tick);
    if (measure == nullptr) {
        return QRect();
    }

    qreal x = 0.0;
    const Fraction offset = { 0, 1 };

    Ms::Segment* s = nullptr;
    for (s = measure->first(Ms::SegmentType::ChordRest); s;) {
        Fraction t1 = s->tick();
        int x1 = s->canvasPos().x();
        qreal x2 = 0;
        Fraction t2;
        Ms::Segment* ns = s->next(Ms::SegmentType::ChordRest);
        if (ns) {
            t2 = ns->tick();
            x2 = ns->canvasPos().x();
        } else {
            t2 = measure->endTick();
            x2 = measure->canvasPos().x() + measure->width();
        }
        t1 += offset;
        t2 += offset;
        if (tick >= t1 && tick < t2) {
            Fraction dt = t2 - t1;
            qreal dx = x2 - x1;
            x = x1 + dx * (tick - t1).ticks() / dt.ticks();
            break;
        }
        s = ns;
    }

    if (s == nullptr) {
        return QRect();
    }

    Ms::System* system = measure->system();
    if (system == nullptr || system->page() == nullptr) {
        return QRect();
    }

    double y = system->staffYpage(0) + system->page()->pos().y();
    double _spatium = score()->spatium();

    qreal mag = _spatium / Ms::SPATIUM20;
    double width = (_spatium * 2.0 + score()->scoreFont()->width(Ms::SymId::noteheadBlack, mag)) / 3;
    double height = 6 * _spatium;

    // set cursor height for whole system
    double y2 = 0.0;

    for (int i = 0; i < score()->nstaves(); ++i) {
        Ms::SysStaff* ss = system->staff(i);
        if (!ss->show() || !score()->staff(i)->show()) {
            continue;
        }
        y2 = ss->y() + ss->bbox().height();
    }
    height += y2;
    y -= 3 * _spatium;

    if (boundaryType == LoopBoundaryType::LoopIn) {
        x = x - _spatium + width / 1.5;
    } else {
        x = x - _spatium * .5;
    }

    return QRect(x, y, width, height);
}

mu::ValCh<LoopBoundaries> NotationPlayback::loopBoundaries() const
{
    return m_loopBoundaries;
}

Tempo NotationPlayback::tempo(tick_t tick) const
{
    Tempo tempo;

    if (!score()) {
        return tempo;
    }

    const Ms::TempoText* tempoText = this->tempoText(tick);
    Ms::TDuration duration = tempoText ? tempoText->duration() : Ms::TDuration();

    if (!tempoText || !duration.isValid()) {
        tempo.duration = DurationType::V_QUARTER;
        tempo.valueBpm = std::round(score()->tempo(Fraction::fromTicks(tick)) * 60.);
        return tempo;
    }

    tempo.valueBpm = tempoText->tempoBpm();
    tempo.duration = duration.type();
    tempo.withDot = duration.dots() > 0;

    return tempo;
}

const Ms::TempoText* NotationPlayback::tempoText(int _tick) const
{
    Fraction tick = Fraction::fromTicks(_tick);
    Ms::TempoText* result = nullptr;

    Ms::SegmentType segmentType = Ms::SegmentType::All;
    for (const Ms::Segment* segment = score()->firstSegment(segmentType); segment; segment = segment->next1(segmentType)) {
        for (Ms::Element* element: segment->annotations()) {
            if (element && element->isTempoText() && element->tick() <= tick) {
                result = Ms::toTempoText(element);
            }
        }
    }

    return result;
}

MeasureBeat NotationPlayback::beat(tick_t tick) const
{
    MeasureBeat measureBeat;

    if (score() && score()->checkHasMeasures()) {
        int dummy = 0;
        score()->sigmap()->tickValues(tick, &measureBeat.measureIndex, &measureBeat.beatIndex, &dummy);

        measureBeat.maxMeasureIndex = score()->measures()->size() - 1;
        measureBeat.maxBeatIndex = score()->sigmap()->timesig(Fraction::fromTicks(tick)).timesig().numerator() - 1;
    }

    return measureBeat;
}

tick_t NotationPlayback::beatToTick(int measureIndex, int beatIndex) const
{
    return score() ? score()->sigmap()->bar2tick(measureIndex, beatIndex) : 0;
}
