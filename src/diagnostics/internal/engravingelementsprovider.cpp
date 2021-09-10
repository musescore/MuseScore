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

#include <sstream>

#include "stringutils.h"

#include "engraving/libmscore/score.h"

#include "log.h"

using namespace mu::diagnostics;

void EngravingElementsProvider::clearStatistic()
{
    m_statistics.clear();
}

void EngravingElementsProvider::printStatistic(const std::string& title)
{
    #define FORMAT(str, width) mu::strings::leftJustified(str, width)
    #define TITLE(str) FORMAT(std::string(str), 20)
    #define VALUE(val) FORMAT(std::to_string(val), 20)

    std::stringstream stream;
    stream << "\n\n";
    stream << title << "\n";
    stream << TITLE("Object") << TITLE("created") << TITLE("deleted") << "\n";

    int regCountTotal = 0;
    int unregCountTotal = 0;
    for (auto it = m_statistics.begin(); it != m_statistics.end(); ++it) {
        const ObjectStatistic& s = it->second;
        stream << FORMAT(it->first, 20)
               << VALUE(s.regCount)
               << VALUE(s.unregCount)
               << "\n";

        regCountTotal += s.regCount;
        unregCountTotal += s.unregCount;
    }

    stream << "-----------------------------------------------------\n";
    stream << FORMAT("Total", 20) << VALUE(regCountTotal) << VALUE(unregCountTotal);

    LOGI() << stream.str() << '\n';
}

void EngravingElementsProvider::reg(const Ms::EngravingObject* e)
{
    TRACEFUNC;
    m_elements.insert(e);
    m_statistics[e->name()].regCount++;
}

void EngravingElementsProvider::unreg(const Ms::EngravingObject* e)
{
    TRACEFUNC;
    m_elements.erase(e);
    m_statistics[e->name()].unregCount++;
}

const EngravingObjectList& EngravingElementsProvider::elements() const
{
    return m_elements;
}

void EngravingElementsProvider::select(const Ms::EngravingObject* e, bool arg)
{
    if (arg) {
        m_selected.insert(e);
    } else {
        m_selected.erase(e);
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
