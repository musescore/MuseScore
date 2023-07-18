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

#include "log.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;

StaffTreeItem::StaffTreeItem(IMasterNotationPtr masterNotation, INotationPtr notation, QObject* parent)
    : AbstractInstrumentsPanelTreeItem(InstrumentsTreeItemType::ItemType::STAFF, masterNotation, notation, parent)
{
    connect(this, &AbstractInstrumentsPanelTreeItem::isVisibleChanged, [this](bool isVisible) {
        if (!m_isInited) {
            return;
        }
        if (isVisible && !this->parentItem()->isVisible()) {
            this->parentItem()->setIsVisible(true, false);
        }

        this->notation()->parts()->setStaffVisible(id(), isVisible);
    });

    setIsEditable(true);
    setIsRemovable(true);
}

void StaffTreeItem::init(const Staff* masterStaff)
{
    IF_ASSERT_FAILED(masterStaff) {
        return;
    }

    const Staff* staff = notation()->parts()->staff(masterStaff->id());
    bool visible = staff && staff->show();

    if (!staff) {
        staff = masterStaff;
    }

    QString staffName = staff->staffName();

    //: Prefix for the display name for a linked staff. Preferably, keep this short.
    QString title = masterStaff->isLinked() ? qtrc("instruments", "[LINK] %1").arg(staffName) : staffName;

    setId(staff->id());
    setTitle(title);
    setIsVisible(visible);

    m_isInited = true;
}

bool StaffTreeItem::isSelectable() const
{
    return true;
}
