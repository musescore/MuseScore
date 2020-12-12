//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "parttreeitem.h"

using namespace mu::instruments;
using namespace mu::notation;
using ItemType = InstrumentTreeItemType::ItemType;

PartTreeItem::PartTreeItem(INotationPartsPtr notationParts, QObject* parent)
    : AbstractInstrumentPanelTreeItem(ItemType::PART, notationParts, parent)
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

void PartTreeItem::moveChildren(const int sourceRow, const int count, AbstractInstrumentPanelTreeItem* destinationParent,
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

    AbstractInstrumentPanelTreeItem* destinationStaffItem = destinationParent->childAtRow(destinationRowLast);
    notationParts()->moveStaves(stavesIds, destinationStaffItem->id(), moveMode);

    AbstractInstrumentPanelTreeItem::moveChildren(sourceRow, count, destinationParent, destinationRow);
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

    AbstractInstrumentPanelTreeItem::removeChildren(row, count, deleteChild);
}
