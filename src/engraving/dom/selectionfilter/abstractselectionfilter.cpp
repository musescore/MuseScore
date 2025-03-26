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

#include "abstractselectionfilter.h"

using namespace mu::engraving;

bool AbstractSelectionFilter::isFiltered(unsigned int type) const
{
    if (type == getNone() || type == getAll()) {
        return m_filteredTypes == type;
    }

    return m_filteredTypes & type;
}

void AbstractSelectionFilter::setFiltered(unsigned int type, bool filtered)
{
    if (type == getNone()) {
        setFiltered(getAll(), !filtered);
        return;
    }

    filtered ? m_filteredTypes |= type : m_filteredTypes &= ~type;
}
