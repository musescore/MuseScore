/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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

#include "engraving/dom/part.h"

#include "log.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;

StaffTreeItem::StaffTreeItem(IMasterNotationPtr masterNotation, INotationPtr notation, QObject* parent)
    : AbstractLayoutPanelTreeItem(LayoutPanelItemType::STAFF, masterNotation, notation, parent)
{
    connect(this, &AbstractLayoutPanelTreeItem::isVisibleChanged, [this](bool isVisible) {
        if (!m_isInited) {
            return;
        }
        if (isVisible && !this->parentItem()->isVisible()) {
            this->parentItem()->setIsVisible(true, false);
        }

        this->notation()->parts()->setStaffVisible(id(), isVisible);
    });

    setSettingsAvailable(true);
    setSettingsEnabled(true);
    setIsRemovable(true);
    setIsSelectable(true);
}

void StaffTreeItem::init(const Staff* masterStaff)
{
    IF_ASSERT_FAILED(masterStaff) {
        return;
    }

    const Staff* staff = notation()->parts()->staff(masterStaff->id());

    // Not Staff::show(): that is also false while the instrument is combined onto shared staves,
    // which the Layout panel shows on the group instead of on the instrument's own staves
    bool visible = staff && staff->visible() && staff->part()->getProperty(mu::engraving::Pid::VISIBLE).toBool();

    if (!staff) {
        staff = masterStaff;
    }

    QString staffName = staff->staffType()->longName();
    if (staffName.isEmpty()) {
        staffName = staff->staffName();
    }

    // Linked staves are shown with a link icon (see PartTreeItem::updateStaffLinks(), which also numbers staves of the
    // same name)
    m_staffName = staffName;

    setId(staff->id());
    setTitle(staffName);
    setIsVisible(visible);

    m_isInited = true;
}

QString StaffTreeItem::staffName() const
{
    return m_staffName;
}

bool StaffTreeItem::isLinked() const
{
    return m_linkRows.size() > 1;
}

int StaffTreeItem::staffRow() const
{
    return m_staffRow;
}

QVariantList StaffTreeItem::linkRows() const
{
    return m_linkRows;
}

QString StaffTreeItem::linkDescription() const
{
    return m_linkDescription;
}

QString StaffTreeItem::accessibleTitle() const
{
    return m_accessibleTitle.isEmpty() ? title() : m_accessibleTitle;
}

void StaffTreeItem::setLinks(int staffRow, const QVariantList& linkRows, const QString& linkDescription,
                             const QString& accessibleTitle)
{
    // Also when only this staff's own row changed (e.g. it was moved), so that its connector line follows
    if (m_staffRow == staffRow && m_linkRows == linkRows && m_linkDescription == linkDescription
        && m_accessibleTitle == accessibleTitle) {
        return;
    }

    m_staffRow = staffRow;
    m_linkRows = linkRows;
    m_linkDescription = linkDescription;
    m_accessibleTitle = accessibleTitle;

    emit linksChanged();
}
