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

void RootTreeItem::onScoreChanged(const mu::engraving::ScoreChanges& changes)
{
    for (AbstractLayoutPanelTreeItem* child : childItems()) {
        child->onScoreChanged(changes);
    }

    updateSharedGroupRoles();
}

std::optional<RootTreeItem::RowMove> RootTreeItem::resolveMove(int firstRow, int lastRow, int targetRow) const
{
    if (targetRow == firstRow) {
        return std::nullopt;
    }

    const std::optional<RowRange> lastRowGroup = sharedGroupRange(lastRow);
    if (lastRowGroup && lastRowGroup->first == lastRow) {
        lastRow = lastRowGroup->last;
    }

    const bool up = targetRow < firstRow;
    int destinationRow = up ? targetRow : std::max(targetRow, lastRow + 1);

    const std::optional<RowRange> group = sharedGroupRange(firstRow);
    const bool isGroupMember = group && childType(firstRow) == LayoutPanelItemType::PART;
    if (isGroupMember && (destinationRow <= group->first || destinationRow > group->last)) {
        return std::nullopt;
    }

    const std::optional<RowRange> destinationGroup = sharedGroupRange(destinationRow);
    if (destinationGroup && (!group || group->first != destinationGroup->first)) {
        destinationRow = up ? destinationGroup->first : destinationGroup->last;
    }

    const int minRow = childType(0) == LayoutPanelItemType::SYSTEM_OBJECTS_LAYER ? 1 : 0;
    if (firstRow < minRow || destinationRow < minRow || destinationRow >= childCount()) {
        return std::nullopt;
    }

    return RowMove { firstRow, lastRow - firstRow + 1, destinationRow };
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

std::optional<RootTreeItem::RowRange> RootTreeItem::sharedGroupRange(int row) const
{
    const AbstractLayoutPanelTreeItem* item = childAtRow(row);
    if (!item) {
        return std::nullopt;
    }

    // A markings row is in a group only among its instruments
    const bool isMarkings = item->type() == LayoutPanelItemType::SYSTEM_OBJECTS_LAYER;
    const PartTreeItem* partItem = isMarkings ? nextPartItemAfter(childItems(), row) : dynamic_cast<const PartTreeItem*>(item);
    if (!partItem || !partItem->sharedGroupHeader() || (isMarkings && !isSharedGroupMember(partItem))) {
        return std::nullopt;
    }

    const PartTreeItem* header = partItem->sharedGroupHeader();
    RowRange range { header->row(), header->row() };

    for (int r = range.first + 1; r < childCount(); ++r) {
        const AbstractLayoutPanelTreeItem* child = childAtRow(r);
        if (child->type() == LayoutPanelItemType::SYSTEM_OBJECTS_LAYER) {
            continue;
        }

        const PartTreeItem* member = dynamic_cast<const PartTreeItem*>(child);
        if (!member || member->sharedGroupHeader() != header) {
            break;
        }

        range.last = r;
    }

    return range;
}
