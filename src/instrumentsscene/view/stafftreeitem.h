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
#ifndef MU_INSTRUMENTSSCENE_STAFFTREEITEM_H
#define MU_INSTRUMENTSSCENE_STAFFTREEITEM_H

#include "abstractinstrumentspaneltreeitem.h"

#include "notation/inotationparts.h"

namespace mu::instrumentsscene {
class StaffTreeItem : public AbstractInstrumentsPanelTreeItem
{
    Q_OBJECT

public:
    explicit StaffTreeItem(notation::INotationPartsPtr notationParts, QObject* parent = nullptr);

    Q_INVOKABLE bool isSmall() const;
    Q_INVOKABLE bool cutawayEnabled() const;
    Q_INVOKABLE int staffType() const;
    Q_INVOKABLE QVariantList voicesVisibility() const;

    void setIsSmall(bool value);
    void setCutawayEnabled(bool value);
    void setStaffType(int type);
    void setVoicesVisibility(const QVariantList& visibility);

private:
    bool m_isSmall = false;
    bool m_cutawayEnabled = false;
    int m_staffType = 0;
    QVariantList m_voicesVisibility;
};
}

#endif // MU_INSTRUMENTSSCENE_STAFFTREEITEM_H
