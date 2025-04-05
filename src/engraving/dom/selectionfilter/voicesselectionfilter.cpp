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

#include "voicesselectionfilter.h"

using namespace mu::engraving;

VoicesSelectionFilter::VoicesSelectionFilter(VoicesSelectionFilterTypes type)
    : AbstractSelectionFilter()
{
    m_filteredTypes = static_cast<int>(type);
}

bool VoicesSelectionFilter::isFiltered(const VoicesSelectionFilterTypes& type) const
{
    return AbstractSelectionFilter::isFiltered(static_cast<unsigned int>(type));
}

void VoicesSelectionFilter::setFiltered(const VoicesSelectionFilterTypes& type, bool filtered)
{
    AbstractSelectionFilter::setFiltered(static_cast<unsigned int>(type), filtered);
}

bool VoicesSelectionFilter::canSelectVoice(track_idx_t track) const
{
    voice_idx_t voice = track % VOICES;
    switch (voice) {
    case 0:
        return isFiltered(VoicesSelectionFilterTypes::FIRST_VOICE);
    case 1:
        return isFiltered(VoicesSelectionFilterTypes::SECOND_VOICE);
    case 2:
        return isFiltered(VoicesSelectionFilterTypes::THIRD_VOICE);
    case 3:
        return isFiltered(VoicesSelectionFilterTypes::FOURTH_VOICE);
    }
    return true;
}
