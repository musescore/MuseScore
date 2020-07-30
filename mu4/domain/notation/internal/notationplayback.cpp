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

#include "audio/midi/event.h" //! TODO Remove me

#include "notationerrors.h"

using namespace mu::domain::notation;
using namespace mu::midi;

static EventType convertType(int type)
{
    switch (type) {
    case Ms::ME_NOTEON:       return ME_NOTEON;
    case Ms::ME_NOTEOFF:      return ME_NOTEOFF;
    case Ms::ME_CONTROLLER:   return ME_CONTROLLER;
    case Ms::ME_PITCHBEND:    return ME_PITCHBEND;
    case Ms::ME_META:         return ME_META;

    case Ms::ME_TICK1:        return ME_TICK1;
    case Ms::ME_TICK2:        return ME_TICK2;
    default: {
        LOGE() << "unknown midi type: " << type;
    }
    }
    return ME_INVALID;
}

NotationPlayback::NotationPlayback(IGetScore* getScore)
    : m_getScore(getScore)
{
}

void NotationPlayback::init()
{
    Ms::Score* score = m_getScore->masterScore();
    IF_ASSERT_FAILED(score) {
        return;
    }

    QObject::connect(score, &Ms::Score::posChanged, [this](Ms::POS pos, int tick) {
        if (Ms::POS::CURRENT == pos) {
            m_playPositionTickChanged.send(tick);
        }
    });
}

std::shared_ptr<MidiStream> NotationPlayback::midiStream() const
{
    Ms::Score* score = m_getScore->masterScore();
    if (!score) {
        return nullptr;
    }

    std::shared_ptr<MidiStream> stream = std::make_shared<MidiStream>();

    makeInitData(stream->initData, score);

    stream->request.onReceive(this, [stream](uint32_t tick) {
        UNUSED(tick);
        stream->stream.close();
    });

    return stream;
}

void NotationPlayback::makeInitData(MidiData& data, Ms::Score* score) const
{
    data.division = Ms::MScore::division;

    Ms::EventMap eventMap;
    makeEventMap(eventMap, score);

    makeTracks(data.tracks, score);
    makeEvents(data.events, eventMap);

    fillTempoMap(data.tempomap, score);

    //! TODO Not implemented, left not to be forgotten
    //fillMetronome(data.metronome, score, midiSpec);
}

void NotationPlayback::makeEventMap(Ms::EventMap& eventMap, Ms::Score* score) const
{
    score->masterScore()->setExpandRepeats(true);
    score->renderMidi(&eventMap, Ms::SynthesizerState());
}

int NotationPlayback::instrumentBank(const Ms::Instrument* instr) const
{
    //! NOTE Temporary solution
    if (instr->useDrumset()) {
        return 128;
    }
    return 0;
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

            uint16_t bank = instrumentBank(instrument);

            for (const Ms::Channel* ch : instrument->channel()) {
                Program p;
                p.channel = ch->channel();
                p.program = ch->program();
                p.bank = bank;

                t.programs.push_back(std::move(p));
            }
        }

        tracks.push_back(std::move(t));
    }
}

void NotationPlayback::makeEvents(midi::Events& events, const Ms::EventMap& msevents) const
{
    for (const auto& evp : msevents) {
        int tick = evp.first;
        const Ms::NPlayEvent ev = evp.second;

        if (ev.type() == Ms::ME_CONTROLLER && ev.controller() == 2) {
            //! TODO Understand why these events
            continue;
        }

        midi::EventType etype = convertType(ev.type());
        if (midi::ME_INVALID == etype) {
            continue;
        } else if (midi::ME_META == etype) {
            continue;
        } else {
            midi::Event e
            { static_cast<uint16_t>(ev.channel()),
              etype,
              ev.dataA(), ev.dataB()
            };

            events.insert({ tick, std::move(e) });
        }
    }
}

