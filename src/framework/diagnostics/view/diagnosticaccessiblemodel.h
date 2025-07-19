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
#ifndef MUSE_DIAGNOSTICS_DIAGNOSTICACCESSIBLEMODEL_H
#define MUSE_DIAGNOSTICS_DIAGNOSTICACCESSIBLEMODEL_H

#include <QAbstractItemModel>
#include <QTimer>
#include <QHash>

#include "modularity/ioc.h"
#include "accessibility/iaccessibilitycontroller.h"

#include "async/asyncable.h"

class QAccessibleInterface;
class QAccessibleEvent;
namespace muse::diagnostics {
class DiagnosticAccessibleModel : public QAbstractItemModel, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT
    Q_PROPERTY(bool isAutoRefresh READ isAutoRefresh WRITE setIsAutoRefresh NOTIFY isAutoRefreshChanged)

    muse::Inject<muse::accessibility::IAccessibilityController> accessibilityController = { this };

public:
    explicit DiagnosticAccessibleModel(QObject* parent = nullptr);
    ~DiagnosticAccessibleModel();

    bool isAutoRefresh() const;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void init();
    Q_INVOKABLE void reload();

    void dumpTree();

public slots:
    void setIsAutoRefresh(bool isAutoRefresh);

signals:
    void beforeReload();
    void afterReload();
    void focusedItem(QModelIndex index);

    void isAutoRefreshChanged();

private:

    enum Roles {
        rItemData = Qt::UserRole + 1
    };

    struct Item
    {
        quintptr key() const { return reinterpret_cast<quintptr>(this); }

        int row() const
        {
            if (m_parent) {
                return m_parent->m_children.indexOf(const_cast<Item*>(this));
            }
            return 0;
        }

        Item* parent() const { return m_parent; }

        void addChild(Item* child) { m_children.append(child); }
        Item* child(int row) const { return m_children.at(row); }
        int childCount() const { return m_children.count(); }

        void setData(const QVariant& d) { m_data = d; }
        QVariant data() const { return m_data; }

        void setIface(QAccessibleInterface* iface) { m_iface = iface; }
        QAccessibleInterface* iface() const { return m_iface; }

        Item(Item* parent)
            : m_parent(parent)
        {
            if (m_parent) {
                m_parent->addChild(this);
            }
        }

        ~Item()
        {
            for (Item* item : m_children) {
                delete item;
            }
        }

    private:
        Item* m_parent = nullptr;
        QList<Item*> m_children;
        QVariant m_data;
        QAccessibleInterface* m_iface = nullptr;
    };

    Item* createItem(Item* parent);
    Item* itemByModelIndex(const QModelIndex& index) const;

    void load(QAccessibleInterface* iface, Item* parent);
    QVariant makeData(const QAccessibleInterface* iface) const;

    void onAccessibleEvent(QAccessibleEvent* ev);
    void onItemChanged(QObject* accessibleObject);
    Item* findItemForIface(const QAccessibleInterface* iface, Item* rootItem) const;

    QObject* m_accessibleRootObject;
    Item* m_rootItem = nullptr;
    QTimer m_refresher;
    bool m_isAutoRefresh = false;
    QHash<quintptr, Item*> m_allItems;
};
}

#endif // MUSE_DIAGNOSTICS_DIAGNOSTICACCESSIBLEMODEL_H
