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
#include "sortfilterproxymodel.h"

#include "types/val.h"

#include "modelutils.h"

using namespace mu::uicomponents;

static const int INVALID_KEY = -1;

SortFilterProxyModel::SortFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent), m_filters(this), m_sorters(this)
{
    ModelUtils::connectRowCountChangedSignal(this, &SortFilterProxyModel::rowCountChanged);

    connect(m_filters.notifier(), &QmlListPropertyNotifier::appended, this, [this](int index) {
        connect(m_filters.at(index), &FilterValue::dataChanged, this, [this]() {
            if (sourceModel()) {
                fillRoleIds();
            }
        });
    });

    connect(m_sorters.notifier(), &QmlListPropertyNotifier::appended, this, [this](int index) {
        connect(m_sorters.at(index), &SorterValue::dataChanged, this, [this]() {
            SorterValue* sorter = currentSorterValue();
            invalidate();

            if (!sorter) {
                return;
            }

            if (sourceModel()) {
                sort(0, sorter->sortOrder());
            }
        });
    });
}

QQmlListProperty<FilterValue> SortFilterProxyModel::filters()
{
    return m_filters.property();
}

QQmlListProperty<SorterValue> SortFilterProxyModel::sorters()
{
    return m_sorters.property();
}

QList<int> SortFilterProxyModel::alwaysIncludeIndices() const
{
    return m_alwaysIncludeIndices;
}

void SortFilterProxyModel::setAlwaysIncludeIndices(const QList<int>& indices)
{
    if (m_alwaysIncludeIndices == indices) {
        return;
    }

    m_alwaysIncludeIndices = indices;
    emit alwaysIncludeIndicesChanged();

    invalidateFilter();
}

QList<int> SortFilterProxyModel::alwaysExcludeIndices() const
{
    return m_alwaysExcludeIndices;
}

void SortFilterProxyModel::setAlwaysExcludeIndices(const QList<int>& indices)
{
    if (m_alwaysExcludeIndices == indices) {
        return;
    }

    m_alwaysExcludeIndices = indices;
    emit alwaysExcludeIndicesChanged();

    invalidateFilter();
}

void SortFilterProxyModel::refresh()
{
    setFilterFixedString(filterRegularExpression().pattern());
    setSortCaseSensitivity(sortCaseSensitivity());
}

bool SortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if (m_alwaysIncludeIndices.contains(sourceRow)) {
        return true;
    }

    if (m_alwaysExcludeIndices.contains(sourceRow)) {
        return false;
    }

    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    QHashIterator<int, FilterValue*> it(m_roleIdToFilterValueHash);
    while (it.hasNext()) {
        it.next();

        QVariant data = sourceModel()->data(index, it.key());
        FilterValue* value = it.value();

        if (!value->enabled()) {
            continue;
        }

        CompareType::Type compareType = value->compareType().value<CompareType::Type>();

        if (CompareType::Contains == compareType) {
            if (!data.toString().contains(value->roleValue().toString(), Qt::CaseInsensitive)) {
                return false;
            }
        } else if (CompareType::Equal == compareType) {
            if (data != value->roleValue()) {
                return false;
            }
        }
    }

    return true;
}

bool SortFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    SorterValue* sorter = currentSorterValue();
    if (!sorter) {
        return false;
    }

    int sorterRoleKey = roleKey(sorter->roleName());

    Val leftData = Val::fromQVariant(sourceModel()->data(left, sorterRoleKey));
    Val rightData = Val::fromQVariant(sourceModel()->data(right, sorterRoleKey));

    return leftData < rightData;
}

void SortFilterProxyModel::reset()
{
    beginResetModel();
    resetInternalData();
    endResetModel();
}

void SortFilterProxyModel::fillRoleIds()
{
    m_roleIdToFilterValueHash.clear();

    if (!sourceModel()) {
        return;
    }

    QHash<QString, FilterValue*> roleNameToValueHash;
    QList<FilterValue*> filterList = m_filters.list();
    for (FilterValue* filter: filterList) {
        roleNameToValueHash.insert(filter->roleName(), filter);
    }

    QHash<int, QByteArray> roles = sourceModel()->roleNames();
    QHash<int, QByteArray>::const_iterator it = roles.constBegin();
    while (it != roles.constEnd()) {
        if (roleNameToValueHash.contains(it.value())) {
            m_roleIdToFilterValueHash.insert(it.key(), roleNameToValueHash[it.value()]);
        }
        ++it;
    }

    setFilterFixedString(filterRegularExpression().pattern());
}

SorterValue* SortFilterProxyModel::currentSorterValue() const
{
    QList<SorterValue*> sorterList = m_sorters.list();
    for (SorterValue* sorter: sorterList) {
        if (sorter->enabled()) {
            return sorter;
        }
    }

    return nullptr;
}

int SortFilterProxyModel::roleKey(const QString& roleName) const
{
    QHash<int, QByteArray> roles = sourceModel()->roleNames();
    for (const QByteArray& roleNameByte: roles.values()) {
        if (roleName == QString(roleNameByte)) {
            return roles.key(roleNameByte);
        }
    }

    return INVALID_KEY;
}
