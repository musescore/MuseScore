/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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
#include "roottreeitem.h"

#include <algorithm>

#include "engraving/dom/score.h"
#include "engraving/dom/sharedpart.h"

#include "parttreeitem.h"
#include "systemobjectslayertreeitem.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace muse;

static inline bool isPartItem(const AbstractLayoutPanelTreeItem* item)
{
    return item->type() == LayoutPanelItemType::PART || item->type() == LayoutPanelItemType::SHARED_PART;
}

static inline bool isSharedGroupMember(const PartTreeItem* item)
{
    return item && (item->sharedGroupRole() == LayoutPanelItemType::GROUP_MEMBER
                    || item->sharedGroupRole() == LayoutPanelItemType::GROUP_LAST_MEMBER);
}

static const PartTreeItem* nextPartItemAfter(const QList<AbstractLayoutPanelTreeItem*>& items, int row)
{
    for (int i = row + 1; i < items.size(); ++i) {
        if (items.at(i)->type() != LayoutPanelItemType::SYSTEM_OBJECTS_LAYER) {
            return dynamic_cast<const PartTreeItem*>(items.at(i));
        }
    }

    return nullptr;
}

RootTreeItem::RootTreeItem(IMasterNotationPtr masterNotation, INotationPtr notation, QObject* parent)
    : AbstractLayoutPanelTreeItem(LayoutPanelItemType::ROOT, masterNotation, notation, parent)
{
}

void RootTreeItem::onScoreChanged(const mu::engraving::ScoreChanges& changes)
{
    for (AbstractLayoutPanelTreeItem* child : childItems()) {
        child->onScoreChanged(changes);
    }

    updateSharedGroupRoles();
}

void RootTreeItem::moveChildren(int sourceRow, int count, AbstractLayoutPanelTreeItem* destinationParent,
                                int destinationRow, bool updateNotation)
{
    AbstractLayoutPanelTreeItem::moveChildren(sourceRow, count, destinationParent, destinationRow, updateNotation);

    if (updateNotation) {
        applyToScore();
    }
}

void RootTreeItem::applyToScore()
{
    INotationParts::PartsLayout layout;
    std::vector<std::pair<SystemObjectsLayerTreeItem*, const Staff*> > layerDestinations;
    const Part* partBelow = nullptr;

    for (int row = childCount() - 1; row >= 0; --row) {
        AbstractLayoutPanelTreeItem* child = childAtRow(row);
        if (isPartItem(child)) {
            partBelow = static_cast<const PartTreeItem*>(child)->part();

            if (notation()->parts()->partExists(child->id())) {
                INotationParts::PartPlacement placement { child->id() };
                if (const SharedPart* sharedPart = partBelow->sharedPart()) {
                    placement.sharedPartId = sharedPart->id();
                }
                layout.parts.push_back(placement);
            }
            continue;
        }

        // The first row's markings are always on the top staff
        if (row == 0 || child->type() != LayoutPanelItemType::SYSTEM_OBJECTS_LAYER) {
            continue;
        }

        auto layer = static_cast<SystemObjectsLayerTreeItem*>(child);
        const Staff* staff = layer->staff();
        const Staff* destination = nullptr;

        if (!partBelow) {
            destination = staff->score()->staves().back();
        } else if (partBelow == staff->part() && !staff->hasSystemObjectsBelowBottomStaff()) {
            destination = staff;
        } else {
            destination = partBelow->staff(0);
        }

        layerDestinations.emplace_back(layer, destination);
        layout.systemObjects.push_back({ staff->id(), destination->id(), !partBelow });
    }

    std::reverse(layout.parts.begin(), layout.parts.end());
    std::reverse(layout.systemObjects.begin(), layout.systemObjects.end());

    notation()->parts()->setPartsLayout(layout);

    // Markings that couldn't move are put back when the panel updates its markings rows
    for (const auto& [layer, destination] : layerDestinations) {
        layer->setStaff(destination);
    }
}

void RootTreeItem::removeChildren(int row, int count, bool deleteChild)
{
    if (deleteChild) {
        IDList partIds;
        IDList stavesIds;

        for (int i = row; i < row + count; ++i) {
            const AbstractLayoutPanelTreeItem* child = childAtRow(i);

            if (isPartItem(child)) {
                partIds.push_back(child->id());
            } else if (child->type() == LayoutPanelItemType::SYSTEM_OBJECTS_LAYER) {
                stavesIds.push_back(child->id());
            }
        }

        masterNotation()->notation()->parts()->removeParts(partIds);
        masterNotation()->notation()->parts()->removeSystemObjects(stavesIds);
    }

    AbstractLayoutPanelTreeItem::removeChildren(row, count, deleteChild);
}

void RootTreeItem::onChildrenChanged()
{
    updateSharedGroupRoles();
}

