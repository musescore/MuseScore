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
#include "stafftreeitem.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;

StaffTreeItem::StaffTreeItem(INotationPartsPtr notationParts, QObject* parent)
    : AbstractInstrumentsPanelTreeItem(InstrumentsTreeItemType::ItemType::STAFF, notationParts, parent)
{
}

bool StaffTreeItem::isSmall() const
{
    return m_isSmall;
}

bool StaffTreeItem::cutawayEnabled() const
{
    return m_cutawayEnabled;
}

int StaffTreeItem::staffType() const
{
    return m_staffType;
}

QVariantList StaffTreeItem::voicesVisibility() const
{
    return m_voicesVisibility;
}

void StaffTreeItem::setIsSmall(bool value)
{
    m_isSmall = value;
}

void StaffTreeItem::setCutawayEnabled(bool value)
{
    m_cutawayEnabled = value;
}

void StaffTreeItem::setStaffType(int type)
{
    m_staffType = type;
}

void StaffTreeItem::setVoicesVisibility(const QVariantList& visibility)
{
    m_voicesVisibility = visibility;
}
