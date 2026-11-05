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

#pragma once

#include "abstractlayoutpaneltreeitem.h"

#include "notation/inotation.h"

namespace mu::instrumentsscene {
class StaffTreeItem : public AbstractLayoutPanelTreeItem
{
    Q_OBJECT

    QML_ELEMENT;
    QML_UNCREATABLE("Must be created in C++ only")

    // Linked staves: the rows (in the instrument) of this staff and the others it's linked to, the link icon's tooltip,
    // and the row's name for screen readers. See PartTreeItem::updateStaffLinks()
    Q_PROPERTY(bool isLinked READ isLinked NOTIFY linksChanged)
    // The staff's row in the instrument (the view's row index isn't reliably updated when rows are moved)
    Q_PROPERTY(int staffRow READ staffRow NOTIFY linksChanged)
    Q_PROPERTY(QVariantList linkRows READ linkRows NOTIFY linksChanged)
    Q_PROPERTY(QString linkDescription READ linkDescription NOTIFY linksChanged)
    Q_PROPERTY(QString accessibleTitle READ accessibleTitle NOTIFY linksChanged)

public:
    StaffTreeItem(notation::IMasterNotationPtr masterNotation, notation::INotationPtr notation, QObject* parent);

    void init(const notation::Staff* masterStaff);

    // The staff's name, before it's numbered to tell it apart from others of the same name
    QString staffName() const;

    bool isLinked() const;
    int staffRow() const;
    QVariantList linkRows() const;
    QString linkDescription() const;
    QString accessibleTitle() const;

    void setLinks(int staffRow, const QVariantList& linkRows, const QString& linkDescription, const QString& accessibleTitle);

signals:
    void linksChanged();

private:
    bool m_isInited = false;
    QString m_staffName;
    int m_staffRow = -1;
    QVariantList m_linkRows;
    QString m_linkDescription;
    QString m_accessibleTitle;
};
}
