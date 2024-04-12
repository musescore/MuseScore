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
#ifndef MU_ENGRAVING_ENGRAVINGELEMENTSPROVIDER_H
#define MU_ENGRAVING_ENGRAVINGELEMENTSPROVIDER_H

#include <string>
#include <map>

#include "iengravingelementsprovider.h"

namespace mu::engraving {
class Score;
class EngravingItem;
class EngravingElementsProvider : public IEngravingElementsProvider
{
public:
    EngravingElementsProvider() = default;

    // statistic
    void clearStatistic() override;
    void printStatistic(const std::string& title) override;

    // register
    void reg(const mu::engraving::EngravingObject* e) override;
    void unreg(const mu::engraving::EngravingObject* e) override;
    const EngravingObjectSet& elements() const override;

    // debug draw
    void select(const mu::engraving::EngravingObject* e, bool arg) override;
    bool isSelected(const mu::engraving::EngravingObject* e) const override;
    muse::async::Channel<const mu::engraving::EngravingObject*, bool> selectChanged() const override;

    void checkTree(mu::engraving::Score* score);

private:

    void checkObjectTree(const mu::engraving::EngravingObject* obj);
    void dumpTree(const mu::engraving::EngravingItem* item, int& level);
    void dumpTreeTree(const mu::engraving::EngravingObject* obj, int& level);

    struct ObjectStatistic
    {
        int regCount = 0;
        int unregCount = 0;
    };

    std::map<std::string, ObjectStatistic> m_statistics;

    EngravingObjectSet m_elements;

    EngravingObjectSet m_selected;
    muse::async::Channel<const mu::engraving::EngravingObject*, bool> m_selectChanged;
};
}

#endif // MU_ENGRAVING_ENGRAVINGELEMENTSPROVIDER_H
