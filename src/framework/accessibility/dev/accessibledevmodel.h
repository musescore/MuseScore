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
#ifndef MU_ACCESSIBILITY_ACCESSIBLEDEVMODEL_H
#define MU_ACCESSIBILITY_ACCESSIBLEDEVMODEL_H

#include <QAbstractListModel>
#include <QTimer>

class QAccessibleInterface;
namespace mu::accessibility {
class AccessibleDevModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool isAutoRefresh READ isAutoRefresh WRITE setIsAutoRefresh NOTIFY isAutoRefreshChanged)

public:
    explicit AccessibleDevModel(QObject* parent = nullptr);

    bool isAutoRefresh() const;

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void reload();

    static void setRootObject(QObject* object);

public slots:
    void setIsAutoRefresh(bool isAutoRefresh);

signals:
    void beforeReload();
    void afterReload();

    void isAutoRefreshChanged();

private:

    static QObject* m_rootObject;

    enum Roles {
        rItemData = Qt::UserRole + 1
    };

    void load(QAccessibleInterface* iface, int& level);

    QVariantList m_items;
    QTimer m_refresher;
    bool m_isAutoRefresh = false;
};
}

#endif // MU_ACCESSIBILITY_ACCESSIBLEDEVMODEL_H
