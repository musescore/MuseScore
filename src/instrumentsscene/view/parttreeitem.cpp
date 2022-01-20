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
#include "parttreeitem.h"

#include "log.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;
using ItemType = InstrumentsTreeItemType::ItemType;

PartTreeItem::PartTreeItem(IMasterNotationPtr masterNotation, INotationPtr notation, QObject* parent)
    : AbstractInstrumentsPanelTreeItem(ItemType::PART, masterNotation, notation, parent)
{
    listenVisibilityChanged();
}

void PartTreeItem::init(const notation::Part* masterPart)
{
    IF_ASSERT_FAILED(masterPart) {
        return;
    }

    const Part* part = notation()->parts()->part(masterPart->id());
    bool partExists = part != nullptr;
    bool visible = partExists && part->show();

    if (!partExists) {
        part = masterPart;
    }

    setId(part->id());
    setTitle(part->instrument()->name());
    setIsVisible(visible);
    setIsEditable(partExists);
    setIsExpandable(partExists);
    setIsRemovable(partExists);

    m_instrumentId = part->instrumentId();
    m_isInited = true;
}

bool PartTreeItem::isSelectable() const
{
    return true;
}

void PartTreeItem::listenVisibilityChanged()
{
    connect(this, &AbstractInstrumentsPanelTreeItem::isVisibleChanged, this, [this](bool isVisible) {
        if (!m_isInited) {
            return;
        }

        INotationPartsPtr parts = notation()->parts();
        if (!parts) {
            return;
        }

        if (parts->partExists(id())) {
            parts->setPartVisible(id(), isVisible);
        } else if (isVisible) {
            createAndAddPart(id());
        }
    });
}

void PartTreeItem::createAndAddPart(const ID& masterPartId)
{
    const Part* masterPart = masterNotation()->parts()->part(masterPartId);
    if (!masterPart) {
        return;
    }

    size_t index = resolveNewPartIndex(masterPartId);

    notation()->parts()->insertPart(masterPart->clone(), index);
}

size_t PartTreeItem::resolveNewPartIndex(const ID& partId) const
{
    IF_ASSERT_FAILED(parentItem()) {
        return 0;
    }

    bool partFound = false;
    ID firstVisiblePartId;

    for (int i = 0; i < parentItem()->childCount(); ++i) {
        const AbstractInstrumentsPanelTreeItem* item = parentItem()->childAtRow(i);

        if (item->id() == partId) {
            partFound = true;
            continue;
        }

        if (!partFound) {
            continue;
        }

        if (item->isVisible()) {
            firstVisiblePartId = item->id();
            break;
        }
    }

    auto parts = notation()->parts()->partList();

    for (size_t i = 0; i < parts.size(); ++i) {
        if (ID(parts[i]->id()) == firstVisiblePartId) {
            return i;
        }
    }

    return parts.size();
}

QString PartTreeItem::instrumentId() const
{
    return m_instrumentId;
}

void PartTreeItem::moveChildren(int sourceRow, int count, AbstractInstrumentsPanelTreeItem* destinationParent,
                                int destinationRow)
{
    IDList stavesIds;

    for (int i = sourceRow; i < sourceRow + count; ++i) {
        stavesIds.push_back(childAtRow(i)->id());
    }

    int destinationRowLast = destinationRow;
    INotationParts::InsertMode moveMode = INotationParts::InsertMode::Before;

    // exclude the control item
    if (destinationRow == destinationParent->childCount() - 1) {
        destinationRowLast = destinationRow - 1;
        moveMode = INotationParts::InsertMode::After;
    }

    AbstractInstrumentsPanelTreeItem* destinationStaffItem = destinationParent->childAtRow(destinationRowLast);
    notation()->parts()->moveStaves(stavesIds, destinationStaffItem->id(), moveMode);
    AbstractInstrumentsPanelTreeItem::moveChildren(sourceRow, count, destinationParent, destinationRow);
}

void PartTreeItem::removeChildren(int row, int count, bool deleteChild)
{
    IDList stavesIds;

    for (int i = row; i < row + count; ++i) {
        stavesIds.push_back(childAtRow(i)->id());
    }

    if (deleteChild) {
        masterNotation()->parts()->removeStaves(stavesIds);
    }

    AbstractInstrumentsPanelTreeItem::removeChildren(row, count, deleteChild);
}
