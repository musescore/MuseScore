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

#include <array>
#include <cstddef>
#include <vector>

#include "midi/miditypes.h"
#include "mpe/events.h"

namespace mu::engraving {
class PlaybackNoteIndex
{
public:
    void clear();
    void add(const muse::mpe::PlaybackEventsMap& events);
    void finalize();

    std::vector<muse::midi::note_idx_t> activePitches(muse::mpe::timestamp_t timestamp) const;

private:
    struct Interval
    {
        muse::mpe::timestamp_t start = 0;
        muse::mpe::timestamp_t end = 0;
        bool openEnded = false;
    };

    static constexpr size_t PITCH_COUNT = 128;
    std::array<std::vector<Interval>, PITCH_COUNT> m_intervals;
};
}
