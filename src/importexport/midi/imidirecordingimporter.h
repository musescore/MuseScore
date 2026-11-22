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

#include <vector>

#include "modularity/imoduleinterface.h"
#include "types/ret.h"
#include "types/retval.h"

#include "engraving/types/fraction.h"
#include "engraving/types/types.h"

namespace mu::engraving {
class Score;
}

namespace mu::iex::midi {
//! A note played during a live MIDI recording.
//! Times are in score ticks (Constants::DIVISION per quarter note),
//! relative to the start of the recording (the first beat after the count-in).
struct RecordedMidiNote {
    int pitch = 0;
    int velocity = 80;
    int onTick = 0;
    int offTick = 0;
};

using RecordedMidiNoteList = std::vector<RecordedMidiNote>;

struct MidiRecordingOptions {
    //! Shortest duration written, in ticks (480 = quarter note, 120 = 16th note)
    int minQuantizationTicks = 120;
    bool searchTuplets = false;
    //! When the gap between two notes is shorter than minQuantizationTicks, the first one is lengthened up to the next one;
    //! the import engine also lengthens notes to minimize the number of rests ("simplify durations").
    //! When disabled, the durations are written as played (on the quantization grid)
    bool legato = true;
    //! The content of the staff is replaced at least over this length from the start (e.g. the previous take of the same recording)
    int minReplacedTicks = 0;
};

class IMidiRecordingImporter : MODULE_GLOBAL_INTERFACE
{
    INTERFACE_ID(IMidiRecordingImporter)

public:
    virtual ~IMidiRecordingImporter() = default;

    //! Converts the recorded notes with the MIDI import engine (quantization, ties, voices)
    //! and writes them into the given staff, starting at startTick (must be the start of a measure).
    //! The existing notes and rests of that staff are replaced over the recorded measures.
    //! The caller is responsible for opening and closing the undoable command.
    //! Returns the length of the replaced range, in ticks from startTick
    virtual muse::RetVal<int> writeRecordedNotes(engraving::Score* score, engraving::staff_idx_t staffIdx,
                                                 const engraving::Fraction& startTick, const RecordedMidiNoteList& notes,
                                                 const MidiRecordingOptions& options) = 0;

    //! The same, split on two staves of the same instrument (e.g. the right and left hand of a piano):
    //! the notes equal to or higher than splitPitch go to the upper staff, the lower notes to the lower staff.
    //! Both staves are replaced over the same measures. Returns the length of the replaced range
    virtual muse::RetVal<int> writeRecordedNotesOnTwoStaves(engraving::Score* score, engraving::staff_idx_t upperStaffIdx,
                                                            engraving::staff_idx_t lowerStaffIdx, const engraving::Fraction& startTick,
                                                            const RecordedMidiNoteList& notes, int splitPitch,
                                                            const MidiRecordingOptions& options) = 0;
};
}