void NotationPlayback::fillTempoMap(std::map<tick_t, tempo_t>& tempos, const Ms::Score* score) const
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

float NotationPlayback::tickToSec(int tick) const
{
    Ms::Score* score = m_getScore->masterScore();
    if (!score) {
        return 0.0f;
    }

    return score->utick2utime(tick);
}

int NotationPlayback::secToTick(float sec) const
{
    Ms::Score* score = m_getScore->masterScore();
    if (!score) {
        return 0;
    }

    return score->utime2utick(sec);
}

//! NOTE Copied from ScoreView::moveCursor(const Fraction& tick)
QRect NotationPlayback::playbackCursorRectByTick(int _tick) const
{
    using namespace Ms;

    Ms::Score* score = m_getScore->masterScore();
    if (!score) {
        return QRect();
    }

    Fraction tick = Fraction::fromTicks(_tick);

    Measure* measure = score->tick2measureMM(tick);
    if (!measure) {
        return QRect();
    }

    System* system = measure->system();
    if (!system) {
        return QRect();
    }

    qreal x = 0.0;
    Segment* s = nullptr;
    for (s = measure->first(Ms::SegmentType::ChordRest); s;) {
        Fraction t1 = s->tick();
        int x1 = s->canvasPos().x();
        qreal x2;
        Fraction t2;
        Segment* ns = s->next(SegmentType::ChordRest);
        while (ns && !ns->visible()) {
            ns = ns->next(SegmentType::ChordRest);
        }
        if (ns) {
            t2 = ns->tick();
            x2 = ns->canvasPos().x();
        } else {
            t2 = measure->endTick();
            // measure->width is not good enough because of courtesy keysig, timesig
            Segment* seg = measure->findSegment(SegmentType::EndBarLine, measure->tick() + measure->ticks());
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
    double _spatium = score->spatium();

    qreal mag = _spatium / SPATIUM20;
    double w  = _spatium * 2.0 + score->scoreFont()->width(SymId::noteheadBlack, mag);
    double h  = 6 * _spatium;
    //
    // set cursor height for whole system
    //
    double y2 = 0.0;

    for (int i = 0; i < score->nstaves(); ++i) {
        SysStaff* ss = system->staff(i);
        if (!ss->show() || !score->staff(i)->show()) {
            continue;
        }
        y2 = ss->bbox().bottom();
    }
    h += y2;
    x -= _spatium;
    y -= 3 * _spatium;

    return QRect(x, y, w, h);
}

mu::RetVal<int> NotationPlayback::playPositionTick() const
{
    Ms::MasterScore* score = m_getScore->masterScore();
    if (!score) {
        return RetVal<int>(make_ret(Err::NoScore));
    }

    return RetVal<int>::make_ok(score->playPos().ticks());
}

void NotationPlayback::setPlayPositionTick(int tick)
{
    Ms::MasterScore* score = m_getScore->masterScore();
    if (!score) {
        return;
    }

    score->setPlayPos(Ms::Fraction::fromTicks(tick));
}

bool NotationPlayback::setPlayPositionByElement(const Element* e)
{
    IF_ASSERT_FAILED(e) {
        return false;
    }

    Ms::MasterScore* score = m_getScore->masterScore();
    if (!score) {
        return false;
    }

    //! NOTE Copied from void ScoreView::mousePressEvent(QMouseEvent* ev)  case ViewState::PLAY: {
    if (!(e->isNote() || e->isRest())) {
        return false;
    }

    if (e->isNote()) {
        e = e->parent();
    }

    const Ms::ChordRest* cr = Ms::toChordRest(e);

    int ticks = score->repeatList().tick2utick(cr->tick().ticks());
    score->setPlayPos(Ms::Fraction::fromTicks(ticks));

    return true;
}

mu::async::Channel<int> NotationPlayback::playPositionTickChanged() const
{
    return m_playPositionTickChanged;
}

MidiData NotationPlayback::playElementMidiData(const Element* e) const
{
    if (e->isNote()) {
        const Ms::Note* note = Ms::toNote(e);
        IF_ASSERT_FAILED(note) {
            return MidiData();
        }
        return playNoteMidiData(note);
    } else if (e->isChord()) {
        const Ms::Chord* chord = Ms::toChord(e);
        IF_ASSERT_FAILED(chord) {
            return MidiData();
        }
        return playChordMidiData(chord);
    } else if (e->isHarmony()) {
        const Ms::Harmony* h = Ms::toHarmony(e);
        IF_ASSERT_FAILED(h) {
            return MidiData();
        }
        return playHarmonyMidiData(h);
    }

    NOT_SUPPORTED << e->name();
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

    Ms::Fraction tick = masterNote->chord()->tick();
    if (tick < Ms::Fraction(0,1)) {
        tick = Ms::Fraction(0,1);
    }

    Ms::Instrument* instr = masterNote->part()->instrument(tick);
    const Ms::Channel* msCh = instr->channel(masterNote->subchannel());

    MidiData midiData;
    midiData.division = Ms::MScore::division;

    Program prog;
    prog.bank = instrumentBank(instr);
    prog.program = msCh->program();
    prog.channel = msCh->channel();

    midiData.events.insert({ 0, Event(prog.channel, ME_NOTEON, pitch, 80) });
    midiData.events.insert({ Ms::MScore::defaultPlayDuration, Event(prog.channel, ME_NOTEOFF, pitch, 0) });
    midiData.events.insert({ Ms::MScore::defaultPlayDuration*2, Event(prog.channel, MIDI_EOT, 0, 0) });

    Track track;
    track.programs.push_back(std::move(prog));
    midiData.tracks.push_back(std::move(track));

    return midiData;
}

MidiData NotationPlayback::playChordMidiData(const Ms::Chord* chord) const
{
    const Ms::Chord* c = Ms::toChord(chord);
    Ms::Part* part = c->staff()->part();
    Ms::Fraction tick = c->segment() ? c->segment()->tick() : Ms::Fraction(0,1);
    Ms::Instrument* instr = part->instrument(tick);

    MidiData midiData;
    midiData.division = Ms::MScore::division;
    Track track;
    for (Ms::Note* n : c->notes()) {
        const Ms::Channel* msCh = instr->channel(n->subchannel());

        channel_t channel = msCh->channel();
        auto found = std::find_if(track.programs.begin(), track.programs.end(), [channel](const Program& p) {
            return p.channel == channel;
        });

        if (found == track.programs.end()) {
            Program prog;
            prog.bank = instrumentBank(instr);
            prog.program = msCh->program();
            prog.channel = msCh->channel();
            track.programs.push_back(prog);
        }

        int pitch = n->ppitch();
        midiData.events.insert({ 0, Event(channel, ME_NOTEON, pitch, 80) });
        midiData.events.insert({ Ms::MScore::defaultPlayDuration, Event(channel, ME_NOTEOFF, pitch, 0) });
        midiData.events.insert({ Ms::MScore::defaultPlayDuration*2, Event(channel, MIDI_EOT, 0, 0) });
    }

    midiData.tracks.push_back(std::move(track));

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

    Program prog;
    prog.bank = 0;
    prog.program = hChannel->program();
    prog.channel = hChannel->channel();

    for (int pitch : pitches) {
        midiData.events.insert({ 0, Event(prog.channel, ME_NOTEON, pitch, 80) });
        midiData.events.insert({ Ms::MScore::defaultPlayDuration, Event(prog.channel, ME_NOTEOFF, pitch, 0) });
        midiData.events.insert({ Ms::MScore::defaultPlayDuration*2, Event(prog.channel, MIDI_EOT, 0, 0) });
    }

    Track track;
    track.programs.push_back(std::move(prog));

    midiData.tracks.push_back(std::move(track));

    return midiData;
}
