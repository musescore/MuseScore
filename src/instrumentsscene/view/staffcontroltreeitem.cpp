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
#include "staffcontroltreeitem.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;

StaffControlTreeItem::StaffControlTreeItem(INotationPartsPtr notationParts, QObject* parent)
    : AbstractInstrumentsPanelTreeItem(InstrumentsTreeItemType::ItemType::CONTROL_ADD_STAFF, notationParts, parent)
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
