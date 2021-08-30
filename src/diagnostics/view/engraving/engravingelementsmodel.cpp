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
#include "engravingelementsmodel.h"

#include <QTextStream>

#include "engraving/libmscore/scoreElement.h"
#include "engraving/libmscore/score.h"

using namespace mu::diagnostics;

EngravingElementsModel::EngravingElementsModel(QObject* parent)
    : QAbstractItemModel(parent)
{
}

QModelIndex EngravingElementsModel::index(int row, int column, const QModelIndex& parent) const
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

QModelIndex EngravingElementsModel::parent(const QModelIndex& child) const
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

int EngravingElementsModel::rowCount(const QModelIndex& parent) const
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

int EngravingElementsModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant EngravingElementsModel::data(const QModelIndex& index, int role) const
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

QHash<int, QByteArray> EngravingElementsModel::roleNames() const
{
    return { { rItemData, "itemData" } };
}

EngravingElementsModel::Item* EngravingElementsModel::createItem(Item* parent)
{
    Item* item = new Item(parent);
    m_allItems.insert(item->key(), item);
    return item;
}

EngravingElementsModel::Item* EngravingElementsModel::itemByModelIndex(const QModelIndex& index) const
{
    return m_allItems.value(index.internalId(), nullptr);
}

QVariant EngravingElementsModel::makeData(const Ms::ScoreElement* el) const
{
    if (!el) {
        return QVariant();
    }

    QVariantMap d;
    d["name"] = el->name();
    d["selected"] = elementsProvider()->isSelected(el);
    return d;
}

void EngravingElementsModel::init()
{
}

void EngravingElementsModel::reload()
{
    beginResetModel();

    m_allItems.clear();
    delete m_rootItem;
    m_rootItem = createItem(nullptr);

    std::list<const Ms::ScoreElement*> elements = elementsProvider()->elements();
    for (const Ms::ScoreElement* el : elements) {
        if (el->isScore()) {
            Item* scoreItem = createItem(m_rootItem);
            scoreItem->setElement(el);
            scoreItem->setData(makeData(el));
            load(elements, scoreItem);
        }
    }

    Item* lossItem = createItem(m_rootItem);
    QVariantMap lossData;
    lossData["name"] = "Loss";
    lossItem->setData(lossData);
    findAndAddLoss(elements, lossItem, m_rootItem);

    endResetModel();

    updateInfo();
}

void EngravingElementsModel::load(const std::list<const Ms::ScoreElement*>& elements, Item* root)
{
    for (const Ms::ScoreElement* el : elements) {
        if (el->treeParent() == root->element()) {
            Item* item = createItem(root);
            item->setElement(el);
            item->setData(makeData(el));
            load(elements, item);
        }
    }
}

const EngravingElementsModel::Item* EngravingElementsModel::findItem(const Ms::ScoreElement* el, const Item* root) const
{
    if (root->element() == el) {
        return root;
    }

    for (int i = 0; i < root->childCount(); ++i) {
        const Item* ch = findItem(el, root->child(i));
        if (ch) {
            return ch;
        }
    }
    return nullptr;
}

void EngravingElementsModel::findAndAddLoss(const std::list<const Ms::ScoreElement*>& elements, Item* lossRoot, const Item* root)
{
    for (const Ms::ScoreElement* el : elements) {
        const Item* it = findItem(el, root);
        if (it) {
            continue;
        }

        Item* item = createItem(lossRoot);
        item->setElement(el);
        item->setData(makeData(el));
    }
}

void EngravingElementsModel::select(QModelIndex index, bool arg)
{
    Item* item = itemByModelIndex(index);
    if (!item) {
        return;
    }

    elementsProvider()->select(item->element(), arg);
    item->setData(makeData(item->element()));

    emit dataChanged(index, index, { rItemData });

    dispatcher()->dispatch("diagnostic-notationview-redraw");
}

void EngravingElementsModel::updateInfo()
{
    std::list<const Ms::ScoreElement*> elements = elementsProvider()->elements();
    QHash<QString, int> els;
    for (const Ms::ScoreElement* el : elements) {
        els[el->name()] += 1;
    }

    {
        m_info.clear();
        QTextStream stream(&m_info);
        for (auto it = els.constBegin(); it != els.constEnd(); ++it) {
            stream << it.key() << ": " << it.value() << "\n";
        }
    }

    {
        m_summary.clear();
        QTextStream stream(&m_summary);
        stream << "Total: " << elements.size();
    }

    emit infoChanged();
    emit summaryChanged();
}

QString EngravingElementsModel::info() const
{
    return m_info;
}

QString EngravingElementsModel::summary() const
{
    return m_summary;
}

// Item ===============
EngravingElementsModel::Item::Item(Item* parent)
    : m_parent(parent)
{
    if (m_parent) {
        m_parent->addChild(this);
    }
}

EngravingElementsModel::Item::~Item()
{
    for (Item* item : m_children) {
        delete item;
    }
}

int EngravingElementsModel::Item::row() const
{
    if (m_parent) {
        return m_parent->m_children.indexOf(const_cast<Item*>(this));
    }
    return 0;
}
