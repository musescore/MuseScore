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

#pragma once

#include <engraving/types/pitchvalue.h>
#include <engraving/types/types.h>
#include <engraving/dom/types.h>

#include "benddatacontext.h"

namespace mu::engraving {
class Note;
class Chord;
}

namespace mu::iex::guitarpro {
class BendDataCollector
{
public:

    void storeBendData(const mu::engraving::Note* note, const mu::engraving::PitchValues& pitchValues);
    BendDataContext collectBendDataContext();

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
        TIED_TO_PREVIOUS_NOTE,
        SLIGHT_BEND
    };

    struct ImportedBendInfo {
        const mu::engraving::Note* note = nullptr;
        std::vector<BendSegment> segments;
        BendType type = BendType::NORMAL_BEND;
        int timeOffsetFromStart = 0;
        int pitchOffsetFromStart = 0;
        bool connectsToNextNote = false;

        bool isSlightBend() const;
        bool startsWithPrebend() const;
    };

#ifndef SPLIT_CHORD_DURATIONS
    struct ChordImportedBendData {
        mu::engraving::Chord* chord = nullptr;
        std::map<mu::engraving::Note*, ImportedBendInfo> dataByNote;
    };

    struct TiedChordsBendDataChunk {
        std::vector<ChordImportedBendData> chordsData;
        bool shouldConnectToNext = false;
    };
#endif

private:
    std::unordered_map<mu::engraving::track_idx_t,
                       std::map<mu::engraving::Fraction,
                                std::unordered_map<const mu::engraving::Note*, ImportedBendInfo> > > m_bendInfoForNote;

#ifndef SPLIT_CHORD_DURATIONS
    std::unordered_map<mu::engraving::track_idx_t,
                       std::map<mu::engraving::Fraction, BendDataCollector::TiedChordsBendDataChunk> > m_regroupedDataByTiedChords;

    // converts m_bendInfoForNote to m_regroupedData, leaves m_bendInfoForNote empty
    // m_bendInfoForNote is needed to store separate bend data for each note, while m_regroupedData stores in format, comfortoble for import
    void regroupBendDataByTiedChords();
    void fillBendDataContext(BendDataContext& bendDataCtx);
#else
    void fillBendsDurations(BendDataContext& bendDataCtx);
    void fillBendData(BendDataContext& bendDataCtx);
#endif
};
} // mu::iex::guitarpro
