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
#ifndef MU_DIAGNOSTICS_ENGRAVINGELEMENTSPROVIDER_H
#define MU_DIAGNOSTICS_ENGRAVINGELEMENTSPROVIDER_H

#include <string>
#include <map>

#include "../iengravingelementsprovider.h"

namespace Ms {
class Score;
class EngravingItem;
}

namespace mu::diagnostics {
class EngravingElementsProvider : public IEngravingElementsProvider
{
public:
    EngravingElementsProvider() = default;

    // statistic
    void clearStatistic() override;
    void printStatistic(const std::string& title) override;

    // registr
    void reg(const Ms::EngravingObject* e) override;
    void unreg(const Ms::EngravingObject* e) override;
    const EngravingObjectList& elements() const override;

    // debug draw
    void select(const Ms::EngravingObject* e, bool arg) override;
    bool isSelected(const Ms::EngravingObject* e) const override;
    async::Channel<const Ms::EngravingObject*, bool> selectChanged() const override;

    void checkTree(Ms::Score* score);

private:

    void checkObjectTree(const Ms::EngravingObject* obj);
    void dumpTree(const Ms::EngravingItem* item, int& level);
    void dumpTreeTree(const Ms::EngravingObject* obj, int& level);

    struct ObjectStatistic
    {
        int regCount = 0;
        int unregCount = 0;
    };

    std::map<std::string, ObjectStatistic> m_statistics;

    EngravingObjectList m_elements;

    EngravingObjectList m_selected;
    async::Channel<const Ms::EngravingObject*, bool> m_selectChanged;
};
}

#endif // MU_DIAGNOSTICS_ENGRAVINGELEMENTSPROVIDER_H