void RootTreeItem::updateSharedGroupRoles()
{
    const QList<AbstractLayoutPanelTreeItem*>& items = childItems();
    PartTreeItem* currentHeader = nullptr;

    for (int i = 0; i < items.size(); ++i) {
        PartTreeItem* partItem = dynamic_cast<PartTreeItem*>(items.at(i));
        if (!partItem) {
            // Markings rows sit next to their instrument, even inside a group
            if (items.at(i)->type() != LayoutPanelItemType::SYSTEM_OBJECTS_LAYER) {
                currentHeader = nullptr;
            }
            continue;
        }

        LayoutPanelItemType::SharedGroupRole role = LayoutPanelItemType::NOT_IN_GROUP;

        if (partItem->type() == LayoutPanelItemType::SHARED_PART) {
            currentHeader = partItem;
            role = LayoutPanelItemType::GROUP_HEADER;
        } else if (currentHeader && partItem->part()->sharedPart() == currentHeader->part()) {
            const PartTreeItem* nextPartItem = nextPartItemAfter(items, i);
            const bool isLast = !nextPartItem || nextPartItem->part()->sharedPart() != currentHeader->part();
            role = isLast ? LayoutPanelItemType::GROUP_LAST_MEMBER : LayoutPanelItemType::GROUP_MEMBER;
        } else {
            currentHeader = nullptr;
        }

        partItem->setSharedGroup(role, currentHeader);
    }
}

PartTreeItem* RootTreeItem::sharedGroupHeaderAt(int row) const
{
    const AbstractLayoutPanelTreeItem* item = childAtRow(row);
    if (!item) {
        return nullptr;
    }

    if (item->type() == LayoutPanelItemType::SYSTEM_OBJECTS_LAYER) {
        const PartTreeItem* nextPartItem = nextPartItemAfter(childItems(), row);
        return isSharedGroupMember(nextPartItem) ? nextPartItem->sharedGroupHeader() : nullptr;
    }

    const PartTreeItem* partItem = dynamic_cast<const PartTreeItem*>(item);
    return partItem ? partItem->sharedGroupHeader() : nullptr;
}

int RootTreeItem::sharedGroupLastRow(const PartTreeItem* header, int fromRow) const
{
    int lastRow = fromRow;

    for (int row = fromRow + 1; row < childCount(); ++row) {
        const AbstractLayoutPanelTreeItem* item = childAtRow(row);
        if (item->type() == LayoutPanelItemType::SYSTEM_OBJECTS_LAYER) {
            continue;
        }

        const PartTreeItem* partItem = dynamic_cast<const PartTreeItem*>(item);
        if (!partItem || partItem->sharedGroupHeader() != header) {
            break;
        }

        lastRow = row;
    }

    return lastRow;
}

std::optional<RootTreeItem::StepMove> RootTreeItem::resolveStepMove(int firstRow, int lastRow, bool up) const
{
    const PartTreeItem* lastItem = dynamic_cast<const PartTreeItem*>(childAtRow(lastRow));
    if (lastItem && lastItem->sharedGroupRole() == LayoutPanelItemType::GROUP_HEADER) {
        lastRow = sharedGroupLastRow(lastItem, lastRow);
    }

    if (leavesSharedGroup(firstRow, lastRow, up)) {
        return std::nullopt;
    }

    int destinationRow = up ? firstRow - 1 : lastRow + 1;

    const PartTreeItem* destinationHeader = sharedGroupHeaderAt(destinationRow);
    if (destinationHeader && destinationHeader != sharedGroupHeaderAt(firstRow)) {
        if (up) {
            while (destinationRow > 0 && childAtRow(destinationRow) != destinationHeader) {
                --destinationRow;
            }
        } else {
            destinationRow = sharedGroupLastRow(destinationHeader, destinationRow);
        }
    }

    if (up) {
        const int minRow = childType(0) == LayoutPanelItemType::SYSTEM_OBJECTS_LAYER ? 1 : 0;
        if (destinationRow < minRow) {
            return std::nullopt;
        }
    } else if (destinationRow >= childCount()) {
        return std::nullopt;
    }

    return StepMove { firstRow, lastRow - firstRow + 1, destinationRow };
}

bool RootTreeItem::leavesSharedGroup(int firstRow, int lastRow, bool up) const
{
    const PartTreeItem* item = dynamic_cast<const PartTreeItem*>(childAtRow(up ? firstRow : lastRow));
    if (!isSharedGroupMember(item)) {
        return false;
    }

    if (up) {
        return childAtRow(firstRow - 1) == item->sharedGroupHeader();
    }

    return item->sharedGroupRole() == LayoutPanelItemType::GROUP_LAST_MEMBER && childAtRow(firstRow) != item->sharedGroupHeader();
}
