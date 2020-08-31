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

PartTreeItem::PartTreeItem(INotationParts* notationParts, QObject* parent)
    : AbstractInstrumentPanelTreeItem(ItemType::PART, notationParts, parent)
{
}

void PartTreeItem::moveChildren(const int sourceRow, const int count, AbstractInstrumentPanelTreeItem* destinationParent,
                                const int destinationRow)
{
    std::vector<QString> instrumentIdVector;

    for (int i = sourceRow; i < sourceRow + count; ++i) {
        instrumentIdVector.push_back(childAtRow(i)->id());
    }

    int destinationRowLast = destinationRow;
    INotationParts::InsertMode moveMode = INotationParts::Before;

    // exclude the control item
    if (destinationRow == destinationParent->childCount() - 1) {
        destinationRowLast = destinationRow - 1;
        moveMode = INotationParts::After;
    }

    AbstractInstrumentPanelTreeItem* destinationInstrumentItem = destinationParent->childAtRow(destinationRowLast);
    notationParts()->moveInstruments(instrumentIdVector, id(), destinationParent->id(), destinationInstrumentItem->id(), moveMode);

    AbstractInstrumentPanelTreeItem::moveChildren(sourceRow, count, destinationParent, destinationRow);
}

void PartTreeItem::removeChildren(const int row, const int count, const bool deleteChild)
{
    std::vector<QString> instrumentIdVector;

    for (int i = row; i < row + count; ++i) {
        instrumentIdVector.push_back(childAtRow(i)->id());
    }

    if (deleteChild) {
        notationParts()->removeInstruments(id(), instrumentIdVector);
    }

    AbstractInstrumentPanelTreeItem::removeChildren(row, count, deleteChild);
}
