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

#include "abstractselectionfilter.h"

namespace mu::engraving {
static constexpr size_t NUM_VOICES_SELECTION_FILTER_TYPES = 4;

enum class VoicesSelectionFilterTypes : unsigned int {
    NONE                    = 0,
    FIRST_VOICE             = 1 << 0,
    SECOND_VOICE            = 1 << 1,
    THIRD_VOICE             = 1 << 2,
    FOURTH_VOICE            = 1 << 3,
    ALL                     = ~(~0u << NUM_VOICES_SELECTION_FILTER_TYPES)
};

class VoicesSelectionFilter : public AbstractSelectionFilter
{
public:
    VoicesSelectionFilter(VoicesSelectionFilterTypes type = VoicesSelectionFilterTypes::ALL);

    unsigned int getAll() const override { return static_cast<unsigned int>(VoicesSelectionFilterTypes::ALL); }
    unsigned int getNone() const override { return static_cast<unsigned int>(VoicesSelectionFilterTypes::NONE); }

    bool isFiltered(const VoicesSelectionFilterTypes& type) const;
    void setFiltered(const VoicesSelectionFilterTypes& type, bool filtered);

    bool canSelectVoice(track_idx_t track) const;
};
}
