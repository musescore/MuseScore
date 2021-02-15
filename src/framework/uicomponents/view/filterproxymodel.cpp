//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
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
#include "filterproxymodel.h"

using namespace mu::uicomponents;

FilterProxyModel::FilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent), m_filters(this)
{
    connect(m_filters.notifier(), &QmlListPropertyNotifier::appended, this, [this](int index) {
        connect(m_filters.at(index), &FilterValue::dataChanged, this, [this]() {
            if (sourceModel()) {
                fillRoleIds();
            }
        });
    });
}

QQmlListProperty<FilterValue> FilterProxyModel::filters()
{
    return m_filters.property();
}

void FilterProxyModel::refresh()
{
    setFilterFixedString(filterRegExp().pattern());
}

bool FilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    QHashIterator<int, FilterValue*> it(m_roleIdToValueHash);
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

void FilterProxyModel::reset()
{
    beginResetModel();
    resetInternalData();
    endResetModel();
}

void FilterProxyModel::fillRoleIds()
{
    m_roleIdToValueHash.clear();

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
            m_roleIdToValueHash.insert(it.key(), roleNameToValueHash[it.value()]);
        }
        ++it;
    }

    setFilterFixedString(filterRegExp().pattern());
}
