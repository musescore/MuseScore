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
#include <set>

#include "engraving/dom/score.h"

#include "parttreeitem.h"
#include "systemobjectslayertreeitem.h"

#include "log.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace muse;

static inline bool isPartItem(const AbstractLayoutPanelTreeItem* item)
{
    return item->type() == LayoutPanelItemType::PART || item->type() == LayoutPanelItemType::SHARED_PART;
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

RootTreeItem::ApplyResult RootTreeItem::applyToScore()
{
    IDList partIds;
    std::vector<std::pair<SystemObjectsLayerTreeItem*, const Staff*> > layerDestinations;
    std::vector<INotationParts::SystemObjectsMove> moves;
    const Part* partBelow = nullptr;

    for (int row = childCount() - 1; row >= 0; --row) {
        AbstractLayoutPanelTreeItem* child = childAtRow(row);
        if (isPartItem(child)) {
            if (notation()->parts()->partExists(child->id())) {
                partIds.push_back(child->id());
            }
            partBelow = static_cast<const PartTreeItem*>(child)->part();
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
        moves.push_back({ staff->id(), destination->id(), !partBelow });
    }

    std::reverse(partIds.begin(), partIds.end());
    std::reverse(moves.begin(), moves.end());

    notation()->parts()->reorderParts(partIds, moves);

    if (moves.empty()) {
        return ApplyResult::Applied;
    }

    IF_ASSERT_FAILED(partBelow) {
        return ApplyResult::AppliedPartly;
    }

    // partBelow is now the top instrument, whose first staff always has markings
    std::set<const Staff*> expectedStaves { partBelow->staff(0) };
    for (const auto& [layer, destination] : layerDestinations) {
        expectedStaves.insert(destination);
    }

    const std::vector<Staff*>& scoreStaves = notation()->parts()->systemObjectStaves();
    const bool allMoved = expectedStaves.size() == layerDestinations.size() + 1
                          && expectedStaves == std::set<const Staff*>(scoreStaves.begin(), scoreStaves.end());
    if (!allMoved) {
        return ApplyResult::AppliedPartly;
    }

    for (const auto& [layer, destination] : layerDestinations) {
        layer->setStaff(destination);
    }

    return ApplyResult::Applied;
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
