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
#include "diagnosticengravingelementsmodel.h"

#include <QTextStream>

#include "engraving/libmscore/scoreElement.h"
#include "engraving/libmscore/score.h"

using namespace mu::diagnostics;

DiagnosticEngravingElementsModel::DiagnosticEngravingElementsModel(QObject* parent)
    : QAbstractItemModel(parent)
{
}

QModelIndex DiagnosticEngravingElementsModel::index(int row, int column, const QModelIndex& parent) const
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

QModelIndex DiagnosticEngravingElementsModel::parent(const QModelIndex& child) const
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

int DiagnosticEngravingElementsModel::rowCount(const QModelIndex& parent) const
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

int DiagnosticEngravingElementsModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant DiagnosticEngravingElementsModel::data(const QModelIndex& index, int role) const
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

QHash<int, QByteArray> DiagnosticEngravingElementsModel::roleNames() const
{
    return { { rItemData, "itemData" } };
}

DiagnosticEngravingElementsModel::Item* DiagnosticEngravingElementsModel::createItem(Item* parent)
{
    Item* item = new Item(parent);
    m_allItems.insert(item->key(), item);
    return item;
}

DiagnosticEngravingElementsModel::Item* DiagnosticEngravingElementsModel::itemByModelIndex(const QModelIndex& index) const
{
    return m_allItems.value(index.internalId(), nullptr);
}

QVariant DiagnosticEngravingElementsModel::makeData(const Ms::ScoreElement* el) const
{
    if (!el) {
        return QVariant();
    }

    QVariantMap d;
    d["name"] = el->name();
    return d;
}

void DiagnosticEngravingElementsModel::init()
{
}

void DiagnosticEngravingElementsModel::reload()
{
    beginResetModel();

    m_allItems.clear();
    delete m_rootItem;
    m_rootItem = createItem(nullptr);

    std::list<const Ms::ScoreElement*> elements = engravingRegister()->elements();
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

void DiagnosticEngravingElementsModel::load(const std::list<const Ms::ScoreElement*>& elements, Item* root)
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

const DiagnosticEngravingElementsModel::Item* DiagnosticEngravingElementsModel::findItem(const Ms::ScoreElement* el, const Item* root) const
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

void DiagnosticEngravingElementsModel::findAndAddLoss(const std::list<const Ms::ScoreElement*>& elements, Item* lossRoot, const Item* root)
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

void DiagnosticEngravingElementsModel::updateInfo()
{
    std::list<const Ms::ScoreElement*> elements = engravingRegister()->elements();
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

QString DiagnosticEngravingElementsModel::info() const
{
    return m_info;
}

QString DiagnosticEngravingElementsModel::summary() const
{
    return m_summary;
}

// Item ===============
DiagnosticEngravingElementsModel::Item::Item(Item* parent)
    : m_parent(parent)
{
    if (m_parent) {
        m_parent->addChild(this);
    }
}

DiagnosticEngravingElementsModel::Item::~Item()
{
    for (Item* item : m_children) {
        delete item;
    }
}

int DiagnosticEngravingElementsModel::Item::row() const
{
    if (m_parent) {
        return m_parent->m_children.indexOf(const_cast<Item*>(this));
    }
    return 0;
}
