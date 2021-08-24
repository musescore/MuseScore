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
#include "roottreeitem.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;

RootTreeItem::RootTreeItem(IMasterNotationPtr masterNotation, INotationPtr notation, QObject* parent)
    : AbstractInstrumentsPanelTreeItem(InstrumentsTreeItemType::ItemType::ROOT, masterNotation, notation, parent)
{
}

void RootTreeItem::moveChildren(const int sourceRow, const int count, AbstractInstrumentsPanelTreeItem* destinationParent,
                                const int destinationRow)
{
    IDList partIds;

    for (int i = sourceRow; i < sourceRow + count; ++i) {
        ID partId = childAtRow(i)->id();

        if (notation()->parts()->partExists(partId)) {
            partIds.push_back(childAtRow(i)->id());
        }
    }

    int destinationRow_ = destinationRow;
    INotationParts::InsertMode moveMode = INotationParts::InsertMode::Before;

    if (destinationRow_ == destinationParent->childCount()) {
        destinationRow_--;
        moveMode = INotationParts::InsertMode::After;
    }

    const AbstractInstrumentsPanelTreeItem* destinationPartItem = destinationParent->childAtRow(destinationRow_);
    if (!destinationPartItem) {
        return;
    }

    notation()->parts()->moveParts(partIds, destinationPartItem->id(), moveMode);

    AbstractInstrumentsPanelTreeItem::moveChildren(sourceRow, count, destinationParent, destinationRow);
}

void RootTreeItem::removeChildren(const int row, const int count, const bool deleteChild)
{
    IDList partIds;

    for (int i = row; i < row + count; ++i) {
        partIds.push_back(childAtRow(i)->id());
    }

    if (deleteChild) {
        notation()->parts()->removeParts(partIds);
    }

    AbstractInstrumentsPanelTreeItem::removeChildren(row, count, deleteChild);
}
