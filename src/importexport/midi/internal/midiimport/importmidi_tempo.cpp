/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "importmidi_inner.h"
#include "importmidi_beat.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/tempo.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/factory.h"
#include "importmidi_operations.h"

#include "log.h"

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

void setTempoToScore(Score* score, int tick, double beatsPerSecond)
{
    if (score->tempomap()->find(tick) != score->tempomap()->end()) {
        return;
    }
    // don't repeat tempo, always set only tempo for tick 0
    if (tick > 0 && score->tempo(Fraction::fromTicks(tick)) == beatsPerSecond) {
        return;
    }

    score->setTempo(Fraction::fromTicks(tick), beatsPerSecond);

    auto* data = midiImportOperations.data();
    if (data->trackOpers.showTempoText.value()) {
        const int tempoInBpm = qRound(beatsPerSecond * 60.0);

        Measure* measure = score->tick2measure(Fraction::fromTicks(tick));
        if (!measure) {
            LOGD("MidiTempo::setTempoToScore: no measure for tick %d", tick);
            return;
        }
        Segment* segment = measure->getSegment(SegmentType::ChordRest, Fraction::fromTicks(tick));
        if (!segment) {
            LOGD("MidiTempo::setTempoToScore: no chord/rest segment for tempo at %d", tick);
            return;
        }

        TempoText* tempoText = mu::engraving::Factory::createTempoText(segment);
        tempoText->setTempo(beatsPerSecond);
        tempoText->setXmlText(QString("<sym>metNoteQuarterUp</sym> = %1").arg(tempoInBpm));
        tempoText->setTrack(0);
        segment->add(tempoText);
        data->hasTempoText = true;          // to show tempo text column in the MIDI import panel
    }
}

double roundToBpm(double beatsPerSecond)
{
    return qRound(beatsPerSecond * 60.0) / 60.0;
}

void applyAllTempoEvents(const std::multimap<int, MTrack>& tracks, Score* score)
{
    for (const auto& track: tracks) {
        if (track.second.isDivisionInTps) {         // ticks per second
            const double ticksPerBeat = Constants::DIVISION;
            const double beatsPerSecond = roundToBpm(track.second.division / ticksPerBeat);
            setTempoToScore(score, 0, beatsPerSecond);
        } else {        // beats per second
            for (const auto& ie : track.second.mtrack->events()) {
                const MidiEvent& e = ie.second;
                if (e.type() == ME_META && e.metaType() == META_TEMPO) {
                    const auto tick = toMuseScoreTicks(
                        ie.first, track.second.division, false);
                    const uchar* data = (uchar*)e.edata();
                    const unsigned tempo = data[2] + (data[1] << 8) + (data[0] << 16);
                    const double beatsPerSecond = roundToBpm(1000000.0 / tempo);
                    setTempoToScore(score, tick.ticks(), beatsPerSecond);
                }
            }
        }
    }
}

void setTempo(const std::multimap<int, MTrack>& tracks, Score* score)
{
    score->tempomap()->clear();
    auto* midiData = midiImportOperations.data();
    std::set<ReducedFraction> beats = midiData->humanBeatData.beatSet;      // copy

    if (beats.empty()) {
        // it's most likely not a human performance;
        // we find all tempo events and set tempo changes to score
        applyAllTempoEvents(tracks, score);
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
        const double tempo = roundToBpm(basicTempo * averageTempoFactor);

        score->tempomap()->clear();             // use only one tempo marking for all score
        setTempoToScore(score, 0, tempo);
    }

    if (score->tempomap()->empty()) {
        score->tempomap()->setTempo(0, 2.0);          // default tempo
    }
}
} // namespace MidiTempo
} // namespace mu::iex::midi
