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

#include "notationselectionfilter.h"

#include "log.h"

using namespace muse;
using namespace mu::notation;

NotationSelectionFilter::NotationSelectionFilter(const IGetScore* getScore, const std::function<void()>& selectionChangedCallback)
    : m_getScore(getScore), m_selectionChangedCallback(selectionChangedCallback)
{
}

bool NotationSelectionFilter::isSelectionTypeFiltered(const SelectionFilterTypesVariant& variant) const
{
    return score()->selectionFilter().isFiltered(variant);
}

void NotationSelectionFilter::setSelectionTypeFiltered(const SelectionFilterTypesVariant& variant, bool filtered)
{
    engraving::Selection& selection = score()->selection();

    score()->selectionFilter().setFiltered(variant, filtered);

    if (selection.isRange()) {
        selection.updateSelectedElements();
        m_selectionChangedCallback();
    }
}

mu::engraving::Score* NotationSelectionFilter::score() const
{
    return m_getScore->score();
}
