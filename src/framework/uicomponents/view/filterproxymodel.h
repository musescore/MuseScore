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
#ifndef MU_FRAMEWORK_FILTERPROXYMODEL_H
#define MU_FRAMEWORK_FILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

#include "filtervalue.h"
#include "qmllistproperty.h"

namespace mu {
namespace framework {
class FilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QQmlListProperty<mu::framework::FilterValue> filters READ filters)

public:
    explicit FilterProxyModel(QObject* parent = nullptr);

    QQmlListProperty<FilterValue> filters();

    Q_INVOKABLE void refresh();

signals:
    void filtersChanged(QQmlListProperty<FilterValue> filters);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const;

private:
    void reset();
    void fillRoleIds();

    QmlListProperty<FilterValue> m_filters;
    QHash<int, FilterValue*> m_roleIdToValueHash;
};
}
}

#endif // MU_FRAMEWORK_FILTERPROXYMODEL_H
