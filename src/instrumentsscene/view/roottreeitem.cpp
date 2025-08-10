/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "parttreeitem.h"
#include "stafftreeitem.h"
#include "systemobjectslayertreeitem.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace muse;

static inline bool isIndexInRange(int index, int start, int end)
{
    return start <= index && index < end;
}

RootTreeItem::RootTreeItem(IMasterNotationPtr masterNotation, INotationPtr notation, QObject* parent)
    : AbstractLayoutPanelTreeItem(LayoutPanelItemType::ROOT, masterNotation, notation, parent)
{
}

MoveParams RootTreeItem::buildMoveParams(int sourceRow, int count, AbstractLayoutPanelTreeItem* destinationParent,
                                         int destinationRow) const
{
    // User is explicitly moving a system objects layer
    if (childType(sourceRow) == LayoutPanelItemType::SYSTEM_OBJECTS_LAYER) {
        DO_ASSERT(count == 1); // only 1 layer can be moved at a time
        return buildSystemObjectsMoveParams(sourceRow, 1, destinationRow);
    }

    // User is going to place parts after a system objects layer
    // Update the system objects position on the score if the resulting parts order remains the same
    if (!partsOrderWillBeChanged(sourceRow, count, destinationRow)) {
        const int newRow = sourceRow > destinationRow ? destinationRow : destinationRow - 1;
        if (childType(newRow) == LayoutPanelItemType::SYSTEM_OBJECTS_LAYER) {
            return buildSystemObjectsMoveParams(sourceRow, count, newRow);
        }
    }

    return buildPartsMoveParams(sourceRow, count, destinationParent, destinationRow);
}

void RootTreeItem::moveChildren(int sourceRow, int count, AbstractLayoutPanelTreeItem* destinationParent,
                                int destinationRow, bool updateNotation)
{
    if (updateNotation) {
        MoveParams moveParams = buildMoveParams(sourceRow, count, destinationParent, destinationRow);
        moveChildrenOnScore(moveParams);
    }

    AbstractLayoutPanelTreeItem::moveChildren(sourceRow, count, destinationParent, destinationRow, updateNotation);
}

void RootTreeItem::moveChildrenOnScore(const MoveParams& params)
{
    if (params.objectsType == LayoutPanelItemType::SYSTEM_OBJECTS_LAYER && !params.destinationObjectId.isValid()) {
        if (params.moveSysObjBelowBottomStaff) {
            notation()->parts()->moveSystemObjectLayerBelowBottomStaff();
        } else {
            notation()->parts()->removeSystemObjects(params.objectIdListToMove);
        }
        return;
    }

    if (!params.isValid()) {
        return;
    }

    if (params.objectsType == LayoutPanelItemType::SYSTEM_OBJECTS_LAYER) {
        auto systemObjectsLayerItem = dynamic_cast<SystemObjectsLayerTreeItem*>(childAtId(params.objectIdListToMove.front(),
                                                                                          LayoutPanelItemType::SYSTEM_OBJECTS_LAYER));

        const Staff* dstStaff = notation()->parts()->staff(params.destinationObjectId);

        if (systemObjectsLayerItem && dstStaff && systemObjectsLayerItem->staff() != dstStaff) {
            notation()->parts()->moveSystemObjects(systemObjectsLayerItem->staff()->id(), dstStaff->id());
            systemObjectsLayerItem->setStaff(dstStaff);
        }

        if (params.moveSysObjBelowBottomStaff) {
            notation()->parts()->moveSystemObjectLayerBelowBottomStaff();
        } else if (params.moveSysObjAboveBottomStaff) {
            notation()->parts()->moveSystemObjectLayerAboveBottomStaff();
        }
    } else {
        notation()->parts()->moveParts(params.objectIdListToMove, params.destinationObjectId, params.insertMode);
    }
}

void RootTreeItem::removeChildren(int row, int count, bool deleteChild)
{
    IDList partIds;
    IDList stavesIds;

    for (int i = row; i < row + count; ++i) {
        const AbstractLayoutPanelTreeItem* child = childAtRow(i);

        if (child->type() == LayoutPanelItemType::PART) {
            partIds.push_back(child->id());
        } else if (child->type() == LayoutPanelItemType::SYSTEM_OBJECTS_LAYER) {
            stavesIds.push_back(child->id());
        }
    }

    if (deleteChild) {
        masterNotation()->notation()->parts()->removeParts(partIds);
        masterNotation()->notation()->parts()->removeSystemObjects(stavesIds);
    }

    AbstractLayoutPanelTreeItem::removeChildren(row, count, deleteChild);
}

bool RootTreeItem::partsOrderWillBeChanged(int sourceRow, int count, int destinationRow) const
{
    if (sourceRow > destinationRow) {
        for (int i = sourceRow - 1; i >= destinationRow; --i) {
            if (childType(i) == LayoutPanelItemType::PART) {
                return true;
            }
        }
    } else {
        for (int i = sourceRow + count; i < destinationRow; ++i) {
            if (childType(i) == LayoutPanelItemType::PART) {
                return true;
            }
        }
    }

    return false;
}

