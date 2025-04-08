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

#include "mscore.h"
#include "chord.h"

namespace mu::engraving {
static constexpr size_t NUMBER_OF_SELECTION_FILTER_TYPES = 23;

enum class SelectionFilterType : unsigned int {
    NONE                    = 0,
    FIRST_VOICE             = 1 << 0,
    SECOND_VOICE            = 1 << 1,
    THIRD_VOICE             = 1 << 2,
    FOURTH_VOICE            = 1 << 3,
    DYNAMIC                 = 1 << 4,
    HAIRPIN                 = 1 << 5,
    FINGERING               = 1 << 6,
    LYRICS                  = 1 << 7,
    CHORD_SYMBOL            = 1 << 8,
    OTHER_TEXT              = 1 << 9,
    ARTICULATION            = 1 << 10,
    ORNAMENT                = 1 << 11,
    SLUR                    = 1 << 12,
    FIGURED_BASS            = 1 << 13,
    OTTAVA                  = 1 << 14,
    PEDAL_LINE              = 1 << 15,
    OTHER_LINE              = 1 << 16,
    ARPEGGIO                = 1 << 17,
    GLISSANDO               = 1 << 18,
    FRET_DIAGRAM            = 1 << 19,
    BREATH                  = 1 << 20,
    TREMOLO                 = 1 << 21,
    GRACE_NOTE              = 1 << 22,
    ALL                     = ~(~0u << NUMBER_OF_SELECTION_FILTER_TYPES)
};

class SelectionFilter
{
public:
    SelectionFilter() = default;
    SelectionFilter(SelectionFilterType type);

    inline bool operator==(const SelectionFilter& f) const { return m_filteredTypes == f.m_filteredTypes; }
    inline bool operator!=(const SelectionFilter& f) const { return !this->operator==(f); }

    bool isFiltered(SelectionFilterType type) const;
    void setFiltered(SelectionFilterType type, bool filtered);

    bool canSelect(const EngravingItem* element) const;
    bool canSelectVoice(track_idx_t track) const;

private:
    unsigned int m_filteredTypes = static_cast<unsigned int>(SelectionFilterType::ALL);
};
}
