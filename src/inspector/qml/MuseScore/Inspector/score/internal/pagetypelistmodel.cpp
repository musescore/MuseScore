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
#include "pagetypelistmodel.h"

#include <QPageSize>

using namespace mu::inspector;

PageTypeListModel::PageTypeListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_roleNames.insert(IdRole, "idRole");
    m_roleNames.insert(NameRole, "nameRole");

    for (int i = 0; i < QPageSize::LastPageSize; ++i) {
        m_pageSizeIdList << i;
    }
}

int PageTypeListModel::rowCount(const QModelIndex&) const
{
    return m_pageSizeIdList.count();
}

QVariant PageTypeListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount() || m_pageSizeIdList.isEmpty()) {
        return QVariant();
    }

    int pageSizeId = m_pageSizeIdList.at(index.row());

    switch (role) {
    case IdRole: return pageSizeId;
    case NameRole: return QPageSize::name(static_cast<QPageSize::PageSizeId>(pageSizeId));
    default: return QVariant();
    }
}

QHash<int, QByteArray> PageTypeListModel::roleNames() const
{
    return m_roleNames;
}

int PageTypeListModel::currentPageSizeId() const
{
    return m_currentPageSizeId;
}

void PageTypeListModel::setCurrentPageSizeId(int currentPageSizeId)
{
    if (m_currentPageSizeId == currentPageSizeId) {
        return;
    }

    m_currentPageSizeId = currentPageSizeId;
    emit currentPageSizeIdChanged(m_currentPageSizeId);
}
