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

#include "bendinfoconverter.h"

using namespace mu::engraving;

namespace mu::iex::guitarpro {
static void fillBendDataForNote(BendDataContext& bendDataCtx, const ImportedBendInfo& importedInfo, int noteIndexInChord);

static void fillSlightBendData(BendDataContext& bendDataCtx, const ImportedBendInfo& importedInfo, int noteIndexInChord);
static void fillPrebendData(BendDataContext& bendDataCtx, const ImportedBendInfo& importedInfo, int noteIndexInChord);
static void fillNormalBendData(BendDataContext& bendDataCtx, const ImportedBendInfo& importedInfo);

void BendDataCollector::storeBendData(const mu::engraving::Note* note, const mu::engraving::PitchValues& pitchValues)
{
#ifdef SPLIT_CHORD_DURATIONS
    {
        if (!m_bendDataCollectorSplitChord) {
            m_bendDataCollectorSplitChord = std::make_unique<BendDataCollectorSplitChord>();
        }

        m_bendDataCollectorSplitChord->storeBendData(note, pitchValues);
    }
#else
    if (!pitchValues.empty()) {
        m_bendInfoForNote[note->track()][note->tick()][note] = BendInfoConverter::fillBendInfo(note, pitchValues);
    }
#endif
}

BendDataContext BendDataCollector::collectBendDataContext()
{
    BendDataContext bendDataCtx;

#ifdef SPLIT_CHORD_DURATIONS
    bendDataCtx.splitChordCtx = std::make_unique<BendDataContextSplitChord>(m_bendDataCollectorSplitChord->collectBendDataContext());
#else
    regroupBendDataByTiedChords();
    fillBendDataContext(bendDataCtx);
#endif

    return bendDataCtx;
}

static void fillBendDataForNote(BendDataContext& bendDataCtx, const ImportedBendInfo& importedInfo, int noteIndexInChord)
{
    const Note* note = importedInfo.note;
    if (!note) {
        return;
    }

    if (importedInfo.type == BendType::SLIGHT_BEND) {
        fillSlightBendData(bendDataCtx, importedInfo, noteIndexInChord);
        return;
    }

    if (importedInfo.type == BendType::PREBEND) {
        fillPrebendData(bendDataCtx, importedInfo, noteIndexInChord);
    }

    fillNormalBendData(bendDataCtx, importedInfo);
}

void fillSlightBendData(BendDataContext& bendDataCtx, const ImportedBendInfo& importedInfo, int noteIndexInChord)
{
    const Note* note = importedInfo.note;
    const Chord* chord = note->chord();
    Fraction tick = chord->tick();

    bend_data_map_t& slightBendChordData = bendDataCtx.slightBendData[note->track()][tick];
    BendNoteData slightBendNoteData;
    slightBendNoteData.quarterTones = 1;

    const auto& seg = importedInfo.segments.front();

    const int distanceToBendStart = importedInfo.timeOffsetFromStart;
    const int bendLength = seg.middleTime - seg.startTime;
    const int currentSegmentLength = seg.endTime - seg.startTime + distanceToBendStart;

    slightBendNoteData.startFactor = (double)distanceToBendStart / currentSegmentLength;
    slightBendNoteData.endFactor = (double)(distanceToBendStart + bendLength) / currentSegmentLength;

    slightBendChordData[noteIndexInChord] = std::move(slightBendNoteData);
}

static void fillPrebendData(BendDataContext& bendDataCtx, const ImportedBendInfo& importedInfo, int noteIndexInChord)
{
    const Note* note = importedInfo.note;
    Fraction tick = note->tick();
    bend_data_map_t& prebendChordData = bendDataCtx.prebendData[note->track()][tick];

    BendNoteData prebendNoteData;
    prebendNoteData.quarterTones = importedInfo.pitchOffsetFromStart / 25;

    prebendChordData[noteIndexInChord] = std::move(prebendNoteData);
}

static void fillNormalBendData(BendDataContext& bendDataCtx, const ImportedBendInfo& importedInfo)
{
    if (importedInfo.segments.empty()) {
        return;
    }

    const Note* note = importedInfo.note;
    const Chord* chord = note->chord();
    track_idx_t track = chord->track();
    Fraction tick = chord->tick();

    for (size_t i = 0; i < importedInfo.segments.size(); i++) {
        const auto& seg = importedInfo.segments[i];
        const int offsetFromStart = (i == 0) ? importedInfo.timeOffsetFromStart : 0;

        BendNoteData data;
        data.quarterTones = seg.endPitch / 25;

        const int distanceToBendStart = offsetFromStart;
        const int bendLength = seg.middleTime - seg.startTime;
        const int currentSegmentLength = seg.endTime - seg.startTime + offsetFromStart;

        data.startFactor = (double)distanceToBendStart / currentSegmentLength;
        data.endFactor = (double)(distanceToBendStart + bendLength) / currentSegmentLength;
        size_t noteIdx = muse::indexOf(note->chord()->notes(), note);

        if (importedInfo.connectionType == ConnectionToNextNoteType::MAIN_NOTE_CONNECTS) {
            bendDataCtx.tiedNotesBendsData[track][tick][noteIdx] = std::move(data);
        } else if (importedInfo.connectionType == ConnectionToNextNoteType::LAST_GRACE_CONNECTS && i == importedInfo.segments.size() - 1) {
            bendDataCtx.graceAfterBendData[track][tick][noteIdx].shouldMoveTie = true;
        } else {
            bendDataCtx.graceAfterBendData[track][tick][noteIdx].data.push_back(std::move(data));
        }
    }
}

void BendDataCollector::regroupBendDataByTiedChords()
{
    for (auto& [track, trackInfo] : m_bendInfoForNote) {
        for (auto& [mainTick, tickInfo] : trackInfo) {
            tied_chords_bend_data_chunk_t chunk;
            const Note* currentNote = tickInfo.begin()->first;

            {
                ChordImportedBendData chordBendData;
                chordBendData.chord = currentNote->chord();

                for (Note* note : currentNote->chord()->notes()) {
                    chordBendData.dataByNote[note] = std::move(tickInfo[note]);
                }

                chunk.push_back(std::move(chordBendData));
            }

            const Tie* tieFor = currentNote->tieFor();
            while (tieFor) {
                currentNote = tieFor->endNote();

                IF_ASSERT_FAILED(currentNote) {
                    LOGE() << "invalid tie encountered while importing guitar bend";
                    break;
                }

                muse::contains(trackInfo, currentNote->tick());
                if (muse::contains(trackInfo, currentNote->tick()) && muse::contains(trackInfo.at(currentNote->tick()), currentNote)) {
                    auto& noteInfo = trackInfo.at(currentNote->tick()).at(currentNote);
                    if (noteInfo.type == BendType::PREBEND) {
                        noteInfo.type = BendType::HOLD;
                        // rewriting first note's info about should it connect to next or not
                        size_t noteIndex = muse::indexOf(currentNote->chord()->notes(), currentNote);
                        auto& firstChordData = chunk[0];
                        firstChordData.dataByNote[firstChordData.chord->notes()[noteIndex]].connectionType
                            = ConnectionToNextNoteType::MAIN_NOTE_CONNECTS;
                    } else {
                        break;
                    }
                }

                ChordImportedBendData chordBendData;
                chordBendData.chord = currentNote->chord();
                for (Note* note : currentNote->chord()->notes()) {
                    chordBendData.dataByNote[note] = std::move(tickInfo[note]);
                }

                chunk.push_back(std::move(chordBendData));
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
            size_t tiedBendNotesAmount = chunk.size();

            auto& firstChordData = chunk.front();
            const Chord* firstChord = firstChordData.chord;

            for (size_t noteIdx = 0; noteIdx < firstChord->notes().size(); noteIdx++) {
                const Note* mainNote = firstChord->notes()[noteIdx];
                if (muse::contains(firstChordData.dataByNote, mainNote)) {
                    auto& dataForFirstNote = firstChordData.dataByNote.at(mainNote);
                    if (tiedBendNotesAmount >= dataForFirstNote.segments.size() && tiedBendNotesAmount > 1) {
                        moveSegmentsToTiedNotes(chunk, dataForFirstNote, noteIdx);
                    } else if (mainNote->tieFor()) {
                        // last grace note should connect to next tied note instead of original note of chord
                        size_t noteIndex = muse::indexOf(mainNote->chord()->notes(), mainNote);
                        auto& lastChordData = chunk.back();
                        lastChordData.dataByNote[lastChordData.chord->notes()[noteIndex]].connectionType
                            = ConnectionToNextNoteType::LAST_GRACE_CONNECTS;
                    }
                }
            }

            for (auto& [chord, dataByNote] : chunk) {
                for (const Note* note : chord->notes()) {
                    int idx = static_cast<int>(muse::indexOf(note->chord()->notes(), note));
                    if (muse::contains(dataByNote, note)) {
                        fillBendDataForNote(bendDataCtx, dataByNote.at(note), idx);
                    }
                }
            }
        }
    }
}

void BendDataCollector::moveSegmentsToTiedNotes(tied_chords_bend_data_chunk_t& dataChunk, ImportedBendInfo& dataForFirstNote,
                                                size_t noteIdx)
{
    dataForFirstNote.connectionType = ConnectionToNextNoteType::MAIN_NOTE_CONNECTS;
    const size_t segmentsSize = dataForFirstNote.segments.size();
    size_t newSegmentsSize = segmentsSize;
    const Note* note = dataForFirstNote.note;
    size_t diff = dataChunk.size() - segmentsSize;

    IF_ASSERT_FAILED(dataChunk.size() >= segmentsSize) {
        return;
    }

    for (size_t i = 1; newSegmentsSize > 1; i++, newSegmentsSize--) {
        const bool chordExists = i < dataChunk.size();
        const bool noteExists = chordExists && noteIdx < dataChunk[i].chord->notes().size();
        const Note* nextNote = noteExists ? dataChunk[i].chord->notes()[noteIdx] : nullptr;
        const bool dataExists = noteExists && muse::contains(dataChunk[i].dataByNote, nextNote);

        if (!dataExists) {
            LOGE() << "bend import error: bends data for tied notes is wrong for track " << note->track() << ", tick " <<
                note->tick().ticks();
            return;
        }

        auto& nextChordData = dataChunk[i];
        auto& dataForNextNote = nextChordData.dataByNote.at(nextNote);
        dataForNextNote.segments.push_back(std::move(dataForFirstNote.segments[i]));
        dataForNextNote.connectionType = ConnectionToNextNoteType::MAIN_NOTE_CONNECTS;
        dataForNextNote.note = nextNote;
    }

    if (newSegmentsSize < segmentsSize) {
        dataForFirstNote.segments.resize(newSegmentsSize);
    }

    // in that case last note should be grace-after
    if (diff == 0) {
        auto& nextChordData = dataChunk.back();
        for (const Note* lastChordNote : nextChordData.chord->notes()) {
            if (muse::contains(nextChordData.dataByNote, lastChordNote)) {
                nextChordData.dataByNote[lastChordNote].connectionType = ConnectionToNextNoteType::NONE;
            }
        }
    }
}
} // namespace mu::iex::guitarpro
