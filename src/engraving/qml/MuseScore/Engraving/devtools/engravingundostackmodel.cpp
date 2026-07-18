/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore Studio
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

#include "engravingundostackmodel.h"

#include "notation/inotation.h"

#include "engraving/dom/score.h"
#include "engraving/editing/transaction/undoablecommand.h"
#include "engraving/editing/transaction/undostack.h"

#include "notation/inotationelements.h" // IWYU pragma: keep
#include "notation/inotationundostack.h" // IWYU pragma: keep

#include "log.h"

using namespace mu::engraving;
using namespace mu::notation;

EngravingUndoStackModel::EngravingUndoStackModel(QObject* parent)
    : QAbstractItemModel(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void EngravingUndoStackModel::init()
{
    onNotationChanged();

    context()->currentNotationChanged().onNotify(this, [this] { onNotationChanged(); });
}

void EngravingUndoStackModel::onNotationChanged()
{
    reload();

    if (INotationPtr notation = context()->currentNotation()) {
        notation->undoStack()->stackChanged().onNotify(this, [this] { reload(); });
    }
}

void EngravingUndoStackModel::reload()
{
    TRACEFUNC;

    beginResetModel();

    m_allItems.clear();
    delete m_rootItem;
    m_rootItem = new Item(nullptr);
    m_allItems.insert(m_rootItem->key(), m_rootItem);

    if (INotationPtr notation = context()->currentNotation()) {
        const UndoStack* undoStack = notation->elements()->msScore()->undoStack();

        for (size_t i = 1; i <= undoStack->size(); ++i) {
            const UndoableTransaction* transaction = undoStack->lastAtIndex(i);
            Item* item = createItem(m_rootItem, transaction, transaction == undoStack->last());
            load(transaction, item);
        }
    }

    endResetModel();
}

void EngravingUndoStackModel::load(const UndoableTransaction* transaction, Item* parent)
{
    TRACEFUNC;

    for (const UndoableCommand* childCommand : transaction->commands()) {
        createItem(parent, childCommand);
    }
}

QModelIndex EngravingUndoStackModel::index(int row, int column, const QModelIndex& parent) const
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

QModelIndex EngravingUndoStackModel::parent(const QModelIndex& child) const
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

int EngravingUndoStackModel::rowCount(const QModelIndex& parent) const
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

int EngravingUndoStackModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant EngravingUndoStackModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != rItemData) {
        return QVariant();
    }

    Item* item = itemByModelIndex(index);
    if (!item) {
        return QVariant();
    }

    return item->data();
}

QHash<int, QByteArray> EngravingUndoStackModel::roleNames() const
{
    return { { rItemData, "itemData" } };
}

/// Generates a seemingly random but stable color based on a pointer address.
/// (If we would use really random colors, the would change on every redraw.)
static QColor colorForPointer(const void* ptr)
{
    static constexpr uint32_t rgbTotalMax = 600;
    static constexpr uint32_t mask = 0xFFFF;
    static constexpr double max = mask + 1.0;

    auto hash = std::hash<const void*> {}(ptr);
    uint32_t cmpnt1 = hash & mask;
    uint32_t cmpnt2 = (hash >> 16) & mask;

    uint32_t r = std::clamp(uint32_t(cmpnt1 / max * 255), 30u, 255u);
    uint32_t g = std::clamp(uint32_t(cmpnt2 / max * 255), 30u, 255u);
    uint32_t b = std::clamp(rgbTotalMax - r - g, 30u, 255u);

    return QColor::fromRgb(r, g, b, 128);
}

EngravingUndoStackModel::Item* EngravingUndoStackModel::createItem(
    Item* parent, const UndoableTransaction* transaction, bool isCurrent)
{
    Item* item = new Item(parent);
    m_allItems.insert(item->key(), item);

    if (transaction) {
        QVariantMap data;
        data["text"] = transaction->actionName().qTranslated();
        data["color"] = colorForPointer(transaction);
        data["isCurrent"] = isCurrent;

        item->setData(data);
    }

    return item;
}

EngravingUndoStackModel::Item* EngravingUndoStackModel::createItem(
    Item* parent, const UndoableCommand* command, bool isCurrent)
{
    Item* item = new Item(parent);
    m_allItems.insert(item->key(), item);

    if (command) {
        QVariantMap data;
        data["text"] = QString(command->name());
        data["color"] = colorForPointer(command);
        data["isCurrent"] = isCurrent;

        item->setData(data);
    }

    return item;
}

EngravingUndoStackModel::Item* EngravingUndoStackModel::itemByModelIndex(const QModelIndex& index) const
{
    return m_allItems.value(index.internalId(), nullptr);
}

// Item ===============
EngravingUndoStackModel::Item::Item(Item* parent)
    : m_parent(parent)
{
    if (m_parent) {
        m_parent->addChild(this);
    }
}

EngravingUndoStackModel::Item::~Item()
{
    for (Item* item : m_children) {
        delete item;
    }
}

int EngravingUndoStackModel::Item::row() const
{
    if (m_parent) {
        return m_parent->m_children.indexOf(const_cast<Item*>(this));
    }
    return 0;
}
