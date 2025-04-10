/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

#ifndef MU_ENGRAVING_ALIGNMENTLAYOUT_DEV_H
#define MU_ENGRAVING_ALIGNMENTLAYOUT_DEV_H

#include <vector>
#include <set>
#include <functional>

namespace mu::engraving {
class EngravingItem;
class System;
}

namespace mu::engraving::rendering::score {
class AlignmentLayout
{
public:
    static void alignItemsWithTheirSnappingChain(const std::vector<EngravingItem*>& elements, const System* system);
    static void alignStaffCenteredItems(const std::vector<EngravingItem*>& elements, const System* system);
    static void alignItemsForSystem(const std::vector<EngravingItem*>& elements, const System* system);
    static void alignItemsGroup(const std::vector<EngravingItem*>& elements, const System* system);

private:
    static void moveItemToY(EngravingItem* item, double y, const System* system);
    static double yOpticalCenter(const EngravingItem* item);
    static void scanConnectedItems(EngravingItem* item, const System* system, std::function<void(EngravingItem*)> func);
    static double computeAverageY(const std::vector<double>& vecOfY);
};
} // namespace mu::engraving::rendering::dev
#endif // MU_ENGRAVING_ALIGNMENTLAYOUT_DEV_H
