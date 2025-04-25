/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#pragma once

#include "abstractlayoutpaneltreeitem.h"

#include "notation/inotationparts.h"

namespace mu::instrumentsscene {
class StaffTreeItem : public AbstractLayoutPanelTreeItem
{
    Q_OBJECT
    Q_PROPERTY(bool isLinked READ isLinked NOTIFY isLinkedChanged)
    Q_PROPERTY(QString linkedStaffName READ linkedStaffName NOTIFY linkedStaffNameChanged)

public:
    StaffTreeItem(notation::IMasterNotationPtr masterNotation, notation::INotationPtr notation, QObject* parent);

    bool isLinked() const { return m_isLinked; }
    QString linkedStaffName() const { return m_linkedStaffName; }

    void init(const notation::Staff* masterStaff);

signals:
    void isLinkedChanged(bool isLinked);
    void linkedStaffNameChanged(QString linkedStaffName);

private:
    void setIsLinked(bool linked);
    void setLinkedStaffName(const QString& name);

    bool m_isInited = false;
    bool m_isLinked = false;
    QString m_linkedStaffName;
};
}
