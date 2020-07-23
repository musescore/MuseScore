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

namespace mu {
namespace framework {
class FilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QObject* sourceModel READ sourceModel_property WRITE setSourceModel_property)

    Q_PROPERTY(QStringList searchRoles READ searchRoles WRITE setSearchRoles NOTIFY searchRolesChanged)
    Q_PROPERTY(QString searchString READ searchString WRITE setSearchString NOTIFY searchStringChanged)

    Q_PROPERTY(QStringList filterRoles READ filterRoles WRITE setFilterRoles NOTIFY filterRolesChanged)
    Q_PROPERTY(QVariantList filterValues READ filterValues WRITE setFilterValues NOTIFY filterValuesChanged)

public:
    explicit FilterProxyModel(QObject* parent = nullptr);

    QObject* sourceModel_property() const;

    QStringList searchRoles() const;
    QString searchString() const;

    QStringList filterRoles() const;
    QVariantList filterValues() const;

public slots:
    void setSourceModel_property(QObject* model);

    void setSearchRoles(const QStringList& names);
    void setSearchString(const QString& filter);

    void setFilterRoles(const QStringList& filterRoles);
    void setFilterValues(const QVariantList& filterValues);

signals:
    void searchRolesChanged() const;
    void searchStringChanged() const;

    void filterRolesChanged(QStringList filterRoles);
    void filterValuesChanged(QVariantList filterRoleValue);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const;

private:
    void reset();

    void fillSearchRoleIds();
    void fillFilterRoleIds();

    bool allowedByFilters(const QModelIndex& index) const;
    bool allowedBySearch(const QModelIndex& index) const;

    QStringList m_searchRoles;
    QList<int> m_searchRoleIds;

    QStringList m_filterRoles;
    QList<int> m_filterRoleIds;
    QVariantList m_filterValues;
};
}
}

#endif // MU_FRAMEWORK_FILTERPROXYMODEL_H
