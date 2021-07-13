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
#include "instrumenttreeitem.h"
#include "stafftreeitem.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;

InstrumentsTreeItem::InstrumentsTreeItem(INotationPartsPtr notationParts, QObject* parent)
    : AbstractInstrumentsPanelTreeItem(InstrumentsTreeItemType::ItemType::INSTRUMENT, notationParts, parent)
{
}

QString InstrumentsTreeItem::partId() const
{
    return m_partId;
}

QString InstrumentsTreeItem::partName() const
{
    return m_partName;
}

QString InstrumentsTreeItem::abbreviature() const
{
    return m_abbreviature;
}

void InstrumentsTreeItem::setPartId(const QString& partId)
{
    m_partId = partId;
}

void InstrumentsTreeItem::setPartName(const QString& partName)
{
    m_partName = partName;
}

void InstrumentsTreeItem::setAbbreviature(const QString& abbreviature)
{
    m_abbreviature = abbreviature;
}

void InstrumentsTreeItem::moveChildren(const int sourceRow, const int count, AbstractInstrumentsPanelTreeItem* destinationParent,
                                       const int destinationRow)
{
    IDList stavesIds;

    for (int i = sourceRow; i < sourceRow + count; ++i) {
        stavesIds.push_back(staffId(i));
    }

    int destinationRowLast = destinationRow;
    INotationParts::InsertMode moveMode = INotationParts::InsertMode::Before;

    // exclude the control item
    if (destinationRow == destinationParent->childCount() - 1) {
        destinationRowLast = destinationRow - 1;
        moveMode = INotationParts::InsertMode::After;
    }

    auto staff = dynamic_cast<const StaffTreeItem*>(destinationParent->childAtRow(destinationRowLast));
    if (!staff) {
        return;
    }

    ID destinationStaffId = staff->id();

    notationParts()->moveStaves(stavesIds, destinationStaffId, moveMode);

    AbstractInstrumentsPanelTreeItem::moveChildren(sourceRow, count, destinationParent, destinationRow);
}

void InstrumentsTreeItem::removeChildren(const int row, const int count, const bool deleteChild)
{
    IDList stavesIds;

    for (int i = row; i < row + count; ++i) {
        stavesIds.push_back(staffId(i));
    }

    if (deleteChild) {
        notationParts()->removeStaves(stavesIds);
    }

    AbstractInstrumentsPanelTreeItem::removeChildren(row, count, deleteChild);
}

void InstrumentsTreeItem::updateCanChangeVisibility()
{
    if (partId().isEmpty() || id().isEmpty()) {
        return;
    }

    ValCh<bool> canChangeVisibilityCh = notationParts()->canChangeInstrumentVisibility(id(), partId());
    setCanChangeVisibility(canChangeVisibilityCh.val);

    canChangeVisibilityCh.ch.onReceive(this, [this](bool value) {
        setCanChangeVisibility(value);
    });
}

ID InstrumentsTreeItem::staffId(int row) const
{
    auto staff = dynamic_cast<const StaffTreeItem*>(childAtRow(row));
    return staff ? staff->id() : ID();
}
