/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "engraving/dom/factory.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace mu::engraving;

StaffControlTreeItem::StaffControlTreeItem(IMasterNotationPtr masterNotation, INotationPtr notation, QObject* parent)
    : AbstractInstrumentsPanelTreeItem(InstrumentsTreeItemType::ItemType::CONTROL_ADD_STAFF, masterNotation, notation, parent)
{
    setTitle(muse::qtrc("instruments", "Add staff"));
}

void StaffControlTreeItem::init(const ID& partId)
{
    m_partId = partId;
}

void StaffControlTreeItem::appendNewItem()
{
    const Part* part = masterNotation()->parts()->part(m_partId);
    if (!part) {
        return;
    }

    staff_idx_t lastStaffIndex = part->nstaves();

    Staff* staff = Factory::createStaff(const_cast<Part*>(part));
    staff->setDefaultClefType(part->instrument()->clefType(lastStaffIndex));

    masterNotation()->parts()->appendStaff(staff, m_partId);
}
