/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "../dom/score.h"

#include "log.h"

namespace mu::engraving {
void EngravingElementsProvider::clearStatistic()
{
    m_statistics.clear();
}

void EngravingElementsProvider::printStatistic(const std::string& title)
{
#define FORMAT(str, width) muse::strings::leftJustified(str, width)
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

    LOGD() << stream.str() << '\n';
}

void EngravingElementsProvider::reg(const mu::engraving::EngravingObject* e)
{
    m_elements.insert(e);
    m_statistics[e->typeName()].regCount++;
}

void EngravingElementsProvider::unreg(const mu::engraving::EngravingObject* e)
{
    m_elements.erase(e);
    m_statistics[e->typeName()].unregCount++;
}

const EngravingObjectSet& EngravingElementsProvider::elements() const
{
    return m_elements;
}

void EngravingElementsProvider::select(const mu::engraving::EngravingObject* e, bool arg)
{
    if (arg) {
        m_selected.insert(e);
    } else {
        m_selected.erase(e);
    }
    m_selectChanged.send(e, arg);
}

bool EngravingElementsProvider::isSelected(const mu::engraving::EngravingObject* e) const
{
    if (std::find(m_selected.cbegin(), m_selected.cend(), e) != m_selected.cend()) {
        return true;
    }
    return false;
}

muse::async::Channel<const mu::engraving::EngravingObject*, bool> EngravingElementsProvider::selectChanged() const
{
    return m_selectChanged;
}

void EngravingElementsProvider::dumpTree(const mu::engraving::EngravingItem* item, int& level)
{
    ++level;
    QString gap;
    gap.fill(' ', level);
    LOGD() << gap << item->typeName();
    for (const mu::engraving::EngravingObject* ch : item->children()) {
        if (!ch->isEngravingItem()) {
            LOGD() << "[" << item->typeName() << ": not item ch: " << ch->typeName();
            continue;
        }
        dumpTree(static_cast<const mu::engraving::EngravingItem*>(ch), level);
    }
    --level;
}
}
