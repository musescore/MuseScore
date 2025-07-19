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
#include "diagnosticaccessiblemodel.h"

#include <QAccessible>
#include <QMetaEnum>

#include "muse_framework_config.h"
#include "accessibility/internal/accessibilitycontroller.h"

#include "log.h"

using namespace muse::diagnostics;
using namespace muse::accessibility;

DiagnosticAccessibleModel::DiagnosticAccessibleModel(QObject* parent)
    : QAbstractItemModel(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
    m_refresher.setInterval(1000);
    m_refresher.setSingleShot(false);
    connect(&m_refresher, &QTimer::timeout, [this]() {
        reload();
    });
}

DiagnosticAccessibleModel::~DiagnosticAccessibleModel()
{
    delete m_rootItem;
}

void DiagnosticAccessibleModel::init()
{
#ifdef MUSE_MODULE_ACCESSIBILITY
    AccessibilityController* accessController = dynamic_cast<AccessibilityController*>(accessibilityController().get());
    AccessibilityController::Item rootItem = accessController->findItem(accessController);
    m_accessibleRootObject = rootItem.object;

    accessController->eventSent().onReceive(this, [this](QAccessibleEvent* ev) { onAccessibleEvent(ev); });
#endif
}

#ifdef MUSE_MODULE_ACCESSIBILITY
static void debug_dumpItem(QAccessibleInterface* item, QTextStream& stream, QString& level)
{
    QAccessibleInterface* prn = item->parent();
    stream << level
           << item->text(QAccessible::Name)
           << " parent: " << (prn ? prn->text(QAccessible::Name) : QString("null"))
           << " childCount: " << item->childCount()
           << Qt::endl;

    QAccessible::State st = item->state();
    stream << level
           << "state:"
           << " invisible: " << st.invisible
           << " invalid: " << st.invalid
           << " disabled: " << st.disabled
           << " active: " << st.active
           << " focusable: " << st.focusable
           << " focused: " << st.focused
           << Qt::endl;

    level += "  ";
    int count = item->childCount();
    for (int i = 0; i < count; ++i) {
        QAccessibleInterface* ch = item->child(i);
        debug_dumpItem(ch, stream, level);
    }
    level.chop(2);
}

static void debug_dumpTree(QAccessibleInterface* item)
{
    QString str;
    QTextStream stream(&str);
    QString level;

    debug_dumpItem(item, stream, level);

    stream.flush();
    LOGI() << "================================================\n";
    std::cout << str.toStdString() << '\n';
}

#endif

void DiagnosticAccessibleModel::dumpTree()
{
#ifdef MUSE_MODULE_ACCESSIBILITY
    AccessibilityController* accessController = dynamic_cast<AccessibilityController*>(accessibilityController().get());
    AccessibilityController::Item rootItem = accessController->findItem(accessController);

    debug_dumpTree(rootItem.iface);
#endif
}

QModelIndex DiagnosticAccessibleModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!m_rootItem) {
        return QModelIndex();
    }

    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    Item* parentItem = nullptr;

    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = itemByModelIndex(parent);
    }

    if (!parentItem) {
        return QModelIndex();
    }

    Item* childItem = parentItem->child(row);
    if (childItem) {
        return createIndex(row, column, childItem->key());
    }

    return QModelIndex();
}

QModelIndex DiagnosticAccessibleModel::parent(const QModelIndex& child) const
{
    if (!m_rootItem) {
        return QModelIndex();
    }

    if (!child.isValid()) {
        return QModelIndex();
    }

    Item* childItem = itemByModelIndex(child);
    if (!childItem) {
        return QModelIndex();
    }

    Item* parentItem = childItem->parent();
    if (parentItem == m_rootItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem->key());
}

int DiagnosticAccessibleModel::rowCount(const QModelIndex& parent) const
{
    if (!m_rootItem) {
        return 0;
    }

    Item* parentItem = nullptr;

    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = itemByModelIndex(parent);
    }

    if (!parentItem) {
        return 0;
    }

    return parentItem->childCount();
}

int DiagnosticAccessibleModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant DiagnosticAccessibleModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() && role != rItemData) {
        return QVariant();
    }

    Item* item = itemByModelIndex(index);
    if (!item) {
        return QVariant();
    }

    return item->data();
}

