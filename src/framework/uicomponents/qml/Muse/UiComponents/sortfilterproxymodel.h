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

#pragma once

#include <QByteArray>
#include <QHash>
#include <QList>
#include <QMetaObject>
#include <QSortFilterProxyModel>

#include <QtQmlIntegration/qqmlintegration.h>

#include "filter.h"
#include "qmllistproperty.h"
#include "sorter.h"

namespace muse::uicomponents {
class SortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)
    Q_PROPERTY(QQmlListProperty<muse::uicomponents::Filter> filters READ filters CONSTANT)
    Q_PROPERTY(QQmlListProperty<muse::uicomponents::Sorter> sorters READ sorters CONSTANT)
    Q_PROPERTY(QList<int> alwaysIncludeIndices READ alwaysIncludeIndices WRITE setAlwaysIncludeIndices NOTIFY alwaysIncludeIndicesChanged)
    Q_PROPERTY(QList<int> alwaysExcludeIndices READ alwaysExcludeIndices WRITE setAlwaysExcludeIndices NOTIFY alwaysExcludeIndicesChanged)

    QML_ELEMENT

public:
    explicit SortFilterProxyModel(QObject* parent = nullptr);

    QQmlListProperty<Filter> filters();
    QQmlListProperty<Sorter> sorters();

    QList<int> alwaysIncludeIndices() const;
    void setAlwaysIncludeIndices(const QList<int>& indices);

    QList<int> alwaysExcludeIndices() const;
    void setAlwaysExcludeIndices(const QList<int>& indices);

    int roleFromRoleName(const QString&) const;
    QHash<int, QByteArray> roleNames() const override;

    void setSourceModel(QAbstractItemModel* sourceModel) override;

signals:
    void rowCountChanged();

    void alwaysIncludeIndicesChanged();
    void alwaysExcludeIndicesChanged();

    void sourceModelRoleNamesChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
    Sorter* currentSorter() const;
    void invalidateFilters() const;
    void updateRoleMap();

    QHash<QByteArray, int> m_roles;

    QmlListProperty<Filter> m_filters;
    QmlListProperty<Sorter> m_sorters;

    QList<int> m_alwaysIncludeIndices;
    QList<int> m_alwaysExcludeIndices;

    QMetaObject::Connection m_subSourceModelConnection;
    QMetaObject::Connection m_sourceDataChangedConnection;
    QMetaObject::Connection m_sourceModelAboutToBeResetConnection;
};
}
