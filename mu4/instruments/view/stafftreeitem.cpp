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
#include "stafftreeitem.h"

using namespace mu::instruments;
using namespace mu::notation;

StaffTreeItem::StaffTreeItem(INotationParts* notationParts, QObject* parent)
    : AbstractInstrumentPanelTreeItem(InstrumentTreeItemType::ItemType::STAFF, notationParts, parent)
{
}

int StaffTreeItem::staffIndex() const
{
    return id().toInt();
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

void StaffTreeItem::setVoicesVisibility(const QVariantList &visibility)
{
    m_voicesVisibility = visibility;
}

void StaffTreeItem::setPartId(const QString& id)
{
    m_partId = id;
}

void StaffTreeItem::setInstrumentId(const QString& id)
{
    m_instrumentId = id;
}
