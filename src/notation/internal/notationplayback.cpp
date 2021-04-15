//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
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
#include "libmscore/sym.h"
#include "libmscore/page.h"
#include "libmscore/staff.h"
#include "libmscore/chordrest.h"
#include "libmscore/chord.h"
#include "libmscore/harmony.h"
#include "libmscore/tempotext.h"
#include "libmscore/tempo.h"

#include "framework/midi_old/event.h" //! TODO Remove me

#include "notationerrors.h"

using namespace mu::notation;
using namespace mu::midi;

static constexpr int MIN_CHUNK_SIZE(10); // measure

NotationPlayback::NotationPlayback(IGetScore* getScore, async::Notification notationChanged)
    : m_getScore(getScore)
{
    m_midiStream = std::make_shared<MidiStream>();
    m_midiStream->isStreamingAllowed = true;
    m_midiStream->request.onReceive(this, [this](tick_t tick) { onChunkRequest(tick); });

    notationChanged.onNotify(this, [this]() {
        updateLoopBoundaries();
    });
}

Ms::Score* NotationPlayback::score() const
{
    return m_getScore->score();
}

Ms::MasterScore* NotationPlayback::masterScore() const
{
    return score() ? score()->masterScore() : nullptr;
}

void NotationPlayback::init()
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    m_midiRenderer = std::unique_ptr<Ms::MidiRenderer>(new Ms::MidiRenderer(score()));
    m_midiRenderer->setMinChunkSize(MIN_CHUNK_SIZE);

    QObject::connect(score(), &Ms::Score::posChanged, [this](Ms::POS pos, int tick) {
        if (Ms::POS::CURRENT == pos) {
            m_playPositionTickChanged.send(tick);
        } else {
            updateLoopBoundaries();
        }
    });
}

void NotationPlayback::updateLoopBoundaries()
{
    LoopBoundaries boundaries;
    boundaries.loopInTick = score()->loopInTick().ticks();
    boundaries.loopOutTick = score()->loopOutTick().ticks();
    boundaries.loopInRect = loopBoundaryRectByTick(LoopBoundaryType::LoopIn, boundaries.loopInTick);
    boundaries.loopOutRect = loopBoundaryRectByTick(LoopBoundaryType::LoopOut, boundaries.loopOutTick);
    boundaries.visible = m_loopBoundaries.val.visible;

    if (m_loopBoundaries.val != boundaries) {
        m_loopBoundaries.set(boundaries);
    }
}

std::shared_ptr<MidiStream> NotationPlayback::midiStream() const
{
    if (!score()) {
        return nullptr;
    }

    IF_ASSERT_FAILED(m_midiRenderer) {
        return nullptr;
    }

    m_midiStream->initData = MidiData();
    m_midiRenderer->setScoreChanged();

    makeInitData(m_midiStream->initData, score());
    midi::Chunk firstChunk;
    makeChunk(firstChunk, 0 /*fromTick*/);
    m_midiStream->initData.chunks.insert({ firstChunk.beginTick, std::move(firstChunk) });

    m_midiStream->lastTick = score()->lastMeasure()->endTick().ticks();

    return m_midiStream;
}

void NotationPlayback::makeInitData(MidiData& data, Ms::Score* score) const
{
    data.division = Ms::MScore::division;

    makeInitEvents(data.initEvents, score);
    makeSynthMap(data.synthMap, score);
    makeTracks(data.tracks, score);
    makeTempoMap(data.tempoMap, score);

    //! TODO Not implemented, left not to be forgotten
    //fillMetronome(data.metronome, score, midiSpec);
}

int NotationPlayback::instrumentBank(const Ms::Instrument* instrument) const
{
    //! NOTE Temporary solution
    if (instrument->useDrumset()) {
        return 128;
    }
    return 0;
}

void NotationPlayback::makeInitEvents(std::vector<midi::Event>& events, const Ms::Score* score) const
{
    Ms::MasterScore* masterScore = score->masterScore();
    for (const Ms::MidiMapping& mm : masterScore->midiMapping()) {
        const Ms::Channel* channel = mm.articulation();
        for (const Ms::MidiCoreEvent& mse : channel->initList()) {
            if (mse.type() == Ms::ME_INVALID) {
                continue;
            }

            midi::Event e(channel->channel(), static_cast<midi::EventType>(mse.type()), mse.dataA(), mse.dataB());
            events.push_back(std::move(e));
        }
    }
}

void NotationPlayback::makeSynthMap(midi::SynthMap& synthMap, const Ms::Score* score) const
{
    Ms::MasterScore* masterScore = score->masterScore();
    for (const Ms::MidiMapping& mm : masterScore->midiMapping()) {
        const Ms::Channel* channel = mm.articulation();
        synthMap.insert({ channel->channel(), channel->synti().toStdString() });
    }
}

