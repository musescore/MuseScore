/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#include "engravingelementsprovider.h"

#include "engraving/libmscore/score.h"

using namespace mu::diagnostics;

void EngravingElementsProvider::reg(const Ms::EngravingObject* e)
{
    if (e->score()->isPaletteScore()) {
        return;
    }

    m_elements.push_back(e);
    m_registreChanged.send(e, true);
}

void EngravingElementsProvider::unreg(const Ms::EngravingObject* e)
{
    m_elements.remove(e);
    m_registreChanged.send(e, false);
}

std::list<const Ms::EngravingObject*> EngravingElementsProvider::elements() const
{
    return m_elements;
}

mu::async::Channel<const Ms::EngravingObject*, bool> EngravingElementsProvider::registreChanged() const
{
    return m_registreChanged;
}

void EngravingElementsProvider::select(const Ms::EngravingObject* e, bool arg)
{
    if (arg) {
        m_selected.push_back(e);
    } else {
        m_selected.remove(e);
    }
    m_selectChanged.send(e, arg);
}

bool EngravingElementsProvider::isSelected(const Ms::EngravingObject* e) const
{
    if (std::find(m_selected.cbegin(), m_selected.cend(), e) != m_selected.cend()) {
        return true;
    }
    return false;
}

mu::async::Channel<const Ms::EngravingObject*, bool> EngravingElementsProvider::selectChanged() const
{
    return m_selectChanged;
}
