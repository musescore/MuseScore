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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "tabdurationsymbol.h"

#include "chordrest.h"
#include "stafftype.h"

using namespace mu::engraving;

TabDurationSymbol::TabDurationSymbol(ChordRest* parent)
    : EngravingItem(ElementType::TAB_DURATION_SYMBOL, parent, ElementFlag::NOT_SELECTABLE)
{
    setGenerated(true);
    m_tab        = 0;
    m_text       = String();
}

TabDurationSymbol::TabDurationSymbol(ChordRest* parent, const StaffType* tab, DurationType type, int dots)
    : EngravingItem(ElementType::TAB_DURATION_SYMBOL, parent, ElementFlag::NOT_SELECTABLE)
{
    setGenerated(true);
    setDuration(type, dots, tab);
}

TabDurationSymbol::TabDurationSymbol(const TabDurationSymbol& e)
    : EngravingItem(e)
{
    m_tab = e.m_tab;
    m_text = e.m_text;
}

void TabDurationSymbol::setDuration(DurationType type, int dots, const StaffType* tab)
{
    m_tab = tab;
    m_text = tab->durationString(type, dots);
}
