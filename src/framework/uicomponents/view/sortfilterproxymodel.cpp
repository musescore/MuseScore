//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "sortfilterproxymodel.h"

#include "val.h"

using namespace mu::uicomponents;

static const int INVALID_KEY = -1;

SortFilterProxyModel::SortFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent), m_filters(this), m_sorters(this)
{
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

void SortFilterProxyModel::refresh()
{
    setFilterFixedString(filterRegExp().pattern());
    setSortCaseSensitivity(sortCaseSensitivity());
}

bool SortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    QHashIterator<int, FilterValue*> it(m_roleIdToFilterValueHash);
    while (it.hasNext()) {
        it.next();

        QVariant data = sourceModel()->data(index, it.key());
        FilterValue* value = it.value();

        if (!value->enabled()) {
            continue;
        }

        CompareType::Type compreType = value->compareType().value<CompareType::Type>();

        if (CompareType::Contains == compreType) {
            if (!data.toString().contains(value->roleValue().toString(), Qt::CaseInsensitive)) {
                return false;
            }
        } else if (CompareType::Equal == compreType) {
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

    setFilterFixedString(filterRegExp().pattern());
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
