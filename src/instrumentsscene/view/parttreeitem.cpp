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

using namespace mu::instrumentsscene;
using namespace mu::notation;
using ItemType = InstrumentsTreeItemType::ItemType;

PartTreeItem::PartTreeItem(INotationPartsPtr notationParts, QObject* parent)
    : AbstractInstrumentsPanelTreeItem(ItemType::PART, notationParts, parent)
{
}

QString PartTreeItem::instrumentId() const
{
    return m_instrumentId;
}

QString PartTreeItem::instrumentName() const
{
    return m_instrumentName;
}

QString PartTreeItem::instrumentAbbreviature() const
{
    return m_instrumentAbbreviature;
}

void PartTreeItem::setInstrumentId(const QString& instrumentId)
{
    m_instrumentId = instrumentId;
}

void PartTreeItem::setInstrumentName(const QString& name)
{
    m_instrumentName = name;
}

void PartTreeItem::setInstrumentAbbreviature(const QString& abbreviature)
{
    m_instrumentAbbreviature = abbreviature;
}

void PartTreeItem::moveChildren(const int sourceRow, const int count, AbstractInstrumentsPanelTreeItem* destinationParent,
                                const int destinationRow)
{
    IDList stavesIds;

    for (int i = sourceRow; i < sourceRow + count; ++i) {
        stavesIds << childAtRow(i)->id();
    }

    int destinationRowLast = destinationRow;
    INotationParts::InsertMode moveMode = INotationParts::InsertMode::Before;

    // exclude the control item
    if (destinationRow == destinationParent->childCount() - 1) {
        destinationRowLast = destinationRow - 1;
        moveMode = INotationParts::InsertMode::After;
    }

    AbstractInstrumentsPanelTreeItem* destinationStaffItem = destinationParent->childAtRow(destinationRowLast);
    notationParts()->moveStaves(stavesIds, destinationStaffItem->id(), moveMode);

    AbstractInstrumentsPanelTreeItem::moveChildren(sourceRow, count, destinationParent, destinationRow);
}

void PartTreeItem::removeChildren(const int row, const int count, const bool deleteChild)
{
    IDList stavesIds;

    for (int i = row; i < row + count; ++i) {
        stavesIds << childAtRow(i)->id();
    }

    if (deleteChild) {
        notationParts()->removeStaves(stavesIds);
    }

    AbstractInstrumentsPanelTreeItem::removeChildren(row, count, deleteChild);
}
