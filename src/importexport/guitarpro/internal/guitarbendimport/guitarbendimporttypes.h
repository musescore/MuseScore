/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
#pragma once

#include <map>
#include <unordered_map>
#include <vector>

#include <engraving/types/fraction.h>
#include <engraving/types/types.h>

namespace mu::engraving {
class Note;
class Chord;
}

namespace mu::iex::guitarpro {
constexpr int GP_PITCH_PER_QUARTERTONE = 25;

// ---- shared types ----

struct SegmentData {
    double startFactor = 0.0;
    double endFactor = 1.0;
    int quarterTones = 0;
};

// tie between originally tied notes should be removed and instead the bend after last grace note should be placed
struct LastGraceNoteData {
    bool shouldMoveTie = false;
    double endFactor = 1.0; // grace notes' durations are split equally and they have startFactor 0 (bend starts from the beginning)
};

struct GraceNotesImportInfo {
    std::vector<SegmentData> data;
    LastGraceNoteData lastNoteData;
};

using segment_data_map_t = std::map<size_t, SegmentData>;
using grace_segment_map_t = std::map<size_t /* idx in chord */, GraceNotesImportInfo>; // each note can have multiple grace notes connected

using GraceAfterTrackMap = std::unordered_map<mu::engraving::track_idx_t,
                                              std::map<mu::engraving::Fraction, grace_segment_map_t> >;

// ---- bend types ----

struct BendChordData {
    mu::engraving::Fraction startTick;
    segment_data_map_t noteDataByIdx;
};

struct BendSegment {
    int startTime = -1;
    int middleTime = -1;
    int endTime = -1;
    int startPitch = -1;
    int endPitch = -1;

    int pitchDiff() const { return endPitch - startPitch; }
};

enum class BendType {
    NORMAL_BEND,
    PREBEND,
    HOLD,
    SLIGHT_BEND
};

enum class ConnectionToNextNoteType {
    NONE, // note doesn't connect to next tied note -> using "grace note after"
    MAIN_NOTE_CONNECTS, // new note isn't added, only adding bend between 2 originally tied notes
    LAST_GRACE_CONNECTS // grace notes are created and last of them connects to next tied note with bend
};

struct ImportedBendInfo {
    const mu::engraving::Note* note = nullptr;
    std::vector<BendSegment> segments;
    BendType type = BendType::NORMAL_BEND;
    int timeOffsetFromStart = 0;
    int pitchOffsetFromStart = 0;

    ConnectionToNextNoteType connectionType = ConnectionToNextNoteType::NONE;
};

struct ChordImportedBendData {
    const mu::engraving::Chord* chord = nullptr;
    std::unordered_map<const mu::engraving::Note*, ImportedBendInfo> dataByNote;
};

using tied_chords_bend_data_chunk_t = std::map<mu::engraving::Fraction, ChordImportedBendData>;

// ---- dive types ----

enum class DiveType {
    NONE,
    DIVE,
    PRE_DIVE,
};

struct ImportedDiveInfo {
    const mu::engraving::Note* note = nullptr;
    DiveType type = DiveType::NONE;
    int quarterTones = 0;
    double startFactor = 0.0;
    double endFactor = 1.0;
    int whammyOriginQt = 0;
    int whammyDestQt = 0;
    std::vector<SegmentData> graceDiveSegments;
};

// ---- data contexts ----

struct BendDataContextSplitChord;

struct BendDataContext {
    GraceAfterTrackMap graceAfterBendData;
    std::unordered_map<mu::engraving::track_idx_t, std::map<mu::engraving::Fraction, segment_data_map_t > > tiedNotesBendsData;
    std::unordered_map<mu::engraving::track_idx_t, std::map<mu::engraving::Fraction, segment_data_map_t > > prebendData;
    std::unordered_map<mu::engraving::track_idx_t, std::map<mu::engraving::Fraction, segment_data_map_t > > slightBendData;
    std::unique_ptr<BendDataContextSplitChord> splitChordCtx;
};

struct DiveDataContext {
    GraceAfterTrackMap graceAfterDiveData;
};
} // mu::iex::guitarpro
