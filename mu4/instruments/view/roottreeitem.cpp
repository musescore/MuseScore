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
#include "roottreeitem.h"

using namespace mu::instruments;
using namespace mu::notation;

RootTreeItem::RootTreeItem(INotationParts* notationParts, QObject* parent)
    : AbstractInstrumentPanelTreeItem(InstrumentTreeItemType::ItemType::ROOT, notationParts, parent)
{
}

void RootTreeItem::moveChildren(const int sourceRow, const int count, AbstractInstrumentPanelTreeItem* destinationParent,
                                const int destinationRow)
{
    std::vector<QString> partIdVector;

    for (int i = sourceRow; i < sourceRow + count; ++i) {
        partIdVector.push_back(childAtRow(i)->id());
    }

    const AbstractInstrumentPanelTreeItem* destinationPartItem = destinationParent->childAtRow(destinationRow);
    if (!destinationPartItem) {
        return;
    }

    INotationParts::InsertMode moveMode = destinationRow
                                          != destinationParent->childCount() ? INotationParts::Before : INotationParts::After;
    notationParts()->moveParts(partIdVector, destinationPartItem->id(), moveMode);

    AbstractInstrumentPanelTreeItem::moveChildren(sourceRow, count, destinationParent, destinationRow);
}

void RootTreeItem::removeChildren(const int row, const int count, const bool deleteChild)
{
    std::vector<QString> partIdVector;

    for (int i = row; i < row + count; ++i) {
        partIdVector.push_back(childAtRow(i)->id());
    }

    if (deleteChild) {
        notationParts()->removeParts(partIdVector);
    }

    AbstractInstrumentPanelTreeItem::removeChildren(row, count, deleteChild);
}
