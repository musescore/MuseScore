/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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
#include "benddatacollector.h"

#include <engraving/dom/chord.h>
#include <engraving/dom/note.h>
#include <engraving/dom/score.h>
#include <engraving/dom/tie.h>
#include <engraving/dom/tuplet.h>

#ifdef SPLIT_CHORD_DURATIONS
#include "bendchorddurationsplitter.h"
#endif

using namespace mu::engraving;

namespace mu::iex::guitarpro {
#ifdef SPLIT_CHORD_DURATIONS
static void fillChordDurationsFromBendDiagram(BendDataContext& bendDataCtx, Fraction totalDuration,
                                              const BendDataCollector::ImportedBendInfo& importedInfo);
#endif

static void fillBendDataForNote(BendDataContext& bendDataCtx, const BendDataCollector::ImportedBendInfo& importedInfo,
                                int noteIndexInChord);

static BendDataCollector::ImportedBendInfo fillBendInfo(const Note* note, const PitchValues& pitchValues);
static void fillSlightBendData(BendDataContext& bendDataCtx, const BendDataCollector::ImportedBendInfo& importedInfo, int noteIndexInChord);
static void fillPrebendData(BendDataContext& bendDataCtx, const BendDataCollector::ImportedBendInfo& importedInfo, int noteIndexInChord);
static void fillNormalBendData(BendDataContext& bendDataCtx, const BendDataCollector::ImportedBendInfo& importedInfo, int noteIndexInChord);

bool BendDataCollector::ImportedBendInfo::isSlightBend() const
{
    return type == BendType::SLIGHT_BEND;
}

bool BendDataCollector::ImportedBendInfo::startsWithPrebend() const
{
    return type == BendType::PREBEND;
}

void BendDataCollector::storeBendData(const mu::engraving::Note* note, const mu::engraving::PitchValues& pitchValues)
{
    if (!pitchValues.empty()) {
        m_bendInfoForNote[note->track()][note->tick()][note] = fillBendInfo(note, pitchValues);
    }
}

BendDataContext BendDataCollector::collectBendDataContext()
{
    BendDataContext bendDataCtx;

#ifdef SPLIT_CHORD_DURATIONS
    fillBendsDurations(bendDataCtx);
    fillBendData(bendDataCtx);
#else
    regroupBendDataByTiedChords();
    fillBendDataContext(bendDataCtx);
#endif

    return bendDataCtx;
}

BendDataCollector::ImportedBendInfo fillBendInfo(const Note* note, const PitchValues& pitchValues)
{
    using bdc = BendDataCollector;

    enum class PitchValuesDiff {
        NONE,
        SAME_PITCH_AND_TIME,
        SAME_PITCH_DIFF_TIME,
        INCREASED_PITCH_SAME_TIME,
        INCREASED_PITCH_DIFF_TIME,
        DECREASED_PITCH_DIFF_TIME
    };

    using pvd = PitchValuesDiff;

    bdc::ImportedBendInfo importedInfo;
    PitchValuesDiff currentPitchDiff;
    PitchValuesDiff previousPitchDiff = PitchValuesDiff::NONE;

    auto pitchDiff = [](const PitchValue& prev, const PitchValue& cur) {
        if (prev.pitch == cur.pitch) {
            return prev.time == cur.time ? pvd::SAME_PITCH_AND_TIME : pvd::SAME_PITCH_DIFF_TIME;
        }

        if (prev.time == cur.time) {
            return pvd::INCREASED_PITCH_SAME_TIME;
        }

        return prev.pitch < cur.pitch ? pvd::INCREASED_PITCH_DIFF_TIME : pvd::DECREASED_PITCH_DIFF_TIME;
    };

    auto addPrebendOrHold = [&importedInfo](const PitchValue& start, bool noteTiedBack) {
        importedInfo.timeOffsetFromStart = start.time;
        importedInfo.pitchOffsetFromStart = start.pitch;
        if (start.pitch > 0) {
            importedInfo.type = (noteTiedBack ? bdc::BendType::TIED_TO_PREVIOUS_NOTE : bdc::BendType::PREBEND);
        }
    };

    for (size_t i = 0; i < pitchValues.size() - 1; i++) {
        currentPitchDiff = pitchDiff(pitchValues[i], pitchValues[i + 1]);
        if (currentPitchDiff == pvd::SAME_PITCH_AND_TIME) {
            continue;
        }

        if (currentPitchDiff == previousPitchDiff && !importedInfo.segments.empty()) {
            bdc::BendSegment& lastSegment = importedInfo.segments.back();
            lastSegment.middleTime = pitchValues[i].time;
            lastSegment.endTime = pitchValues[i + 1].time;
            lastSegment.endPitch = pitchValues[i + 1].pitch;
            continue;
        }

        switch (currentPitchDiff) {
        case pvd::NONE:
        case pvd::SAME_PITCH_AND_TIME:
            break;

        case pvd::SAME_PITCH_DIFF_TIME:
        {
            if (importedInfo.segments.empty()) {
                addPrebendOrHold(pitchValues[i], note->tieBack());
            } else {
                bdc::BendSegment& lastSegment = importedInfo.segments.back();
                lastSegment.middleTime = lastSegment.endTime;
                lastSegment.endTime = pitchValues[i + 1].time;
            }

            break;
        }

        case pvd::INCREASED_PITCH_SAME_TIME:
        {
            if (importedInfo.segments.empty()) {
                addPrebendOrHold(pitchValues[i], note->tieBack());
            }

            break;
        }

        case pvd::INCREASED_PITCH_DIFF_TIME:
        case pvd::DECREASED_PITCH_DIFF_TIME:
        {
            if (importedInfo.segments.empty()) {
                addPrebendOrHold(pitchValues[i], note->tieBack());
            }

            bdc::BendSegment newSegment;
            newSegment.startPitch = pitchValues[i].pitch;
            newSegment.endPitch = pitchValues[i + 1].pitch;
            newSegment.startTime = pitchValues[i].time;
            newSegment.middleTime = pitchValues[i + 1].time;
            newSegment.endTime = pitchValues[i + 1].time;
            importedInfo.segments.push_back(newSegment);
            break;
        }
        }

        previousPitchDiff = currentPitchDiff;
    }

    importedInfo.note = note;
    if (!importedInfo.segments.empty()) {
        importedInfo.segments.back().endTime = PitchValue::MAX_TIME;
    }

    for (auto& seg : importedInfo.segments) {
        if (seg.endTime - seg.middleTime == 1) {
            seg.middleTime = seg.endTime;
        }
    }

    if (importedInfo.segments.size() == 1 && importedInfo.type == bdc::BendType::NORMAL_BEND
        && importedInfo.segments.front().pitchDiff() == 25) {
        importedInfo.type = bdc::BendType::SLIGHT_BEND;
    }

    return importedInfo;
}

static void fillBendDataForNote(BendDataContext& bendDataCtx, const BendDataCollector::ImportedBendInfo& importedInfo, int noteIndexInChord)
{
    const Note* note = importedInfo.note;
    if (!note) {
        return;
    }

    if (importedInfo.isSlightBend()) {
        fillSlightBendData(bendDataCtx, importedInfo, noteIndexInChord);
        return;
    }

    if (importedInfo.startsWithPrebend()) {
        fillPrebendData(bendDataCtx, importedInfo, noteIndexInChord);
    }

    fillNormalBendData(bendDataCtx, importedInfo, noteIndexInChord);
}

void fillSlightBendData(BendDataContext& bendDataCtx, const BendDataCollector::ImportedBendInfo& importedInfo, int noteIndexInChord)
{
    const Note* note = importedInfo.note;
    const Chord* chord = note->chord();
    Fraction tick = chord->tick();

    BendDataContext::bend_data_map_t& slightBendChordData = bendDataCtx.slightBendData[note->track()][tick];
    BendDataContext::BendNoteData slightBendNoteData;
    slightBendNoteData.quarterTones = 1;

    const auto& seg = importedInfo.segments.front();

    const int distanceToBendStart = importedInfo.timeOffsetFromStart;
    const int bendLength = seg.middleTime - seg.startTime;
    const int currentSegmentLength = seg.endTime - seg.startTime + distanceToBendStart;

    slightBendNoteData.startFactor = (double)distanceToBendStart / currentSegmentLength;
    slightBendNoteData.endFactor = (double)(distanceToBendStart + bendLength) / currentSegmentLength;

    slightBendChordData[noteIndexInChord] = std::move(slightBendNoteData);
}

static void fillPrebendData(BendDataContext& bendDataCtx, const BendDataCollector::ImportedBendInfo& importedInfo, int noteIndexInChord)
{
    const Note* note = importedInfo.note;
    Fraction tick = note->tick();
    BendDataContext::bend_data_map_t& prebendChordData = bendDataCtx.prebendData[note->track()][tick];

    BendDataContext::BendNoteData prebendNoteData;
    prebendNoteData.quarterTones = importedInfo.pitchOffsetFromStart / 25;

    prebendChordData[noteIndexInChord] = std::move(prebendNoteData);
}

static void fillNormalBendData(BendDataContext& bendDataCtx, const BendDataCollector::ImportedBendInfo& importedInfo, int noteIndexInChord)
{
    if (importedInfo.segments.empty()) {
        return;
    }

    const Note* note = importedInfo.note;
    const Chord* chord = note->chord();
    track_idx_t track = chord->track();
    Fraction tick = chord->tick();

#ifndef SPLIT_CHORD_DURATIONS
    for (size_t i = 0; i < importedInfo.segments.size(); i++) {
        const auto& seg = importedInfo.segments[i];
        const int offsetFromStart = (i == 0) ? importedInfo.timeOffsetFromStart : 0;

        BendDataContext::BendNoteData data;
        data.quarterTones = seg.endPitch / 25;

        const int distanceToBendStart = offsetFromStart;
        const int bendLength = seg.middleTime - seg.startTime;
        const int currentSegmentLength = seg.endTime - seg.startTime + offsetFromStart;

        data.startFactor = (double)distanceToBendStart / currentSegmentLength;
        data.endFactor = (double)(distanceToBendStart + bendLength) / currentSegmentLength;
        size_t noteIdx = muse::indexOf(note->chord()->notes(), note);

        if (importedInfo.connectsToNextNote) {
            bendDataCtx.tiedNotesBendsData[track][tick][noteIdx] = std::move(data);
        } else {
            bendDataCtx.graceAfterBendData[track][tick][noteIdx].push_back(std::move(data));
        }
    }

#else
    const Fraction chordStartTick = chord->tick();
    const auto& durations = bendDataCtx.bendChordDurations;
    Fraction ratio = Fraction(1, 1);

    if (const Tuplet* tuplet = chord->tuplet()) {
        ratio = tuplet->ratio();
    }

    if (durations.find(chord->track()) == durations.end()) {
        LOGE() << "bend import error : no information about chord duration for track " << chord->track();
        return;
    }

    const auto& trackDurations = durations.at(chord->track());
    if (trackDurations.find(chordStartTick.ticks()) == trackDurations.end()) {
        LOGE() << "bend import error : no information about chord duration for track " << chord->track() << " tick " <<
            chordStartTick.ticks();
        return;
    }

    const auto& tickDurations = trackDurations.at(chordStartTick.ticks());

    if (tickDurations.empty()) {
        LOGE() << "bend import error : chord durations are empty for track " << chord->track() << " tick " <<
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

        BendDataContext::BendChordData& bendChordData
            = bendDataCtx.bendDataByEndTick[note->track()][(currentTick + tickDuration).ticks()];
        bendChordData.startTick = currentTick;
        BendDataContext::BendNoteData bendNoteData;
        bendNoteData.quarterTones = seg.endPitch / 25;
        bendChordData.noteDataByIdx[noteIndexInChord] = std::move(bendNoteData);

        currentTick += tickDuration;
    }
#endif
}

#ifndef SPLIT_CHORD_DURATIONS
void BendDataCollector::regroupBendDataByTiedChords()
{
    for (auto& [track, trackInfo] : m_bendInfoForNote) {
        for (auto& [mainTick, tickInfo] : trackInfo) {
            TiedChordsBendDataChunk chunk;
            const Note* currentNote = tickInfo.begin()->first;

            ChordImportedBendData chordBendData;
            chordBendData.chord = currentNote->chord();

            for (Note* note : currentNote->chord()->notes()) {
                chordBendData.dataByNote[note] = std::move(tickInfo[note]);
            }

            chunk.chordsData.push_back(std::move(chordBendData));

            const Tie* tieFor = currentNote->tieFor();
            while (tieFor) {
                currentNote = tieFor->endNote();

                IF_ASSERT_FAILED(currentNote) {
                    LOGE() << "invalid tie encountered while importing guitar bend";
                    break;
                }

                muse::contains(trackInfo, currentNote->tick());
                if (muse::contains(trackInfo, currentNote->tick()) && muse::contains(trackInfo.at(currentNote->tick()), currentNote)) {
                    break;
                }

                ChordImportedBendData chordBendData;
                chordBendData.chord = currentNote->chord();
                for (Note* note : currentNote->chord()->notes()) {
                    chordBendData.dataByNote[note] = std::move(tickInfo[note]);
                }

                chunk.chordsData.push_back(std::move(chordBendData));
                tieFor = currentNote->tieFor();
            }

            m_regroupedDataByTiedChords[track][mainTick] = std::move(chunk);
        }
    }

    m_bendInfoForNote.clear();
}

void BendDataCollector::fillBendDataContext(BendDataContext& bendDataCtx)
{
    for (auto& [track, trackInfo] : m_regroupedDataByTiedChords) {
        for (auto& [tick, chunk] : trackInfo) {
            size_t tiedBendNotesAmount = chunk.chordsData.size();
            if (tiedBendNotesAmount > 1) {
                auto& firstChordData = chunk.chordsData.front();
                const Chord* firstChord = firstChordData.chord;
                for (size_t noteIdx = 0; noteIdx < firstChord->notes().size(); noteIdx++) {
                    Note* note = firstChord->notes()[noteIdx];
                    if (muse::contains(firstChordData.dataByNote, note)) {
                        auto& dataForFirstNote = firstChordData.dataByNote.at(note);
                        dataForFirstNote.connectsToNextNote = true;
                        const size_t segmentsSize = dataForFirstNote.segments.size();
                        size_t newSegmentsSize = segmentsSize;
                        size_t diff = tiedBendNotesAmount - segmentsSize;
                        // if first chord in chunk has more segments than tied notes, move segments forward to other notes
                        if (diff > 0) {
                            for (size_t i = 1; (i <= diff) && (newSegmentsSize > 1); i++, newSegmentsSize--) {
                                IF_ASSERT_FAILED(i < chunk.chordsData.size()) {
                                    LOGE() << "bend import error: bends data for tied notes is wrong for track " << track << ", tick " <<
                                        tick.ticks();
                                    continue;
                                }

                                auto& nextChordData = chunk.chordsData[i];

                                IF_ASSERT_FAILED(noteIdx < nextChordData.chord->notes().size()) {
                                    LOGE() << "bend import error: mismatch in tied chords' notes amount for track " << track << ", tick " <<
                                        tick.ticks();
                                    continue;
                                }

                                Note* nextNote = nextChordData.chord->notes()[noteIdx];

                                IF_ASSERT_FAILED(muse::contains(nextChordData.dataByNote, nextNote)) {
                                    LOGE() << "bend import error: bends data for tied notes is wrong for track " << track << ", tick " <<
                                        tick.ticks();
                                    continue;
                                }

                                auto& dataForNextNote = nextChordData.dataByNote.at(nextNote);
                                dataForNextNote.segments.push_back(std::move(dataForFirstNote.segments[i]));
                                dataForNextNote.connectsToNextNote = true;
                                dataForNextNote.note = nextNote;
                            }

                            if (newSegmentsSize < segmentsSize) {
                                dataForFirstNote.segments.resize(newSegmentsSize);
                            }
                        } else {
                            // TODO: add grace bends between tied notes
                        }
                    }
                }
            }

            for (auto& [chord, dataByNote] : chunk.chordsData) {
                for (Note* note : chord->notes()) {
                    int idx = static_cast<int>(muse::indexOf(note->chord()->notes(), note));
                    if (muse::contains(dataByNote, note)) {
                        fillBendDataForNote(bendDataCtx, dataByNote.at(note), idx);
                    }
                }
            }
        }
    }
}

#else

void BendDataCollector::fillBendData(BendDataContext& bendDataCtx)
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

void BendDataCollector::fillBendsDurations(BendDataContext& bendDataCtx)
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

static std::vector<Fraction> splittedDurations(const BendDataCollector::ImportedBendInfo& importedInfo, Fraction totalDuration)
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

static void splitBendChordDurations(BendDataContext& bendDataCtx, Fraction totalDuration,
                                    const BendDataCollector::ImportedBendInfo& importedInfo)
{
    const Chord* chord = importedInfo.note->chord();
    bendDataCtx.bendChordDurations[chord->track()][chord->tick().ticks()] = splittedDurations(importedInfo, totalDuration);
}

static void fillChordDurationsFromBendDiagram(BendDataContext& bendDataCtx, Fraction totalDuration,
                                              const BendDataCollector::ImportedBendInfo& importedInfo)
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

#endif
} // namespace mu::iex::guitarpro
