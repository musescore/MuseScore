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

#include "../mscore.h"
#include "../chord.h"

#include "abstractselectionfilter.h"

namespace mu::engraving {
static constexpr size_t NUM_ELEMENTS_SELECTION_FILTER_TYPES = 19;

enum class ElementsSelectionFilterTypes : unsigned int {
    NONE                    = 0,
    DYNAMIC                 = 1 << 0,
    HAIRPIN                 = 1 << 1,
    FINGERING               = 1 << 2,
    LYRICS                  = 1 << 3,
    CHORD_SYMBOL            = 1 << 4,
    OTHER_TEXT              = 1 << 5,
    ARTICULATION            = 1 << 6,
    ORNAMENT                = 1 << 7,
    SLUR                    = 1 << 8,
    FIGURED_BASS            = 1 << 9,
    OTTAVA                  = 1 << 10,
    PEDAL_LINE              = 1 << 11,
    OTHER_LINE              = 1 << 12,
    ARPEGGIO                = 1 << 13,
    GLISSANDO               = 1 << 14,
    FRET_DIAGRAM            = 1 << 15,
    BREATH                  = 1 << 16,
    TREMOLO                 = 1 << 17,
    GRACE_NOTE              = 1 << 18,
    ALL                     = ~(~0u << NUM_ELEMENTS_SELECTION_FILTER_TYPES)
};

class ElementsSelectionFilter : public AbstractSelectionFilter
{
public:
    ElementsSelectionFilter(ElementsSelectionFilterTypes type = ElementsSelectionFilterTypes::ALL);

    unsigned int getAll() const override { return static_cast<unsigned int>(ElementsSelectionFilterTypes::ALL); }
    unsigned int getNone() const override { return static_cast<unsigned int>(ElementsSelectionFilterTypes::NONE); }

    bool isFiltered(const ElementsSelectionFilterTypes& type) const;
    void setFiltered(const ElementsSelectionFilterTypes& type, bool filtered);

    bool canSelect(const EngravingItem* element) const;
};
}
