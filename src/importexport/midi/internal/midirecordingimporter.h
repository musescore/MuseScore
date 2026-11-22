/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "../imidirecordingimporter.h"

namespace mu::iex::midi {
class MidiFile;

class MidiRecordingImporter : public IMidiRecordingImporter
{
public:
    muse::RetVal<int> writeRecordedNotes(engraving::Score* score, engraving::staff_idx_t staffIdx, const engraving::Fraction& startTick,
                                         const RecordedMidiNoteList& notes, const MidiRecordingOptions& options) override;

    muse::RetVal<int> writeRecordedNotesOnTwoStaves(engraving::Score* score, engraving::staff_idx_t upperStaffIdx,
                                                    engraving::staff_idx_t lowerStaffIdx, const engraving::Fraction& startTick,
                                                    const RecordedMidiNoteList& notes, int splitPitch,
                                                    const MidiRecordingOptions& options) override;

    //! Exposed for tests: <upper staff (pitch >= splitPitch), lower staff>
    static std::pair<RecordedMidiNoteList, RecordedMidiNoteList> splitNotes(const RecordedMidiNoteList& notes, int splitPitch);

    //! Exposed for tests: builds the single-track MIDI file passed to the import engine
    static MidiFile makeMidiFile(const engraving::Score* score, engraving::staff_idx_t staffIdx, const engraving::Fraction& startTick,
                                 const RecordedMidiNoteList& notes);

    //! Exposed for tests: clamps the notes to the available measures and drops the invalid ones
    static RecordedMidiNoteList sanitizeNotes(const RecordedMidiNoteList& notes, int maxTicks);

    //! Exposed for tests: when the silence between the end of a note and the start of the next one
    //! is shorter than maxGapTicks, the note is lengthened up to the next one (legato), like a human would read it.
    //! The notes must be sorted by start time
    static RecordedMidiNoteList applyLegato(const RecordedMidiNoteList& notes, int maxGapTicks);

    //! Exposed for tests: makes gridTicks the shortest duration written.
    //! The quantization of the MIDI import engine is a maximum grid (it is refined for shorter notes), so:
    //! - without tuplets, the start and end of the notes are moved onto the grid, and last at least one grid step;
    //! - with tuplets, the start times are kept (for the tuplet detection) and the notes shorter than the grid
    //!   are only lengthened, without exceeding the start of the next note.
    //! The notes must be sorted by start time; the result is too
    static RecordedMidiNoteList applyMinimumQuantization(const RecordedMidiNoteList& notes, int gridTicks, bool keepStartTimes);
};
}
