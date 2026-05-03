/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "filtervalue.h"

#include "sortfilterproxymodel.h"

using namespace muse::uicomponents;

FilterValue::FilterValue(QObject* parent)
    : Filter(parent)
{
}

bool FilterValue::acceptsRow(const int sourceRow, const QModelIndex& sourceParent,
                             const SortFilterProxyModel& proxyModel)
{
    const int role = proxyModel.roleFromRoleName(m_roleName);
    if (role == -1) {
        return true;
    }

    const QAbstractItemModel* sourceModel = proxyModel.sourceModel();
    const QModelIndex index = sourceModel->index(sourceRow, 0, sourceParent);
    const QVariant data = sourceModel->data(index, role);
    switch (m_compareType) {
    case CompareType::Equal:
        return data == m_roleValue;
    case CompareType::NotEqual:
        return data != m_roleValue;
    case CompareType::Contains:
        return data.toString().contains(m_roleValue.toString(), Qt::CaseInsensitive);
    }

    return false;
}

QString FilterValue::roleName() const
{
    return m_roleName;
}

QVariant FilterValue::roleValue() const
{
    return m_roleValue;
}

CompareType::Type FilterValue::compareType() const
{
    return m_compareType;
}

void FilterValue::setRoleName(QString roleName)
{
    if (m_roleName == roleName) {
        return;
    }

    m_roleName = roleName;
    emit dataChanged();
}

void FilterValue::setRoleValue(QVariant value)
{
    if (m_roleValue == value) {
        return;
    }

    m_roleValue = value;
    emit dataChanged();
}

void FilterValue::setCompareType(CompareType::Type type)
{
    if (m_compareType == type) {
        return;
    }

    m_compareType = type;
    emit dataChanged();
}
