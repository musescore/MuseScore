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
#ifndef MU_UICOMPONENTS_SORTFILTERPROXYMODEL_H
#define MU_UICOMPONENTS_SORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

#include "filtervalue.h"
#include "sortervalue.h"
#include "qmllistproperty.h"

namespace mu::uicomponents {
class SortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)

    Q_PROPERTY(QQmlListProperty<mu::uicomponents::FilterValue> filters READ filters CONSTANT)
    Q_PROPERTY(QQmlListProperty<mu::uicomponents::SorterValue> sorters READ sorters CONSTANT)
    Q_PROPERTY(QList<int> excludeIndexes READ excludeIndexes WRITE setExcludeIndexes NOTIFY excludeIndexesChanged)

public:
    explicit SortFilterProxyModel(QObject* parent = nullptr);

    QQmlListProperty<FilterValue> filters();
    QQmlListProperty<SorterValue> sorters();

    QList<int> excludeIndexes();

    Q_INVOKABLE void refresh();

public slots:
    void setExcludeIndexes(QList<int> excludeIndexes);

signals:
    void rowCountChanged();

    void filtersChanged(QQmlListProperty<mu::uicomponents::FilterValue> filters);
    void excludeIndexesChanged(QList<int> excludeIndexes);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const;
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const;

private:
    void reset();
    void fillRoleIds();

    SorterValue* currentSorterValue() const;
    int roleKey(const QString& roleName) const;

    QmlListProperty<FilterValue> m_filters;
    QHash<int, FilterValue*> m_roleIdToFilterValueHash;
    QList<int> m_excludeIndexes;

    QmlListProperty<SorterValue> m_sorters;
};
}

#endif // MU_UICOMPONENTS_SORTFILTERPROXYMODEL_H
