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

#include "voicesselectionfiltermodel.h"

#include "log.h"
#include "translation.h"

using namespace mu::notation;
using namespace mu::engraving;

VoicesSelectionFilterModel::VoicesSelectionFilterModel(QObject* parent)
    : AbstractSelectionFilterModel(parent)
{
}

void VoicesSelectionFilterModel::loadTypes()
{
    for (size_t i = 0; i < NUM_VOICES_SELECTION_FILTER_TYPES; i++) {
        m_types << static_cast<VoicesSelectionFilterTypes>(1 << i);
    }
}

QString VoicesSelectionFilterModel::titleForType(const SelectionFilterTypesVariant& variant) const
{
    const VoicesSelectionFilterTypes type = std::get<VoicesSelectionFilterTypes>(variant);

    switch (type) {
    case VoicesSelectionFilterTypes::ALL:
        return muse::qtrc("notation", "All");
    case VoicesSelectionFilterTypes::FIRST_VOICE:
        return muse::qtrc("notation", "Voice %1").arg(1);
    case VoicesSelectionFilterTypes::SECOND_VOICE:
        return muse::qtrc("notation", "Voice %1").arg(2);
    case VoicesSelectionFilterTypes::THIRD_VOICE:
        return muse::qtrc("notation", "Voice %1").arg(3);
    case VoicesSelectionFilterTypes::FOURTH_VOICE:
        return muse::qtrc("notation", "Voice %1").arg(4);
    case VoicesSelectionFilterTypes::NONE:
        break;
    }

    return {};
}
