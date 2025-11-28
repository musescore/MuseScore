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

#pragma once

#include <QList>
#include <QHash>
#include <QAbstractListModel>
#include <qqmlintegration.h>

namespace mu::inspector {
class PageTypeListModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int currentPageSizeId READ currentPageSizeId WRITE setCurrentPageSizeId NOTIFY currentPageSizeIdChanged)

public:
    enum RoleNames {
        IdRole = Qt::UserRole + 1,
        NameRole
    };

    explicit PageTypeListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int currentPageSizeId() const;

public slots:
    void setCurrentPageSizeId(int currentPageSizeId);

signals:
    void currentPageSizeIdChanged(int currentPageSizeId);

private:
    int m_currentPageSizeId = -1;

    QList<int> m_pageSizeIdList;
    QHash<int, QByteArray> m_roleNames;
};
}
