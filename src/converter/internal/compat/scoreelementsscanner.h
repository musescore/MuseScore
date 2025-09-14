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

#include "engraving/types/types.h"

namespace mu::engraving {
class Score;
}

namespace mu::converter {
class ScoreElementScanner
{
public:
    struct ElementInfo
    {
        muse::String name;
        muse::String notes;
        mu::engraving::staff_idx_t staffIdx = 0;
        mu::engraving::voice_idx_t voiceIdx = 0;
        size_t measureIdx = 0;
        float beat = 0.f;
    };

    struct Options {
        Options() {}

        mu::engraving::ElementTypeSet acceptedTypes;
        bool avoidDuplicates = false;
    };

    // Instrument -> ElementType -> Elements
    using ElementInfoList = std::vector<ElementInfo>;
    using ElementMap = std::map<mu::engraving::ElementType, ElementInfoList>;
    using InstrumentElementMap = std::map<mu::engraving::InstrumentTrackId, ElementMap>;

    static InstrumentElementMap scanElements(mu::engraving::Score* score, const Options& options = {});
};
}
