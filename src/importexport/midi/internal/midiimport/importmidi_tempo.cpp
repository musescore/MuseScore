/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "importmidi_tempo.h"

#include <map>

#include "importmidi_inner.h"
#include "importmidi_beat.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/factory.h"
#include "engraving/types/constants.h"
#include "importmidi_operations.h"

#include "modularity/ioc.h"
#include "imidiconfiguration.h"

#include "global/realfn.h"
#include "global/log.h"

using namespace mu::engraving;

namespace mu::iex::midi {
namespace MidiTempo {
ReducedFraction time2Tick(double time, double ticksPerSec)
{
    return ReducedFraction::fromTicks(int(ticksPerSec * time));
}

// tempo in beats per second

double findBasicTempo(const std::multimap<int, MTrack>& tracks, bool isHumanPerformance)
{
    for (const auto& track: tracks) {
        // don't read tempo from tempo track for human performed files
        // because very often the tempo in such track is completely erroneous
        if (isHumanPerformance && track.second.chords.empty()) {
            continue;
        }
        for (const auto& ie : track.second.mtrack->events()) {
            const MidiEvent& e = ie.second;
            if (e.type() == ME_META && e.metaType() == META_TEMPO) {
                const uchar* data = (uchar*)e.edata();
                const unsigned tempo = data[2] + (data[1] << 8) + (data[0] << 16);
                return 1000000.0 / double(tempo);
            }
        }
    }

    return 2;     // default beats per second = 120 beats per minute
}

static void setTempoToScore(Score* score, int tick, double beatsPerSecond, const bool roundTempo, BeatsPerSecond& lastTempo)
{
    Measure* measure = score->tick2measure(Fraction::fromTicks(tick));
    if (!measure) {
        LOGD("MidiTempo::setTempoToScore: no measure for tick %d", tick);
        return;
    }

    const Fraction f = Fraction::fromTicks(tick);
    Segment* existingSegment = measure->findSegment(SegmentType::ChordRest, f);
    if (!existingSegment) {
        existingSegment = measure->findSegment(SegmentType::TimeTick, f);
    }

    if (existingSegment) {
        for (EngravingItem* e : existingSegment->annotations()) {
            if (e->isTempoText()) {
                lastTempo = toTempoText(e)->tempo();
                return;          // already have a tempo marking here
            }
        }
    }

    // don't repeat tempo, always set only tempo for tick 0
    if (tick > 0 && lastTempo == beatsPerSecond) {
        return;
    }

    auto* data = midiImportOperations.data();
    const bool showTempoText = data->trackOpers.showTempoText.value();

    if (!showTempoText && tick == 0 && muse::RealIsEqual(beatsPerSecond, Constants::DEFAULT_TEMPO.val)) {
        return;
    }

    const double tempoInBpm = roundTempo ? qRound(beatsPerSecond * 60.0) : (beatsPerSecond * 60.0);

    Segment* segment = measure->getChordRestOrTimeTickSegment(f);
    TempoText* tempoText = mu::engraving::Factory::createTempoText(segment);
    tempoText->setTempo(beatsPerSecond);
    tempoText->setXmlText(String(u"<sym>metNoteQuarterUp</sym> = %1").arg(tempoInBpm));
    tempoText->setTrack(0);
    tempoText->setVisible(showTempoText);
    segment->add(tempoText);

    lastTempo = beatsPerSecond;

    if (showTempoText) {
        data->hasTempoText = true;          // to show tempo text column in the MIDI import panel
    }
}

static inline double roundToBpm(double beatsPerSecond)
{
    return qRound(beatsPerSecond * 60.0) / 60.0;
}

static void applyAllTempoEvents(const std::multimap<int, MTrack>& tracks, Score* score, const bool roundTempo)
{
    // Tempo-only tracks all share the same key in `tracks`, so iterating it directly would visit
    // their events track-by-track rather than in score-tick order. Collect first, then apply in tick order.
    std::multimap<int, double> tempoEvents;     // score tick -> beats per second

    for (const auto& track: tracks) {
        if (track.second.isDivisionInTps) {         // ticks per second
            const double ticksPerBeat = Constants::DIVISION;
            const double beatsPerSecond = roundTempo
                                          ? roundToBpm(track.second.division / ticksPerBeat)
                                          : (track.second.division / ticksPerBeat);
            tempoEvents.emplace(0, beatsPerSecond);
        } else {        // beats per second
            for (const auto& ie : track.second.mtrack->events()) {
                const MidiEvent& e = ie.second;
                if (e.type() == ME_META && e.metaType() == META_TEMPO) {
                    const auto tick = toMuseScoreTicks(
                        ie.first, track.second.division, false);
                    const uchar* data = (uchar*)e.edata();
                    const unsigned tempo = data[2] + (data[1] << 8) + (data[0] << 16);
                    const double beatsPerSecond =  roundTempo
                                                  ? roundToBpm(1000000.0 / tempo)
                                                  : (1000000.0 / tempo);
                    tempoEvents.emplace(tick.ticks(), beatsPerSecond);
                }
            }
        }
    }

    BeatsPerSecond lastTempo = Constants::DEFAULT_TEMPO;
    for (const auto& [tick, beatsPerSecond] : tempoEvents) {
        setTempoToScore(score, tick, beatsPerSecond, roundTempo, lastTempo);
    }
}

void setTempo(const std::multimap<int, MTrack>& tracks, Score* score)
{
    muse::GlobalInject<mu::iex::midi::IMidiImportExportConfiguration> configuration;
    const bool roundTempo = configuration() ? configuration()->roundTempo() : true;

    auto* midiData = midiImportOperations.data();
    std::set<ReducedFraction> beats = midiData->humanBeatData.beatSet;      // copy

    if (beats.empty()) {
        // it's most likely not a human performance;
        // we find all tempo events and set tempo changes to score
        applyAllTempoEvents(tracks, score, roundTempo);
    } else {            // calculate and set tempo from adjusted beat locations
        if (midiData->trackOpers.measureCount2xLess.value()) {
            MidiBeat::removeEvery2ndBeat(beats);
        }

        Q_ASSERT_X(beats.size() > 1, "MidiBeat::setTempo", "Human beat count < 2");

        double averageTempoFactor = 0.0;
        int counter = 0;
        auto it = beats.begin();
        auto beatStart = *it;
        const auto newBeatLen = ReducedFraction::fromTicks(Constants::DIVISION);

        for (++it; it != beats.end(); ++it) {
            const auto& beatEnd = *it;

            Q_ASSERT_X(beatEnd > beatStart, "MidiBeat::detectTempoChanges",
                       "Beat end <= beat start that is incorrect");

            averageTempoFactor += (newBeatLen / (beatEnd - beatStart)).toDouble();
            ++counter;
            beatStart = beatEnd;
        }
        averageTempoFactor /= counter;

        const double basicTempo = MidiTempo::findBasicTempo(tracks, true);
        const double tempo = roundTempo ? roundToBpm(basicTempo * averageTempoFactor)
                             : (basicTempo * averageTempoFactor);

        BeatsPerSecond lastTempo = Constants::DEFAULT_TEMPO;
        setTempoToScore(score, 0, tempo, roundTempo, lastTempo);
    }
}
} // namespace MidiTempo
} // namespace mu::iex::midi
