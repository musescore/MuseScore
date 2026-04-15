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

#include "sortfilterproxymodel.h"

#include <QTimer>

#include "global/types/val.h"

#include "uicomponents/view/modelutils.h"

using namespace muse::uicomponents;

static constexpr int INVALID_KEY = -1;

SortFilterProxyModel::SortFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent), m_filters(this), m_sorters(this)
{
    ModelUtils::connectRowCountChangedSignal(this, &SortFilterProxyModel::rowCountChanged);

    const auto invalidateRows = [this]() {
        beginFilterChange();
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
        endFilterChange(Direction::Rows);
#else
        invalidateFilter();
#endif
    };
    const auto onFilterChanged = [this, invalidateRows](FilterValue* changedFilterValue) {
        if (changedFilterValue->async()) {
            QTimer::singleShot(0, this, invalidateRows);
        } else {
            invalidateRows();
        }
    };

    connect(m_filters.notifier(), &QmlListPropertyNotifier::appended, this, [this, onFilterChanged](int index) {
        FilterValue* filter = m_filters.at(index);

        if (filter->enabled()) {
            onFilterChanged(filter);
        }

        connect(filter, &FilterValue::dataChanged, this, [onFilterChanged, filter] { onFilterChanged(filter); });
    });

    auto onSortersChanged = [this] {
        SorterValue* sorter = currentSorterValue();
        invalidate();

        if (!sorter) {
            return;
        }

        if (sourceModel()) {
            sort(0, sorter->sortOrder());
        }
    };

    connect(m_sorters.notifier(), &QmlListPropertyNotifier::appended, this, [this, onSortersChanged](int index) {
        onSortersChanged();

        connect(m_sorters.at(index), &SorterValue::dataChanged, this, onSortersChanged);
    });

    connect(this, &SortFilterProxyModel::sourceModelRoleNamesChanged, this, [this]() {
        invalidate();
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

    beginFilterChange();
    m_alwaysIncludeIndices = indices;
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    endFilterChange(QSortFilterProxyModel::Direction::Rows);
#else
    invalidateFilter();
#endif

    emit alwaysIncludeIndicesChanged();
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

    beginFilterChange();
    m_alwaysExcludeIndices = indices;
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    endFilterChange(QSortFilterProxyModel::Direction::Rows);
#else
    invalidateFilter();
#endif

    emit alwaysExcludeIndicesChanged();
}

QHash<int, QByteArray> SortFilterProxyModel::roleNames() const
{
    if (!sourceModel()) {
        return {};
    }

    return sourceModel()->roleNames();
}

void SortFilterProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
{
    if (m_subSourceModelConnection) {
        disconnect(m_subSourceModelConnection);
    }

    QSortFilterProxyModel::setSourceModel(sourceModel);

    emit sourceModelRoleNamesChanged();

    if (auto sourceSortFilterModel = qobject_cast<SortFilterProxyModel*>(sourceModel)) {
        m_subSourceModelConnection = connect(sourceSortFilterModel, &SortFilterProxyModel::sourceModelRoleNamesChanged,
                                             this, &SortFilterProxyModel::sourceModelRoleNamesChanged);
    }
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
    const QList<FilterValue*> filters = m_filters.list();
    for (auto* filter : filters) {
        if (!filter->enabled()) {
            continue;
        }

        const int role = roleKey(filter->roleName());
        if (role == INVALID_KEY) {
            continue;
        }

        QVariant data = sourceModel()->data(index, role);
        switch (filter->compareType()) {
        case CompareType::Equal:
            if (data != filter->roleValue()) {
                return false;
            }
            break;
        case CompareType::NotEqual:
            if (data == filter->roleValue()) {
                return false;
            }
            break;
        case CompareType::Contains:
            if (!data.toString().contains(filter->roleValue().toString(), Qt::CaseInsensitive)) {
                return false;
            }
            break;
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

SorterValue* SortFilterProxyModel::currentSorterValue() const
{
    const QList<SorterValue*> sorterList = m_sorters.list();
    for (SorterValue* sorter : sorterList) {
        if (sorter->enabled()) {
            return sorter;
        }
    }

    return nullptr;
}

int SortFilterProxyModel::roleKey(const QString& roleName) const
{
    return roleNames().key(roleName.toUtf8(), INVALID_KEY);
}
