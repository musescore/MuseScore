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
#pragma once

#include "global/types/string.h"
#include "global/types/val.h"
#include "global/realfn.h"

#include "engraving/types/types.h"

namespace mu::converter {
struct ElementInfo
{
    mu::engraving::ElementType type = mu::engraving::ElementType::INVALID;
    muse::String name;
    std::map<muse::String, muse::Val> data;

    struct Note {
        muse::String name;
        std::map<muse::String, muse::Val> data;
    };

    using NoteList = std::vector<Note>;
    NoteList notes;

    struct Duration {
        muse::String name;
        uint8_t dots = 0;
    } duration;

    struct Location {
        mu::engraving::staff_idx_t staffIdx = muse::nidx;
        mu::engraving::voice_idx_t voiceIdx = muse::nidx;
        size_t measureIdx = muse::nidx;
        float beat = -1.f;

        bool operator==(const Location& l) const
        {
            return staffIdx == l.staffIdx
                   && voiceIdx == l.voiceIdx
                   && measureIdx == l.measureIdx
                   && muse::RealIsEqual(beat, l.beat);
        }
    } start, end;
};

// Instrument -> Elements
using ElementInfoList = std::vector<ElementInfo>;
using ElementMap = std::map<mu::engraving::InstrumentTrackId, ElementInfoList>;
}
