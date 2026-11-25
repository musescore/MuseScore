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
#include <vector>

#include "mpe/mpetypes.h"

#include "engraving/types/bps.h"
#include "engraving/types/types.h"

namespace mu::engraving {
class EngravingItem;
class Note;

//! An item (or an automation lane edit) that determines a value playback uses for a note
struct NotePlaybackSource {
    const EngravingItem* item = nullptr; // nullptr: the point was drawn directly in the automation lane
    bool editedInAutomation = false;     // the item's point was changed by hand in the automation lane

    bool operator==(const NotePlaybackSource& src) const;
};

//! What playback uses for a note on one pass through the score (repeats can play a note several times)
struct NotePlaybackPass {
    int passNumber = 1;  // 1-based, in playback order
    int utick = 0;

    BeatsPerSecond tempoValue;       // as marked in the score / automation
    BeatsPerSecond multipliedTempo;  // after the playback speed setting
    bool isDefaultTempo = false;     // nothing in the score sets the tempo here
    std::vector<NotePlaybackSource> tempoSources;
    std::vector<NotePlaybackSource> tempoOrigins;  // what the accel./rit. among tempoSources start from

    double dynamicValue = 0.0;      // normalized [0, 1], as used by playback
    bool isDefaultDynamic = false;  // nothing in the score sets the dynamic here
    std::vector<NotePlaybackSource> dynamicSources;
    std::vector<NotePlaybackSource> dynamicOrigins;  // what the hairpins among dynamicSources start from
};

//! A score item that changes how playback performs the note (staccato, slur, pedal, trill, tie, fermata, ...)
struct NotePlaybackMarking {
    const EngravingItem* item = nullptr;

    // What playback does because of the item; empty for items that change timing
    // rather than articulation (ties, fermatas)
    std::vector<muse::mpe::ArticulationType> effects;
};

struct NotePlaybackInfo {
    const Note* note = nullptr;        // the master-score note analysed
    const Note* attackNote = nullptr;  // first note of the (played) tie chain; its tick decides the dynamic

    int userVelocity = 0;  // per-note velocity override; 0 = none

    std::vector<NotePlaybackPass> passes;

    std::vector<NotePlaybackMarking> markings;

    bool isValid() const;
};

//! Reads the score's automation curves and tempo timeline, i.e. the same data playback uses.
//! expandRepeats must match the "play repeats" setting of the playback model.
NotePlaybackInfo collectNotePlaybackInfo(const Note* note, bool expandRepeats);

//! Adds the items that PlaybackModel::articulationSources() found, one marking per item
void addArticulationMarkings(NotePlaybackInfo& info, const std::map<muse::mpe::ArticulationType, const EngravingItem*>& sources);

//! This is the ordinary dynamic (pppppp..ffffff) whose playback level is closest to normalizedValue
DynamicType nearestOrdinaryDynamic(double normalizedValue);
} // namespace mu::engraving
