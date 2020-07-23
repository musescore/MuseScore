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

using namespace mu::framework;

FilterProxyModel::FilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
}

QObject* FilterProxyModel::sourceModel_property() const
{
    return QSortFilterProxyModel::sourceModel();
}

QStringList FilterProxyModel::searchRoles() const
{
    return m_searchRoles;
}

QString FilterProxyModel::searchString() const
{
    return filterRegExp().pattern();
}

QStringList FilterProxyModel::filterRoles() const
{
    return m_filterRoles;
}

QVariantList FilterProxyModel::filterValues() const
{
    return m_filterValues;
}

void FilterProxyModel::setSourceModel_property(QObject* source)
{
    QAbstractItemModel* model = dynamic_cast<QAbstractItemModel*>(source);

    if (!model) {
        return;
    }

    setSourceModel(model);

    if (!m_searchRoles.isEmpty() && model) {
        fillSearchRoleIds();
    }
}

void FilterProxyModel::setSearchRoles(const QStringList& names)
{
    if (m_searchRoles == names) {
        return;
    }

    m_searchRoleIds.clear();
    m_searchRoles = names;
    emit searchRolesChanged();

    if (sourceModel()) {
        fillSearchRoleIds();
    }
}

void FilterProxyModel::setSearchString(const QString& filter)
{
    if (searchString() == filter) {
        return;
    }

    setFilterFixedString(filter);
    reset();

    emit searchStringChanged();
}

void FilterProxyModel::setFilterRoles(const QStringList& filterRoles)
{
    if (m_filterRoles == filterRoles) {
        return;
    }

    m_filterRoleIds.clear();
    m_filterRoles = filterRoles;
    emit filterRolesChanged(filterRoles);

    if (sourceModel()) {
        fillFilterRoleIds();
    }
}

void FilterProxyModel::setFilterValues(const QVariantList& filterValues)
{
    if (m_filterValues == filterValues) {
        return;
    }

    m_filterValues = filterValues;
    emit filterValuesChanged(m_filterValues);
}

bool FilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    bool ok = allowedByFilters(index);
    ok &= allowedBySearch(index);

    return ok;
}

void FilterProxyModel::reset()
{
    beginResetModel();
    resetInternalData();
    endResetModel();
}

void FilterProxyModel::fillSearchRoleIds()
{
    m_searchRoleIds.clear();

    if (!sourceModel()) {
        return;
    }

    QHash<int, QByteArray> roles = sourceModel()->roleNames();
    QHash<int, QByteArray>::const_iterator i = roles.constBegin();
    while (i != roles.constEnd()) {
        if (m_searchRoles.contains(i.value())) {
            m_searchRoleIds.append(i.key());
        }
        ++i;
    }

    setFilterFixedString(filterRegExp().pattern());
}

void FilterProxyModel::fillFilterRoleIds()
{
    m_filterRoleIds.clear();

    if (!sourceModel()) {
        return;
    }

    QHash<int, QByteArray> roles = sourceModel()->roleNames();
    QHash<int, QByteArray>::const_iterator i = roles.constBegin();
    while (i != roles.constEnd()) {
        if (m_filterRoles.contains(i.value())) {
            m_filterRoleIds.append(i.key());
        }
        ++i;
    }

    setFilterFixedString(filterRegExp().pattern());
}

bool FilterProxyModel::allowedByFilters(const QModelIndex& index) const
{
    if (m_filterRoleIds.isEmpty()) {
        return true;
    }

    for (int i = 0; i < m_filterRoleIds.count(); ++i) {
        QVariant data = sourceModel()->data(index, m_filterRoleIds.at(i));

        if (data != m_filterValues[i]) {
            return false;
        }
    }

    return true;
}

bool FilterProxyModel::allowedBySearch(const QModelIndex& index) const
{
    QString filter = searchString();
    if (filter.isEmpty()) {
        return true;
    }

    for (int i = 0; i < m_searchRoleIds.count(); ++i) {
        QVariant data = sourceModel()->data(index, m_searchRoleIds.at(i));

        if (data.toString().contains(filter, Qt::CaseInsensitive)) {
            return true;
        }
    }

    return false;
}
