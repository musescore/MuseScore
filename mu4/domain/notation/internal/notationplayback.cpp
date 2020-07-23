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

#include "audio/midi/event.h" //! TODO Remove me

using namespace mu::domain::notation;
using namespace mu::audio::midi;

static EventType convertType(int type)
{
    switch (type) {
    case Ms::ME_NOTEON:       return ME_NOTEON;
    case Ms::ME_NOTEOFF:      return ME_NOTEOFF;
    case Ms::ME_CONTROLLER:   return ME_CONTROLLER;
    case Ms::ME_PITCHBEND:    return ME_PITCHBEND;
    case Ms::ME_META:         return ME_META;

    case Ms::ME_TICK1:        return ME_INVALID;
    case Ms::ME_TICK2:        return ME_INVALID;
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

std::shared_ptr<MidiStream> NotationPlayback::midiStream() const
{
    Ms::Score* score = m_getScore->score();
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
    MetaInfo meta;
    makeMetaInfo(meta, score);

    data.division = Ms::MScore::division;
    data.tracks.resize(meta.tracksCount);

    Ms::EventMap eventMap;
    makeEventMap(eventMap, score);
    fillTracks(data.tracks, eventMap, meta);

    fillTempoMap(data.tempomap, score);

    //! TODO Not implemented, left not to be forgotten
    //fillMetronome(data.metronome, score, midiSpec);
}

void NotationPlayback::makeEventMap(Ms::EventMap& eventMap, Ms::Score* score) const
{
    score->masterScore()->setExpandRepeats(true);
    score->renderMidi(&eventMap, Ms::SynthesizerState());
}

void NotationPlayback::makeMetaInfo(MetaInfo& meta, const Ms::Score* score) const
{
    auto parts = score->parts();

    meta.tracksCount = parts.size();

    auto bankForInstrument = [](const Ms::Instrument* instr) {
                                 //! NOTE Temporary solution
                                 if (instr->useDrumset()) {
                                     return 128;
                                 }
                                 return 0;
                             };

    for (int pi = 0; pi < parts.size(); ++pi) {
        const Ms::Part* part = parts.at(pi);

        const Ms::InstrumentList* instList = part->instruments();
        for (auto it = instList->cbegin(); it != instList->cend(); ++it) {
            const Ms::Instrument* instrument = it->second;

            uint16_t bank = bankForInstrument(instrument);

            for (const Ms::Channel* ch : instrument->channel()) {
                ChanInfo chi;
                chi.trackIdx = pi;
                chi.bank = bank;
                chi.program = ch->program();

                meta.channels.insert({ ch->channel(), chi });
            }
        }
    }
}

void NotationPlayback::fillTracks(std::vector<audio::midi::Track>& tracks, const Ms::EventMap& eventMap,
                                  const MetaInfo& meta) const
{
    uint16_t ch_num = 1; //! NOTE channel 0 reserved for metronome
    auto findOrAddChannel = [&ch_num](audio::midi::Track& t, const ChanInfo& chi) -> audio::midi::Channel& {
                                for (auto& ch : t.channels) {
                                    if (ch.program == chi.program && ch.bank == chi.bank) {
                                        return ch;
                                    }
                                }

                                audio::midi::Channel ch;
                                ch.num = ch_num;
                                ch.program = chi.program;
                                ch.bank = chi.bank;

                                ++ch_num;

                                t.channels.push_back(std::move(ch));
                                return t.channels.back();
                            };

    for (const auto& evp : eventMap) {
        int tick = evp.first;
        const Ms::NPlayEvent ev = evp.second;

        if (ev.type() == Ms::ME_CONTROLLER && ev.controller() == 2) {
            //! TODO Understand why these events
            continue;
        }

        auto foundIt = meta.channels.find(ev.channel());
        if (foundIt == meta.channels.end()) {
            Q_ASSERT(foundIt != meta.channels.end());
            continue;
        }

        const ChanInfo& chi = foundIt->second;

        audio::midi::Track& track = tracks.at(chi.trackIdx);
        audio::midi::Channel& ch = findOrAddChannel(track, chi);

        audio::midi::EventType etype = convertType(ev.type());
        if (audio::midi::ME_INVALID == etype) {
            continue;
        } else if (audio::midi::ME_META == etype) {
            continue;
        } else {
            audio::midi::Event e
            { static_cast<uint32_t>(tick),
              etype,
              ev.dataA(), ev.dataB()
            };

            ch.events.push_back(std::move(e));
        }
    }
}

void NotationPlayback::fillTempoMap(std::map<uint32_t, uint32_t>& tempos, const Ms::Score* score) const
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
            uint32_t tempo = (uint32_t)lrint((1.0 / (it->second.tempo * relTempo)) * 1000000.0);

            tempos.insert({ it->first + tickOffset, tempo });
        }
    }
}

//! NOTE Copied from ScoreView::moveCursor(const Fraction& tick)
QRect NotationPlayback::playbackCursorRect(float sec) const
{
    using namespace Ms;

    Score* score = m_getScore->score();
    if (!score) {
        return QRect();
    }

    int _tick = score->utime2utick(sec);
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
                x2 = measure->canvasPos().x() + measure->width();         //safety, should not happen
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
