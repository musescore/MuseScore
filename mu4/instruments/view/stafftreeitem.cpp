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
    connect(this, &AbstractInstrumentPanelTreeItem::isVisibleChanged, [this](const bool isVisible) {
        this->notationParts()->setStaffVisible(id().toInt(), isVisible);
    });
}

void StaffTreeItem::setPartId(const QString& id)
{
    m_partId = id;
}

void StaffTreeItem::setInstrumentId(const QString& id)
{
    m_instrumentId = id;
}
