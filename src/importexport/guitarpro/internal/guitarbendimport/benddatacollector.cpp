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

#include "bendchorddurationsplitter.h"

using namespace mu::engraving;

namespace mu::iex::guitarpro {
constexpr int BEND_DIVISIONS = 60;

static void fillChordDurationsFromBendDiagram(BendDataContext& bendDataCtx, const BendDataCollector::ImportedBendInfo& importedInfo);
static void fillBendDataForNote(BendDataContext& bendDataCtx, const BendDataCollector::ImportedBendInfo& importedInfo,
                                int noteIndexInChord);
static std::vector<BendDataCollector::BendSegment> bendSegmentsFromPitchValues(const PitchValues& pitchValues, bool noteTiedBack);
static bool isSlightBend(const BendDataCollector::ImportedBendInfo& importedInfo);
static BendDataCollector::ImportedBendInfo fillBendInfo(const Note* note, const PitchValues& pitchValues);

void BendDataCollector::storeBendData(const mu::engraving::Note* note, const mu::engraving::PitchValues& pitchValues)
{
    if (!pitchValues.empty()) {
        m_bendInfoForNote[note->track()][note->tick().ticks()][note] = fillBendInfo(note, pitchValues);
    }
}

BendDataContext BendDataCollector::collectBendDataContext()
{
    BendDataContext bendDataCtx;

    for (const auto& [track, trackInfo] : m_bendInfoForNote) {
        for (const auto& [tick, tickInfo] : trackInfo) {
            if (!tickInfo.empty()) {
                // TODO: now using the top note of chord to fill durations
                fillChordDurationsFromBendDiagram(bendDataCtx, tickInfo.begin()->second);
            }
        }
    }

    for (const auto& [track, trackInfo] : m_bendInfoForNote) {
        for (const auto& [tick, tickInfo] : trackInfo) {
            for (const auto& [note, importedBendInfo] : tickInfo) {
                int idx = muse::indexOf(note->chord()->notes(), note);
                fillBendDataForNote(bendDataCtx, importedBendInfo, idx);
            }
        }
    }

    return bendDataCtx;
}

std::vector<BendDataCollector::BendSegment> bendSegmentsFromPitchValues(const PitchValues& pitchValues,
                                                                        bool noteTiedBack)
{
    enum PitchDiff {
        NONE,
        SAME,
        UP,
        DOWN
    };

    std::vector<BendDataCollector::BendSegment> bendSegments;
    PitchDiff currentPitchDiff;
    PitchDiff previousPitchDiff = PitchDiff::NONE;

    auto pitchDiff = [](int prevPitch, int currentPitch) {
        if (prevPitch == currentPitch) {
            return PitchDiff::SAME;
        }

        return (prevPitch < currentPitch) ? PitchDiff::UP : PitchDiff::DOWN;
    };

    if (pitchValues.front().pitch != 0 && !noteTiedBack) {
        BendDataCollector::BendSegment seg;
        seg.startTime = seg.endTime = 0;
        seg.startPitch = 0;
        seg.endPitch = pitchValues.front().pitch;

        bendSegments.push_back(seg);
    }

    for (size_t i = 0; i < pitchValues.size() - 1; i++) {
        currentPitchDiff = pitchDiff(pitchValues[i].pitch, pitchValues[i + 1].pitch);
        if (currentPitchDiff == previousPitchDiff) {
            if (!bendSegments.empty()) {
                BendDataCollector::BendSegment& lastSeg = bendSegments.back();
                lastSeg.endTime = pitchValues[i + 1].time;
                lastSeg.startPitch = pitchValues[i].pitch;
                lastSeg.endPitch = pitchValues[i + 1].pitch;
            }

            continue;
        }

        if (currentPitchDiff == PitchDiff::SAME) {
            BendDataCollector::BendSegment seg;
            seg.startTime = pitchValues[i].time;
            seg.endTime = pitchValues[i + 1].time;
            seg.startPitch = pitchValues[i].pitch;
            seg.endPitch = pitchValues[i + 1].pitch;
            bendSegments.push_back(seg);
        } else {
            if (previousPitchDiff != PitchDiff::SAME || bendSegments.empty()) {
                BendDataCollector::BendSegment seg;
                seg.startTime = pitchValues[i].time;
                seg.endTime = pitchValues[i + 1].time;
                bendSegments.push_back(seg);
            } else {
                BendDataCollector::BendSegment& lastSeg = bendSegments.back();
                lastSeg.middleTime = pitchValues[i].time;
                lastSeg.endTime = pitchValues[i + 1].time;
            }

            bendSegments.back().startPitch = pitchValues[i].pitch;
            bendSegments.back().endPitch = pitchValues[i + 1].pitch;
        }

        previousPitchDiff = currentPitchDiff;
    }

    return bendSegments;
}

BendDataCollector::ImportedBendInfo fillBendInfo(const Note* note, const PitchValues& pitchValues)
{
    BendDataCollector::ImportedBendInfo info;
    info.segments = bendSegmentsFromPitchValues(pitchValues, note->tieBack());
    info.note = note;

    for (const auto& bs : info.segments) {
        if (bs.pitchDiff() != 0 && bs.startTime != bs.endTime) {
            info.pitchChangesAmount++;
        }
    }

    return info;
}

static bool isSlightBend(const BendDataCollector::ImportedBendInfo& importedInfo)
{
    if (importedInfo.pitchChangesAmount != 1 || importedInfo.note->tieFor()) {
        return false;
    }

    for (const auto& seg : importedInfo.segments) {
        if (seg.pitchDiff() == 25) {
            return true;
        }
    }

    return false;
}

void fillSlightBendData(BendDataContext& bendDataCtx, const BendDataCollector::ImportedBendInfo& importedInfo, int noteIndexInChord)
{
    const Note* note = importedInfo.note;
    const Chord* chord = note->chord();
    Fraction tick = chord->tick();

    std::vector<Fraction> bendDurations;
    Fraction duration = chord->actualTicks();
    bendDurations.push_back(duration);

    bendDataCtx.bendChordDurations[note->track()][tick.ticks()] = std::move(bendDurations);

    BendDataContext::BendChordData& slightBendChordData = bendDataCtx.bendDataByEndTick[note->track()][tick.ticks()];
    slightBendChordData.startTick = tick;

    BendDataContext::BendNoteData slightBendNoteData;
    slightBendNoteData.quarterTones = 1;

    const auto& firstSeg = importedInfo.segments.front();
    if (firstSeg.middleTime != -1) {
        slightBendNoteData.startFactor = (double)firstSeg.middleTime / BEND_DIVISIONS;
    }

    slightBendNoteData.endFactor = (double)(firstSeg.endTime + 1) / BEND_DIVISIONS;
    slightBendNoteData.type = GuitarBendType::SLIGHT_BEND;

    slightBendChordData.noteDataByIdx[noteIndexInChord] = std::move(slightBendNoteData);
}

static bool isFirstPrebend(const BendDataCollector::ImportedBendInfo& importedInfo)
{
    const auto& firstSeg = importedInfo.segments.front();
    return firstSeg.startTime == firstSeg.endTime;
}

static void fillPrebendData(BendDataContext& bendDataCtx, const BendDataCollector::ImportedBendInfo& importedInfo, int noteIndexInChord)
{
    const Note* note = importedInfo.note;
    Fraction tick = note->tick();
    BendDataContext::BendChordData& prebendChordData = bendDataCtx.bendDataByEndTick[note->track()][tick.ticks()];
    prebendChordData.startTick = tick;

    const auto& firstSeg = importedInfo.segments.front();

    BendDataContext::BendNoteData prebendNoteData;
    prebendNoteData.type = GuitarBendType::PRE_BEND;
    prebendNoteData.quarterTones = firstSeg.endPitch / 25;

    prebendChordData.noteDataByIdx[noteIndexInChord] = std::move(prebendNoteData);
}

static void fillNormalBendData(BendDataContext& bendDataCtx, const BendDataCollector::ImportedBendInfo& importedInfo,
                               size_t startIndex, int noteIndexInChord)
{
    if (startIndex >= importedInfo.segments.size()) {
        return;
    }

    const Note* note = importedInfo.note;
    const Chord* chord = note->chord();
    const Fraction chordStartTick = chord->tick();
    const auto& durations = bendDataCtx.bendChordDurations;

    size_t currentIndex = startIndex;
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

    Fraction currentTick = chordStartTick;

    for (const auto& tickDuration : tickDurations) {
        if (currentIndex >= importedInfo.segments.size()) {
            break;
        }

        const auto& seg = importedInfo.segments[currentIndex];

        BendDataContext::BendChordData& bendChordData = bendDataCtx.bendDataByEndTick[note->track()][(currentTick + tickDuration).ticks()];
        bendChordData.startTick = currentTick;
        BendDataContext::BendNoteData bendNoteData;
        bendNoteData.type = GuitarBendType::BEND;
        bendNoteData.quarterTones = seg.endPitch / 25;
        bendChordData.noteDataByIdx[noteIndexInChord] = std::move(bendNoteData);

        currentTick += tickDuration;
        currentIndex++;
    }
}

static void addFullChordDuration(BendDataContext& bendDataCtx, const BendDataCollector::ImportedBendInfo& importedInfo)
{
    const Chord* chord = importedInfo.note->chord();
    Fraction tick = chord->tick();

    std::vector<Fraction> bendDurations;
    Fraction duration = chord->actualTicks();
    bendDurations.push_back(duration);
    bendDataCtx.bendChordDurations[chord->track()][tick.ticks()] = std::move(bendDurations);
}

static int getMaxDenominatorForSplit(const Fraction& duration)
{
    int denominator = 1;

    while (Fraction(1, denominator * 2) > duration) {
        denominator *= 2;
    }

    return denominator * 2;
}

static std::vector<Fraction> splittedDurations(const BendDataCollector::ImportedBendInfo& importedInfo, size_t startIndex)
{
    const Note* note = importedInfo.note;
    const Chord* chord = note->chord();
    const auto& bendSegments = importedInfo.segments;

    std::vector<Fraction> proportions;

    // TODO: Fraction duration = chord->endTickIncludingTied() - chord->tick();
    Fraction duration = chord->actualTicks();
    const int maxDenominator = getMaxDenominatorForSplit(duration / 4);

    for (size_t i = startIndex; i < bendSegments.size(); i++) {
        const auto& seg = bendSegments[i];
        if (seg.endTime != seg.startTime) {
            Fraction targetProportion(seg.endTime - seg.startTime, BEND_DIVISIONS);
            targetProportion.reduce();
            proportions.push_back(targetProportion);
        }
    }

    Fraction proportionsSum = std::accumulate(proportions.begin(), proportions.end(), Fraction(0, 1));

    if (proportionsSum < Fraction(1, 1)) {
        Fraction lastProportion = Fraction(1, 1) - proportionsSum;
        proportions.push_back(lastProportion);
    }

    return BendChordDurationSplitter::findValidNoteSplit(chord->actualTicks(), proportions, maxDenominator);
}

static void splitBendChordDurations(BendDataContext& bendDataCtx, const BendDataCollector::ImportedBendInfo& importedInfo,
                                    size_t startIndex)
{
    if (startIndex >= importedInfo.segments.size()) {
        return;
    }

    const Chord* chord = importedInfo.note->chord();
    bendDataCtx.bendChordDurations[chord->track()][chord->tick().ticks()] = splittedDurations(importedInfo, startIndex);
}

static void fillChordDurationsFromBendDiagram(BendDataContext& bendDataCtx, const BendDataCollector::ImportedBendInfo& importedInfo)
{
    const Note* note = importedInfo.note;
    IF_ASSERT_FAILED(note) {
        LOGE() << "couldn't fill chord durations: note is NULL";
        return;
    }

    const auto& bendSegments = importedInfo.segments;
    IF_ASSERT_FAILED(!bendSegments.empty()) {
        LOGE() << "couldn't fill chord durations: bend segments are empty";
        return;
    }

    if (isSlightBend(importedInfo)) {
        addFullChordDuration(bendDataCtx, importedInfo);
        return;
    }

    size_t startIndex = 0;

    if (isFirstPrebend(importedInfo)) {
        addFullChordDuration(bendDataCtx, importedInfo);
        startIndex = 1;
        if (importedInfo.segments.size() > 1 && importedInfo.segments[1].pitchDiff() == 0) {
            startIndex = 2;
        }
    }

    splitBendChordDurations(bendDataCtx, importedInfo, startIndex);
}

static void fillBendDataForNote(BendDataContext& bendDataCtx, const BendDataCollector::ImportedBendInfo& importedInfo, int noteIndexInChord)
{
    const Note* note = importedInfo.note;
    IF_ASSERT_FAILED(note) {
        LOGE() << "couldn't fill bend data: note is NULL";
        return;
    }

    const auto& bendSegments = importedInfo.segments;
    IF_ASSERT_FAILED(!bendSegments.empty()) {
        LOGE() << "couldn't fill bend data: bend segments are empty";
        return;
    }

    if (isSlightBend(importedInfo)) {
        fillSlightBendData(bendDataCtx, importedInfo, noteIndexInChord);
        return;
    }

    size_t startIndex = 0;

    if (isFirstPrebend(importedInfo)) {
        fillPrebendData(bendDataCtx, importedInfo, noteIndexInChord);
        startIndex = 1;
        if (importedInfo.segments.size() > 1 && importedInfo.segments[1].pitchDiff() == 0) {
            startIndex = 2;
        }
    }

    fillNormalBendData(bendDataCtx, importedInfo, startIndex, noteIndexInChord);
}
} // namespace mu::iex::guitarpro
