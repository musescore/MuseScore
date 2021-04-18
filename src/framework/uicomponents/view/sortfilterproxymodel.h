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

    Q_PROPERTY(QQmlListProperty<mu::uicomponents::FilterValue> filters READ filters)
    Q_PROPERTY(QQmlListProperty<mu::uicomponents::SorterValue> sorters READ sorters)

public:
    explicit SortFilterProxyModel(QObject* parent = nullptr);

    QQmlListProperty<FilterValue> filters();
    QQmlListProperty<SorterValue> sorters();

    Q_INVOKABLE void refresh();

signals:
    void filtersChanged(QQmlListProperty<FilterValue> filters);

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

    QmlListProperty<SorterValue> m_sorters;
};
}

#endif // MU_UICOMPONENTS_SORTFILTERPROXYMODEL_H
