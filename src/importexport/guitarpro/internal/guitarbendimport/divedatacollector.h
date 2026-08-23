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

#include "guitarbendimporttypes.h"

#include <engraving/types/pitchvalue.h>

namespace mu::engraving {
class Chord;
}

namespace mu::iex::guitarpro {
class DiveDataCollector
{
public:
    DiveDataCollector() = default;

    void collectDiveData(const mu::engraving::Chord* chord, const mu::engraving::PitchValues& pitchValues);
    DiveDataContext& context() { return m_ctx; }

private:
    struct PrevWhammy {
        int destPitch = 0;
        mu::engraving::Fraction endTick;
    };

    DiveDataContext m_ctx;
    std::unordered_map<mu::engraving::track_idx_t, PrevWhammy> m_lastWhammyByTrack;
};
} // namespace mu::iex::guitarpro