void NotationPlayback::makeTracks(std::vector<midi::Track>& tracks, const Ms::Score* score) const
{
    auto parts = score->parts();

    tracks.reserve(parts.size());

    for (int pi = 0; pi < parts.size(); ++pi) {
        midi::Track t;
        t.num = pi;

        const Ms::Part* part = parts.at(pi);
        const Ms::InstrumentList* instList = part->instruments();
        for (auto it = instList->cbegin(); it != instList->cend(); ++it) {
            const Ms::Instrument* instrument = it->second;
            for (const Ms::Channel* ch : instrument->channel()) {
                t.channels.push_back(ch->channel());
            }
        }

        tracks.push_back(std::move(t));
    }
}

void NotationPlayback::makeTempoMap(midi::TempoMap& tempos, const Ms::Score* score) const
{
    Ms::TempoMap* tempomap = score->tempomap();
    qreal relTempo = tempomap->relTempo();
    for (const Ms::RepeatSegment* rs : score->repeatList()) {
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
}

void NotationPlayback::onChunkRequest(tick_t tick)
{
    if (tick >= m_midiStream->lastTick) {
        m_midiStream->stream.send(midi::Chunk());
        return;
    }

    midi::Chunk chunk;
    makeChunk(chunk, tick);
    m_midiStream->stream.send(chunk);
}

void NotationPlayback::makeChunk(midi::Chunk& chunk, tick_t fromTick) const
{
    Ms::EventMap msevents;

    const Ms::MidiRenderer::Chunk mschunk = m_midiRenderer->chunkAt(fromTick);
    if (!mschunk) {
        return;
    }

    chunk.beginTick = mschunk.tick1();
    chunk.endTick = mschunk.tick2();

    masterScore()->setExpandRepeats(configuration()->isPlayRepeatsEnabled());

    Ms::SynthesizerState synState;// = mscore->synthesizerState();
    Ms::MidiRenderer::Context ctx(synState);
    ctx.metronome = configuration()->isMetronomeEnabled();
    ctx.renderHarmony = true;
    m_midiRenderer->renderChunk(mschunk, &msevents, ctx);

    for (const auto& evp : msevents) {
        tick_t tick = evp.first;
        const Ms::NPlayEvent ev = evp.second;

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
        chunk.events.insert({ tick, std::move(e) });
    }
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

float NotationPlayback::tickToSec(int tick) const
{
    return score() ? score()->utick2utime(tick) : 0.0;
}

int NotationPlayback::secToTick(float sec) const
{
    return score() ? score()->utime2utick(sec) : 0;
}

//! NOTE Copied from ScoreView::moveCursor(const Fraction& tick)
QRect NotationPlayback::playbackCursorRectByTick(int _tick) const
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

//! NOTE Copied from PositionCursor::move(const Fraction& t)
mu::RetVal<int> NotationPlayback::playPositionTick() const
{
    if (!score()) {
        return RetVal<int>(make_ret(Err::NoScore));
    }

    return RetVal<int>::make_ok(score()->playPos().ticks());
}

void NotationPlayback::setPlayPositionTick(int tick)
{
    if (!score()) {
        return;
    }

    score()->setPlayPos(Ms::Fraction::fromTicks(tick));
}

bool NotationPlayback::setPlayPositionByElement(const Element* element)
{
    IF_ASSERT_FAILED(element) {
        return false;
    }

    if (!score()) {
        return false;
    }

    //! NOTE Copied from void ScoreView::mousePressEvent(QMouseEvent* ev)  case ViewState::PLAY: {
    if (!(element->isNote() || element->isRest())) {
        return false;
    }

    if (element->isNote()) {
        element = element->parent();
    }

    const Ms::ChordRest* cr = Ms::toChordRest(element);

    int ticks = score()->repeatList().tick2utick(cr->tick().ticks());
    score()->setPlayPos(Ms::Fraction::fromTicks(ticks));

    return true;
}

mu::async::Channel<int> NotationPlayback::playPositionTickChanged() const
{
    return m_playPositionTickChanged;
}

MidiData NotationPlayback::playElementMidiData(const Element* element) const
{
    if (element->isNote()) {
        const Ms::Note* note = Ms::toNote(element);
        IF_ASSERT_FAILED(note) {
            return MidiData();
        }
        return playNoteMidiData(note);
    } else if (element->isChord()) {
        const Ms::Chord* chord = Ms::toChord(element);
        IF_ASSERT_FAILED(chord) {
            return MidiData();
        }
        return playChordMidiData(chord);
    } else if (element->isHarmony()) {
        const Ms::Harmony* h = Ms::toHarmony(element);
        IF_ASSERT_FAILED(h) {
            return MidiData();
        }
        return playHarmonyMidiData(h);
    }

    NOT_SUPPORTED << element->name();
    return MidiData();
}

//! NOTE Copied from MuseScore::play(Element* e, int pitch)
MidiData NotationPlayback::playNoteMidiData(const Ms::Note* note) const
{
    int pitch = note->ppitch();
    const Ms::Note* masterNote = note;
    if (note->linkList().size() > 1) {
        for (Ms::ScoreElement* se : note->linkList()) {
            if (se->score() == note->masterScore() && se->isNote()) {
                masterNote = Ms::toNote(se);
                break;
            }
        }
    }

    MidiData midiData;
    midiData.division = Ms::MScore::division;

    makeInitEvents(midiData.initEvents, masterNote->score());

    Ms::Fraction tick = masterNote->chord()->tick();
    if (tick < Ms::Fraction(0,1)) {
        tick = Ms::Fraction(0,1);
    }
    Ms::Instrument* instr = masterNote->part()->instrument(tick);
    channel_t channel = instr->channel(masterNote->subchannel())->channel();

    Track t;
    t.num = 0;
    t.channels.push_back(channel);
    midiData.tracks.push_back(t);

    Chunk chunk;
    chunk.beginTick = 0;
    chunk.endTick = Ms::MScore::defaultPlayDuration * 2;
    auto event = midi::Event(midi::Event::Opcode::NoteOn);
    event.setChannel(channel);
    event.setNote(pitch);
    event.setVelocityFraction(0.63f); //as 80 for 127 scale
    chunk.events.insert({ chunk.beginTick, event });

    event.setOpcode(midi::Event::Opcode::NoteOff);
    event.setVelocity(0);
    chunk.events.insert({ Ms::MScore::defaultPlayDuration, event });
    midiData.chunks.insert({ chunk.beginTick, std::move(chunk) });

    return midiData;
}

MidiData NotationPlayback::playChordMidiData(const Ms::Chord* chord) const
{
    Ms::Part* part = chord->part();
    Ms::Fraction tick = chord->segment() ? chord->segment()->tick() : Ms::Fraction(0,1);
    Ms::Instrument* instr = part->instrument(tick);
    channel_t channel = instr->channel(chord->notes()[0]->subchannel())->channel();

    MidiData midiData;
    midiData.division = Ms::MScore::division;
    makeInitEvents(midiData.initEvents, chord->score());

    Track t;
    t.num = 0;
    t.channels.push_back(channel);
    midiData.tracks.push_back(t);

    Chunk chunk;
    chunk.beginTick = 0;
    chunk.endTick = Ms::MScore::defaultPlayDuration * 2;
    for (Ms::Note* n : chord->notes()) {
        int pitch = n->ppitch();

        auto event = midi::Event(midi::Event::Opcode::NoteOn);
        event.setChannel(channel);
        event.setNote(pitch);
        event.setVelocityFraction(0.63f); //as 80 for 127 scale
        chunk.events.insert({ chunk.beginTick, event });

        event.setOpcode(midi::Event::Opcode::NoteOff);
        event.setVelocity(0);
        chunk.events.insert({ Ms::MScore::defaultPlayDuration, event });
        chunk.events.insert({ chunk.endTick, midi::Event::NOOP() });
    }

    midiData.chunks.insert({ chunk.beginTick, std::move(chunk) });

    return midiData;
}

MidiData NotationPlayback::playHarmonyMidiData(const Ms::Harmony* harmony) const
{
    const Ms::Harmony* h = Ms::toHarmony(harmony);
    if (!h->isRealizable()) {
        return MidiData();
    }

    const Ms::RealizedHarmony& r = h->getRealizedHarmony();
    QList<int> pitches = r.pitches();

    const Ms::Channel* hChannel = harmony->part()->harmonyChannel();
    if (!hChannel) {
        return MidiData();
    }

    MidiData midiData;
    midiData.division = Ms::MScore::division;

    makeInitEvents(midiData.initEvents, harmony->score());

    channel_t channel = hChannel->channel();

    Chunk chunk;
    chunk.beginTick = 0;
    chunk.endTick = Ms::MScore::defaultPlayDuration * 2;

    auto noteOn = midi::Event(midi::Event::Opcode::NoteOn);
    noteOn.setChannel(channel);
    noteOn.setVelocityFraction(0.63f); //as 80 for 127 scale

    auto noteOff = midi::Event(midi::Event::Opcode::NoteOff);
    noteOff.setChannel(channel);

    for (int pitch : pitches) {
        noteOn.setNote(pitch);
        noteOff.setNote(pitch);
        chunk.events.insert({ chunk.beginTick, noteOn });
        chunk.events.insert({ Ms::MScore::defaultPlayDuration, noteOff });
    }

    chunk.events.insert({ chunk.endTick, midi::Event::NOOP() });
    midiData.chunks.insert({ chunk.beginTick, std::move(chunk) });

    return midiData;
}

void NotationPlayback::addLoopBoundary(LoopBoundaryType boundaryType, int tick)
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

Tempo NotationPlayback::tempo(int tick) const
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

MeasureBeat NotationPlayback::beat(int tick) const
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

int NotationPlayback::beatToTick(int measureIndex, int beatIndex) const
{
    return score() ? score()->sigmap()->bar2tick(measureIndex, beatIndex) : 0;
}
