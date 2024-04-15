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

void EngravingElementsProvider::checkTree(mu::engraving::Score* score)
{
    LOGD() << "\n\n\n";
    LOGD() << "========================";
    checkObjectTree(score->rootItem());

    LOGD() << "========================";
//    LOGI() << "dumpTree:";
//    int level = 0;
//    dumpTree(m_masterScore->rootItem(), level);

//    LOGI() << "========================";
//    LOGI() << "dumpTreeTree:";
//    level = 0;
//    dumpTreeTree(m_masterScore, level);

//    LOGI() << "========================";
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

void EngravingElementsProvider::dumpTreeTree(const mu::engraving::EngravingObject* obj, int& level)
{
    ++level;
    QString gap;
    gap.fill(' ', level);
    LOGD() << gap << obj->typeName();

    for (const mu::engraving::EngravingObject* child : obj->scanChildren()) {
        dumpTreeTree(child, level);
    }

    --level;
}

void EngravingElementsProvider::checkObjectTree(const mu::engraving::EngravingObject* obj)
{
    mu::engraving::EngravingObject* p1 = obj->parent();
    mu::engraving::EngravingObject* p2 = obj->scanParent();
    if (p1 && p2 && p1 != p2) {
        LOGD() << "obj: " << obj->typeName();
        LOGD() << "parents is differens, p1: " << p1->typeName() << ", p2: " << p2->typeName();
    }

    size_t ch1 = obj->children().size();
    size_t ch2 = obj->scanChildren().size();
    if (ch1 != ch2) {
        LOGD() << "obj: " << obj->typeName();
        LOGD() << "chcount is differens, ch1: " << ch1 << ", ch2: " << ch2;

        LOGD() << "children1:";
        for (size_t i = 0; i < obj->children().size(); ++i) {
            LOGD() << i << ": " << obj->children().at(i)->typeName();
        }

        LOGD() << "children2:";

        int i = 0;
        for (const mu::engraving::EngravingObject* child : obj->scanChildren()) {
            LOGD() << i++ << ": " << child->typeName();
        }
    }

    for (const mu::engraving::EngravingObject* ch : obj->children()) {
        checkObjectTree(ch);
    }
}
}