QHash<int, QByteArray> DiagnosticAccessibleModel::roleNames() const
{
    return { { rItemData, "itemData" } };
}

void DiagnosticAccessibleModel::onAccessibleEvent(QAccessibleEvent* ev)
{
    LOGD() << "event: " << ev->type();
    if (ev->type() == QAccessible::ObjectCreated || ev->type() == QAccessible::ObjectDestroyed) {
        // todo
    } else {
        onItemChanged(ev->object());
    }
}

void DiagnosticAccessibleModel::onItemChanged(QObject* accessibleObject)
{
    if (!m_rootItem) {
        return;
    }

    QAccessibleInterface* iface = QAccessible::queryAccessibleInterface(accessibleObject);
    IF_ASSERT_FAILED(iface) {
        return;
    }

    Item* item = findItemForIface(iface, m_rootItem);
    if (!item) {
        LOGE() << "not found item: " << iface->text(QAccessible::Name);
        return;
    }

    QVariant newData = makeData(iface);
    item->setData(newData);

    QModelIndex index = createIndex(item->row(), 0, item->key());

    QAccessible::State st = iface->state();
    if (st.focused) {
        emit focusedItem(index);
    }

    emit dataChanged(index, index, { rItemData });
}

DiagnosticAccessibleModel::Item* DiagnosticAccessibleModel::findItemForIface(const QAccessibleInterface* iface, Item* rootItem) const
{
    if (!rootItem) {
        return nullptr;
    }

    if (rootItem->iface() == iface) {
        return rootItem;
    }

    for (int i = 0; i < rootItem->childCount(); ++i) {
        Item* it = findItemForIface(iface, rootItem->child(i));
        if (it) {
            return it;
        }
    }

    return nullptr;
}

DiagnosticAccessibleModel::Item* DiagnosticAccessibleModel::createItem(Item* parent)
{
    Item* item = new Item(parent);
    m_allItems.insert(item->key(), item);
    return item;
}

DiagnosticAccessibleModel::Item* DiagnosticAccessibleModel::itemByModelIndex(const QModelIndex& index) const
{
    return m_allItems.value(index.internalId(), nullptr);
}

void DiagnosticAccessibleModel::reload()
{
    IF_ASSERT_FAILED(m_accessibleRootObject) {
        return;
    }

    QAccessibleInterface* iface = QAccessible::queryAccessibleInterface(m_accessibleRootObject);
    IF_ASSERT_FAILED(iface) {
        return;
    }

    m_refresher.stop();

    emit beforeReload();
    beginResetModel();

    m_allItems.clear();
    delete m_rootItem;
    m_rootItem = createItem(nullptr);
    load(iface, m_rootItem);

    endResetModel();
    emit afterReload();

    if (m_isAutoRefresh) {
        m_refresher.start();
    }
}

QVariant DiagnosticAccessibleModel::makeData(const QAccessibleInterface* iface) const
{
    QAccessibleInterface* prn = iface->parent();

    static const QMetaEnum roleEnum = QMetaEnum::fromType<QAccessible::Role>();

    QVariantMap itemData;
    itemData["role"] = QString(roleEnum.valueToKey(iface->role()));
    itemData["name"] = iface->text(QAccessible::Name);
    itemData["parent"] = prn ? prn->text(QAccessible::Name) : QString("null");
    itemData["children"] = iface->childCount();

    QAccessible::State st = iface->state();
    QVariantMap state;
    state["invisible"] = st.invisible;
    state["invalid"] = st.invalid;
    state["disabled"] = st.disabled;
    state["active"] = st.active;
    state["focusable"] = st.focusable;
    state["focused"] = st.focused;
    itemData["state"] = state;

    return itemData;
}

void DiagnosticAccessibleModel::load(QAccessibleInterface* iface, Item* parent)
{
    Item* item = createItem(parent);
    item->setIface(iface);
    item->setData(makeData(iface));

    int childCount = iface->childCount();
    for (int i = 0; i < childCount; ++i) {
        QAccessibleInterface* child = iface->child(i);
        load(child, item);
    }
}

void DiagnosticAccessibleModel::setIsAutoRefresh(bool isAutoRefresh)
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

bool DiagnosticAccessibleModel::isAutoRefresh() const
{
    return m_isAutoRefresh;
}
