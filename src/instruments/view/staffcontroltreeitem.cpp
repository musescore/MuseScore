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
#include "staffcontroltreeitem.h"

using namespace mu::instruments;
using namespace mu::notation;

StaffControlTreeItem::StaffControlTreeItem(INotationPartsPtr notationParts, QObject* parent)
    : AbstractInstrumentPanelTreeItem(InstrumentTreeItemType::ItemType::CONTROL_ADD_STAFF, notationParts, parent)
{
}

void StaffControlTreeItem::appendNewItem()
{
    const Part* part = this->part();
    if (!part) {
        return;
    }

    int lastStaffIndex = part->nstaves();

    Staff* staff = new Staff();
    staff->setDefaultClefType(part->instrument()->clefType(lastStaffIndex));

    notationParts()->appendStaff(staff, m_partId);
}

const Part* StaffControlTreeItem::part() const
{
    for (const Part* part : notationParts()->partList()) {
        if (part->id() == m_partId) {
            return part;
        }
    }

    return nullptr;
}

void StaffControlTreeItem::setPartId(const QString& id)
{
    m_partId = id;
}