MoveParams RootTreeItem::buildSystemObjectsMoveParams(int sourceRow, int count, int destinationRow) const
{
    const AbstractLayoutPanelTreeItem* srcItem = childAtRow(sourceRow);
    const AbstractLayoutPanelTreeItem* dstItem = childAtRow(destinationRow);
    if (!srcItem) {
        return {};
    }

    const Staff* srcStaff = nullptr;
    const Staff* dstStaff = nullptr;

    if (srcItem->type() == LayoutPanelItemType::SYSTEM_OBJECTS_LAYER && destinationRow >= childCount()) {
        srcStaff = static_cast<const SystemObjectsLayerTreeItem*>(srcItem)->staff();
        dstStaff = srcStaff->score()->staves().back();
        MoveParams moveParams;
        moveParams.objectsType = LayoutPanelItemType::SYSTEM_OBJECTS_LAYER;
        moveParams.objectIdListToMove.push_back(srcStaff->id());
        moveParams.destinationObjectId = dstStaff->id();
        moveParams.moveSysObjBelowBottomStaff = true;
        return moveParams;
    }

    if (!dstItem) {
        return {};
    }

    auto resolveDstStaff = [](const AbstractLayoutPanelTreeItem* item) -> const Staff* {
        if (item) {
            if (item->type() == LayoutPanelItemType::PART) {
                return static_cast<const PartTreeItem*>(item)->part()->staff(0);
            } else if (item->type() == LayoutPanelItemType::SYSTEM_OBJECTS_LAYER) {
                return static_cast<const SystemObjectsLayerTreeItem*>(item)->staff();
            }
        }

        return nullptr;
    };

    if (srcItem->type() == LayoutPanelItemType::SYSTEM_OBJECTS_LAYER) {
        srcStaff = static_cast<const SystemObjectsLayerTreeItem*>(srcItem)->staff();
        dstStaff = resolveDstStaff(dstItem);
    } else if (dstItem->type() == LayoutPanelItemType::SYSTEM_OBJECTS_LAYER) {
        srcStaff = static_cast<const SystemObjectsLayerTreeItem*>(dstItem)->staff();

        if (sourceRow > destinationRow) {
            dstStaff = resolveDstStaff(childAtRow(sourceRow + count));
        } else {
            dstStaff = resolveDstStaff(srcItem);
        }
    }

    if (!srcStaff) {
        return {};
    }

    MoveParams moveParams;
    moveParams.objectsType = LayoutPanelItemType::SYSTEM_OBJECTS_LAYER;
    moveParams.objectIdListToMove.push_back(srcStaff->id());
    if (dstStaff) {
        moveParams.destinationObjectId = dstStaff->id();
    }
    if (srcStaff->systemObjectsBelowBottomStaff() && destinationRow < sourceRow) {
        moveParams.moveSysObjAboveBottomStaff = true;
    }

    return moveParams;
}

MoveParams RootTreeItem::buildPartsMoveParams(int sourceRow, int count, AbstractLayoutPanelTreeItem* destinationParent,
                                              int destinationRow) const
{
    IDList partIds;
    for (int i = sourceRow; i < sourceRow + count; ++i) {
        ID partId = childAtRow(i)->id();

        if (notation()->parts()->partExists(partId)) {
            partIds.push_back(partId);
        }
    }

    MoveParams moveParams;
    moveParams.objectsType = LayoutPanelItemType::PART;
    moveParams.objectIdListToMove = partIds;

    int destinationRow_ = destinationRow;
    int childCount = destinationParent->childCount();
    bool moveDown = destinationRow > sourceRow;
    INotationParts::InsertMode moveMode = INotationParts::InsertMode::Before;
    const AbstractLayoutPanelTreeItem* destinationPartItem = nullptr;

    do {
        destinationPartItem = destinationParent->childAtRow(destinationRow_);

        if (destinationPartItem) {
            if (destinationPartItem->type() == LayoutPanelItemType::PART) {
                if (notation()->parts()->partExists(destinationPartItem->id())) {
                    break;
                }
            }

            destinationPartItem = nullptr;
        }

        if (moveDown) {
            destinationRow_--;
            moveMode = INotationParts::InsertMode::After;
        } else {
            destinationRow_++;
            moveMode = INotationParts::InsertMode::Before;
        }
    } while (isIndexInRange(destinationRow_, 0, childCount) && !isIndexInRange(destinationRow_, sourceRow, sourceRow + count));

    if (destinationPartItem) {
        moveParams.destinationObjectId = destinationPartItem->id();
    }

    moveParams.insertMode = moveMode;

    return moveParams;
}
