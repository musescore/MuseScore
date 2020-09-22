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
#include "instrumenttreeitem.h"
#include "stafftreeitem.h"

using namespace mu::instruments;
using namespace mu::notation;

InstrumentTreeItem::InstrumentTreeItem(INotationParts* notationParts, QObject* parent)
    : AbstractInstrumentPanelTreeItem(InstrumentTreeItemType::ItemType::INSTRUMENT, notationParts, parent)
{
}

QString InstrumentTreeItem::partId() const
{
    return m_partId;
}

QString InstrumentTreeItem::partName() const
{
    return m_partName;
}

QString InstrumentTreeItem::abbreviature() const
{
    return m_abbreviature;
}

void InstrumentTreeItem::setPartId(const QString& partId)
{
    m_partId = partId;
}

void InstrumentTreeItem::setPartName(const QString& partName)
{
    m_partName = partName;
}

void InstrumentTreeItem::setAbbreviature(const QString& abbreviature)
{
    m_abbreviature = abbreviature;
}

void InstrumentTreeItem::moveChildren(const int sourceRow, const int count, AbstractInstrumentPanelTreeItem* destinationParent,
                                      const int destinationRow)
{
    IDList stavesIds;

    for (int i = sourceRow; i < sourceRow + count; ++i) {
        stavesIds.push_back(staffId(i));
    }

    int destinationRowLast = destinationRow;
    INotationParts::InsertMode moveMode = INotationParts::Before;

    // exclude the control item
    if (destinationRow == destinationParent->childCount() - 1) {
        destinationRowLast = destinationRow - 1;
        moveMode = INotationParts::After;
    }

    auto staff = dynamic_cast<const StaffTreeItem*>(destinationParent->childAtRow(destinationRowLast));
    if (!staff) {
        return;
    }

    ID destinationStaffId = staff->id();

    notationParts()->moveStaves(stavesIds, destinationStaffId, moveMode);

    AbstractInstrumentPanelTreeItem::moveChildren(sourceRow, count, destinationParent, destinationRow);
}

void InstrumentTreeItem::removeChildren(const int row, const int count, const bool deleteChild)
{
    IDList stavesIds;

    for (int i = row; i < row + count; ++i) {
        stavesIds.push_back(staffId(i));
    }

    if (deleteChild) {
        notationParts()->removeStaves(stavesIds);
    }

    AbstractInstrumentPanelTreeItem::removeChildren(row, count, deleteChild);
}

void InstrumentTreeItem::updateCanChangeVisibility()
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

ID InstrumentTreeItem::staffId(int row) const
{
    auto staff = dynamic_cast<const StaffTreeItem*>(childAtRow(row));
    return staff ? staff->id() : ID();
}
