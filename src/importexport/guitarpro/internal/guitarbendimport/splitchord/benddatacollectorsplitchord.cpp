/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include "benddatacollectorsplitchord.h"

#include <engraving/dom/chord.h>
#include <engraving/dom/note.h>
#include <engraving/dom/score.h>
#include <engraving/dom/tie.h>
#include <engraving/dom/tuplet.h>

#include "../bendinfoconverter.h"
#include "bendchorddurationsplitter.h"

using namespace mu::engraving;

namespace mu::iex::guitarpro {
static void fillChordDurationsFromBendDiagram(BendDataContextSplitChord& bendDataCtx, Fraction totalDuration,
                                              const ImportedBendInfo& importedInfo);

static void fillBendDataForNote(BendDataContextSplitChord& bendDataCtx, const ImportedBendInfo& importedInfo, int noteIndexInChord);

void BendDataCollectorSplitChord::storeBendData(const mu::engraving::Note* note, const mu::engraving::PitchValues& pitchValues)
{
    if (!pitchValues.empty()) {
        m_bendInfoForNote[note->track()][note->tick()][note] = BendInfoConverter::fillBendInfo(note, pitchValues);
    }
}

BendDataContextSplitChord BendDataCollectorSplitChord::collectBendDataContext()
{
    BendDataContextSplitChord bendDataCtx;

    fillBendsDurations(bendDataCtx);
    fillBendData(bendDataCtx);

    return bendDataCtx;
}

static void fillBendDataForNote(BendDataContextSplitChord& bendDataCtx, const ImportedBendInfo& importedInfo, int noteIndexInChord)
{
    if (importedInfo.segments.empty()) {
        return;
    }

    const Note* note = importedInfo.note;
    const Chord* chord = note->chord();
    track_idx_t track = chord->track();

    const Fraction chordStartTick = chord->tick();
    const auto& durations = bendDataCtx.bendChordDurations;
    Fraction ratio = Fraction(1, 1);

    if (const Tuplet* tuplet = chord->tuplet()) {
        ratio = tuplet->ratio();
    }

    if (durations.find(chord->track()) == durations.end()) {
        LOGE() << "bend import error : no information about chord duration for track " << track;
        return;
    }

    const auto& trackDurations = durations.at(track);
    if (trackDurations.find(chordStartTick.ticks()) == trackDurations.end()) {
        LOGE() << "bend import error : no information about chord duration for track " << track << " tick " <<
            chordStartTick.ticks();
        return;
    }

    const auto& tickDurations = trackDurations.at(chordStartTick.ticks());

    if (tickDurations.empty()) {
        LOGE() << "bend import error : chord durations are empty for track " << track << " tick " <<
            chordStartTick.ticks();
        return;
    }

    IF_ASSERT_FAILED(tickDurations.size() > 1) {
        LOGE() << "not possible to split durations of chord";
        return;
    }

    Fraction currentTick = chordStartTick;
    for (size_t i = 0; i < tickDurations.size() - 1; i++) {
        if (i >= importedInfo.segments.size()) {
            break;
        }

        Fraction tickDuration = tickDurations[i] / ratio;

        const auto& seg = importedInfo.segments[i];

        BendChordData& bendChordData
            = bendDataCtx.bendDataByEndTick[note->track()][(currentTick + tickDuration).ticks()];
        bendChordData.startTick = currentTick;
        BendNoteData bendNoteData;
        bendNoteData.quarterTones = seg.endPitch / 25;
        bendChordData.noteDataByIdx[noteIndexInChord] = std::move(bendNoteData);

        currentTick += tickDuration;
    }
}

void BendDataCollectorSplitChord::fillBendData(BendDataContextSplitChord& bendDataCtx)
{
    for (auto& [track, trackInfo] : m_bendInfoForNote) {
        for (auto& [tick, tickInfo] : trackInfo) {
            for (auto& [note, dataByNote] : tickInfo) {
                Chord* chord = note->chord();
                int idx = static_cast<int>(muse::indexOf(chord->notes(), note));
                fillBendDataForNote(bendDataCtx, dataByNote, idx);
            }
        }
    }
}

void BendDataCollectorSplitChord::fillBendsDurations(BendDataContextSplitChord& bendDataCtx)
{
    std::unordered_map<mu::engraving::track_idx_t, std::map<Fraction, std::vector<const Chord*> > > tiedNotesAfterBend;
    for (const auto& [track, trackInfo] : m_bendInfoForNote) {
        for (const auto& [mainTick, tickInfo] : trackInfo) {
            // now using the top note of chord to fill durations
            const Note* currentNote = tickInfo.begin()->first;
            auto& currentTickTiedChords = tiedNotesAfterBend[track][mainTick];
            currentTickTiedChords.push_back(currentNote->chord());
            const Tie* tieFor = currentNote->tieFor();

            bool deleteLastTiedChord = true;
            while (tieFor) {
                currentNote = tieFor->endNote();

                IF_ASSERT_FAILED(currentNote) {
                    LOGE() << "invalid tie encountered while importing guitar bend";
                    break;
                }

                if (trackInfo.find(currentNote->tick()) != trackInfo.end()) {
                    deleteLastTiedChord = false;
                    bendDataCtx.chordTicksForTieBack[track].insert(currentNote->tick());
                    break;
                }

                tiedNotesAfterBend[track][mainTick].push_back(currentNote->chord());
                tieFor = currentNote->tieFor();
            }

            for (size_t i = 1; i < currentTickTiedChords.size(); i++) {
                if (i != currentTickTiedChords.size() - 1 || deleteLastTiedChord) {
                    bendDataCtx.reduntantChordTicks[track].insert(currentTickTiedChords[i]->tick());
                }
            }
        }
    }

    auto isPowerOfTwo = [](int n) {
        return n > 0 && (n & (n - 1)) == 0;
    };

    for (const auto& [track, trackInfo] : tiedNotesAfterBend) {
        const auto& importedInfoForTrack = m_bendInfoForNote.at(track);
        for (const auto& [mainTick, chords] : trackInfo) {
            const auto& importedInfoForTick = importedInfoForTrack.at(mainTick);
            const Chord* lastChord = chords.back();
            Fraction totalDuration = lastChord->tick() - mainTick + lastChord->actualTicks();
            if (Tuplet* tuplet = lastChord->tuplet()) {
                totalDuration *= tuplet->ratio();
            }

            totalDuration.reduce();
            IF_ASSERT_FAILED(!totalDuration.negative() && isPowerOfTwo(totalDuration.denominator())) {
                LOGE() << "bend import error: duration is wrong for track " << track << ", tick " << mainTick << "(" <<
                    totalDuration.numerator() << " / " << totalDuration.denominator() << ")";
                continue;
            }

            if (!importedInfoForTick.empty()) {
                fillChordDurationsFromBendDiagram(bendDataCtx, totalDuration, importedInfoForTick.begin()->second);
            }
        }
    }

    for (const auto& [track, trackInfo] : bendDataCtx.bendChordDurations) {
        if (bendDataCtx.reduntantChordTicks.find(track) == bendDataCtx.reduntantChordTicks.end()) {
            continue;
        }

        for (const auto& [mainTick, durations] : trackInfo) {
            Fraction currentTick = Fraction::fromTicks(mainTick);
            bendDataCtx.reduntantChordTicks.at(track).erase(currentTick);

            for (const Fraction& duration : durations) {
                currentTick += duration;
                bendDataCtx.reduntantChordTicks.at(track).erase(currentTick);
            }
        }
    }
}

static int getMaxDenominatorForSplit(const Fraction& duration)
{
    int denominator = 1;

    while (Fraction(1, denominator * 2) > duration) {
        denominator *= 2;
    }

    return denominator * 2;
}

static std::vector<Fraction> splittedDurations(const ImportedBendInfo& importedInfo, Fraction totalDuration)
{
    const auto& bendSegments = importedInfo.segments;

    std::vector<Fraction> proportions;

    const int maxDenominator = getMaxDenominatorForSplit(totalDuration / 4);

    for (size_t i = 0; i < bendSegments.size(); i++) {
        const auto& seg = bendSegments[i];
        if (seg.endTime != seg.startTime) {
            Fraction targetProportion(seg.endTime - seg.startTime,  PitchValue::MAX_TIME);
            targetProportion.reduce();
            proportions.push_back(targetProportion);
        }
    }

    Fraction proportionsSum = std::accumulate(proportions.begin(), proportions.end(), Fraction(0, 1));

    if (proportionsSum < Fraction(1, 1)) {
        Fraction lastProportion = Fraction(1, 1) - proportionsSum;
        proportions.push_back(lastProportion);
    }

    return BendChordDurationSplitter::findValidNoteSplit(totalDuration, proportions, maxDenominator);
}

static void splitBendChordDurations(BendDataContextSplitChord& bendDataCtx, Fraction totalDuration,
                                    const ImportedBendInfo& importedInfo)
{
    const Chord* chord = importedInfo.note->chord();
    bendDataCtx.bendChordDurations[chord->track()][chord->tick().ticks()] = splittedDurations(importedInfo, totalDuration);
}

static void fillChordDurationsFromBendDiagram(BendDataContextSplitChord& bendDataCtx, Fraction totalDuration,
                                              const ImportedBendInfo& importedInfo)
{
    if (importedInfo.segments.empty()) {
        return;
    }

    const Note* note = importedInfo.note;
    IF_ASSERT_FAILED(note) {
        LOGE() << "couldn't fill chord durations: note is NULL";
        return;
    }

    splitBendChordDurations(bendDataCtx, totalDuration, importedInfo);
}
} // namespace mu::iex::guitarpro
