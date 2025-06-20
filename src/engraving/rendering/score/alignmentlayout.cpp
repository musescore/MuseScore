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

#include "alignmentlayout.h"
#include "systemlayout.h"

#include "dom/dynamic.h"
#include "dom/expression.h"
#include "dom/hairpin.h"
#include "dom/system.h"
#include "dom/text.h"

namespace mu::engraving::rendering::score {
void AlignmentLayout::alignItemsGroup(const std::vector<EngravingItem*>& elements, const System* system)
{
    if (elements.empty()) {
        return;
    }

    double outermostY = yOpticalCenter(elements.front());
    for (const EngravingItem* element : elements) {
        double curY = yOpticalCenter(element);
        outermostY = element->placeAbove() ? std::min(outermostY, curY) : std::max(outermostY, curY);
    }

    for (EngravingItem* element : elements) {
        moveItemToY(element, outermostY, system);
    }
}

void AlignmentLayout::alignItemsWithTheirSnappingChain(const std::vector<EngravingItem*>& elements, const System* system)
{
    std::set<EngravingItem*> alignedItems;

    double outermostY = 0.0;
    bool firstItem = true;
    auto computeOutermostY = [&outermostY, &firstItem](EngravingItem* item) {
        double curY = yOpticalCenter(item);
        if (firstItem) {
            outermostY = curY;
            firstItem = false;
        } else {
            outermostY = item->placeAbove() ? std::min(outermostY, curY) : std::max(outermostY, curY);
        }
    };

    auto moveElementsToOutermostY = [&outermostY, &alignedItems, system](EngravingItem* item) {
        alignedItems.insert(item);
        moveItemToY(item, outermostY, system);
    };

    for (EngravingItem* item : elements) {
        if (muse::contains(alignedItems, item)) {
            continue;
        }
        firstItem = true;
        scanConnectedItems(item, system, computeOutermostY);
        scanConnectedItems(item, system, moveElementsToOutermostY);
    }
}

void AlignmentLayout::alignStaffCenteredItems(const std::vector<EngravingItem*>& elements, const System* system)
{
    std::vector<double> vecOfCurrentY;

    auto collectCurrentYandComputeEdges = [&vecOfCurrentY](EngravingItem* item) {
        vecOfCurrentY.push_back(yOpticalCenter(item));
    };

    double averageY = 0.0;
    auto limitAverageYInsideAvailableSpace = [&averageY](EngravingItem* item) {
        double yCur = yOpticalCenter(item);
        double intendedMove = averageY - yCur;
        const EngravingItem::LayoutData::StaffCenteringInfo& staffCenteringInfo = item->ldata()->staffCenteringInfo();
        double maxMoveAbove = -staffCenteringInfo.availableVertSpaceAbove;
        double maxMoveBelow = staffCenteringInfo.availableVertSpaceBelow;
        double maxAllowedMove = std::clamp(intendedMove, maxMoveAbove, maxMoveBelow);
        if (!muse::RealIsEqual(maxAllowedMove, intendedMove)) {
            averageY += -intendedMove + maxAllowedMove;
        }
    };

    std::set<EngravingItem*> alignedItems;
    auto moveElementsToAverageY = [&averageY, &alignedItems, system](EngravingItem* item) {
        alignedItems.insert(item);
        moveItemToY(item, averageY, system);
    };

    for (EngravingItem* item : elements) {
        if (muse::contains(alignedItems, item)) {
            continue;
        }
        vecOfCurrentY.clear();
        scanConnectedItems(item, system, collectCurrentYandComputeEdges);
        averageY = computeAverageY(vecOfCurrentY);
        scanConnectedItems(item, system, limitAverageYInsideAvailableSpace);
        scanConnectedItems(item, system, moveElementsToAverageY);
    }
}

void AlignmentLayout::alignItemsForSystem(const std::vector<EngravingItem*>& elements, const System* system)
{
    struct StaffItemGroups {
        std::vector<EngravingItem*> itemsAbove;
        std::vector<EngravingItem*> itemsBelow;
    };

    std::map<staff_idx_t, StaffItemGroups> staffItems;

    for (EngravingItem* item : elements) {
        if (item->addToSkyline() && item->verticalAlign()) {
            item->placeAbove() ? staffItems[item->staffIdx()].itemsAbove.push_back(item)
            : staffItems[item->staffIdx()].itemsBelow.push_back(item);
        }
    }

    for (const auto& staffItem : staffItems) {
        AlignmentLayout::alignItemsGroup(staffItem.second.itemsAbove, system);
        AlignmentLayout::alignItemsGroup(staffItem.second.itemsBelow, system);
    }
}

void AlignmentLayout::moveItemToY(EngravingItem* item, double y, const System* system)
{
    double curY = yOpticalCenter(item);
    double yDiff = y - curY;
    item->mutldata()->moveY(yDiff);
    SystemLayout::updateSkylineForElement(item, system, yDiff);
}

double AlignmentLayout::yOpticalCenter(const EngravingItem* item)
{
    double curY = item->pos().y();
    switch (item->type()) {
    case ElementType::DYNAMIC:
    case ElementType::EXPRESSION:
    case ElementType::FOOTNOTE:
    {
        curY += item->staffOffsetY();
        AlignV vertAlign = toTextBase(item)->align().vertical;
        double bboxHeight = item->ldata()->bbox().height();
        switch (vertAlign) {
        case AlignV::TOP:
            curY += 0.5 * bboxHeight;
            break;
        case AlignV::VCENTER:
            break;
        case AlignV::BOTTOM:
            curY -= 0.5 * bboxHeight;
            break;
        case AlignV::BASELINE:
            if (item->isDynamic()) {
                curY -= 0.46 * item->spatium() * toDynamic(item)->dynamicsSize(); // approximated half x-height of dynamic
            } else {
                curY -= 0.5 * toExpression(item)->fontMetrics().xHeight();
            }
            break;
        }
        break;
    }
    case ElementType::HAIRPIN_SEGMENT:
    {
        const HairpinSegment* hairpinSeg = toHairpinSegment(item);
        if (hairpinSeg->hairpin()->isLineType()) {
            Text* text = hairpinSeg->text();
            if (!text) {
                text = hairpinSeg->endText();
            }
            if (text) {
                curY -= 0.5 * text->fontMetrics().xHeight();
            } else {
                curY -= 0.5 * item->spatium();
            }
        }
    }
    default:
        break;
    }
    return curY;
}

void AlignmentLayout::scanConnectedItems(EngravingItem* item, const System* system, std::function<void(EngravingItem*)> func)
{
    func(item);

    EngravingItem* snappedBefore = item->ldata()->itemSnappedBefore();
    while (snappedBefore && snappedBefore->findAncestor(ElementType::SYSTEM) == system) {
        func(snappedBefore);
        snappedBefore = snappedBefore->ldata()->itemSnappedBefore();
    }

    EngravingItem* snappedAfter = item->ldata()->itemSnappedAfter();
    while (snappedAfter && snappedAfter->findAncestor(ElementType::SYSTEM) == system) {
        func(snappedAfter);
        snappedAfter = snappedAfter->ldata()->itemSnappedAfter();
    }
}

double AlignmentLayout::computeAverageY(const std::vector<double>& vecOfY)
{
    double sum = std::accumulate(vecOfY.begin(), vecOfY.end(), 0.0);
    return sum / static_cast<double>(vecOfY.size());
}
} // namespace mu::engraving::rendering::dev
