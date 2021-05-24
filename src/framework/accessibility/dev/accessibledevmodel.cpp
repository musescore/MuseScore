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
#include "accessibledevmodel.h"

#include <QAccessible>

#include "log.h"

using namespace mu::accessibility;

QObject* AccessibleDevModel::m_rootObject = nullptr;

AccessibleDevModel::AccessibleDevModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_refresher.setInterval(500);
    m_refresher.setSingleShot(false);
    connect(&m_refresher, &QTimer::timeout, [this]() {
        reload();
    });
}

void AccessibleDevModel::setRootObject(QObject* object)
{
    m_rootObject = object;
}

QVariant AccessibleDevModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    switch (role) {
    case rItemData: return m_items[index.row()];
    }

    return QVariant();
}

int AccessibleDevModel::rowCount(const QModelIndex&) const
{
    return m_items.count();
}

QHash<int, QByteArray> AccessibleDevModel::roleNames() const
{
    return { { rItemData, "itemData" } };
}

void AccessibleDevModel::reload()
{
    IF_ASSERT_FAILED(m_rootObject) {
        return;
    }

    QAccessibleInterface* iface = QAccessible::queryAccessibleInterface(m_rootObject);
    IF_ASSERT_FAILED(m_rootObject) {
        return;
    }

    m_refresher.stop();

    emit beforeReload();
    beginResetModel();

    m_items.clear();

    int level = 0;
    load(iface, level);

    endResetModel();
    emit afterReload();

    if (m_isAutoRefresh) {
        m_refresher.start();
    }
}

void AccessibleDevModel::load(QAccessibleInterface* iface, int& level)
{
    QAccessibleInterface* prn = iface->parent();
    int childCount = iface->childCount();

    QVariantMap item;
    item["name"] = iface->text(QAccessible::Name);
    item["level"] = level;
    item["parent"] = prn ? prn->text(QAccessible::Name) : QString("null");
    item["children"] = iface->childCount();

    QAccessible::State st = iface->state();
    QVariantMap state;
    state["invisible"] = st.invisible;
    state["invalid"] = st.invalid;
    state["disabled"] = st.disabled;
    state["active"] = st.active;
    state["focusable"] = st.focusable;
    state["focused"] = st.focused;
    item["state"] = state;

    m_items << item;

    ++level;
    for (int i = 0; i < childCount; ++i) {
        QAccessibleInterface* child = iface->child(i);
        load(child, level);
    }
    --level;
}

void AccessibleDevModel::setIsAutoRefresh(bool isAutoRefresh)
{
    if (m_isAutoRefresh == isAutoRefresh) {
        return;
    }

    m_isAutoRefresh = isAutoRefresh;
    emit isAutoRefreshChanged();

    if (isAutoRefresh) {
        m_refresher.start();
    }
}

bool AccessibleDevModel::isAutoRefresh() const
{
    return m_isAutoRefresh;
}
