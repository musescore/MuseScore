/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "engraving/dom/engravingobject.h"
#include "engraving/dom/score.h"
#include "engraving/dom/masterscore.h"
#include "dataformatter.h"

#include "log.h"

using namespace muse;
using namespace mu::engraving;

EngravingElementsModel::EngravingElementsModel(QObject* parent)
    : QAbstractItemModel(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
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

    if (item->data().isEmpty()) {
        item->setData(makeData(item->element()));
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

QVariantMap EngravingElementsModel::makeData(const mu::engraving::EngravingObject* el) const
{
    TRACEFUNC;
    if (!el) {
        return QVariantMap();
    }

    auto formatRect = [](const muse::RectF& r) {
        QString str = "[";
        str += DataFormatter::formatReal(r.x(), 1) + u", ";
        str += DataFormatter::formatReal(r.y(), 1) + u", ";
        str += DataFormatter::formatReal(r.width(), 1) + u", ";
        str += DataFormatter::formatReal(r.height(), 1) + u"]";
        return str;
    };

    auto formatPoint= [](const muse::PointF& p) {
        QString str = "[";
        str += DataFormatter::formatReal(p.x(), 1) + u", ";
        str += DataFormatter::formatReal(p.y(), 1) + u"]";
        return str;
    };

    QString name;
    if (el->isScore()) {
        const mu::engraving::Score* score = mu::engraving::toScore(el);
        if (score->isMaster()) {
            name = "MasterScore: " + score->name().toQString();
        } else {
            name = "Score: " + score->name().toQString();
        }
    } else {
        name = el->typeName();
    }

    QString info = name + ": ";
    info += "children: " + QString::number(el->children().size());
    info += "\n";
    if (el->isEngravingItem()) {
        const mu::engraving::EngravingItem* item = mu::engraving::toEngravingItem(el);
        info += "pagePos: " + formatPoint(item->pagePos()) + ", bbox: " + formatRect(item->ldata()->bbox());
    }

    QVariantMap d;
    d["selected"] = elementsProvider()->isSelected(el);
    d["info"] = info;

//    if (el->children().size() != size_t(el->treeChildCount())) {
//        d["color"] = "#ff0000";
//    }

    return d;
}

void EngravingElementsModel::init()
{
}

void EngravingElementsModel::reload()
{
    TRACEFUNC;

    beginResetModel();

    m_allItems.clear();
    delete m_rootItem;
    m_rootItem = createItem(nullptr);

    const EngravingObjectSet& elements = elementsProvider()->elements();
    EngravingObjectSet notpalettes;

    for (const mu::engraving::EngravingObject* el : elements) {
        if (el == mu::engraving::gpaletteScore || el->score() == mu::engraving::gpaletteScore) {
            continue;
        }

        notpalettes.insert(el);

        if (el->isScore() && mu::engraving::toScore(el)->isMaster()) {
            Item* scoreItem = createItem(m_rootItem);
            scoreItem->setElement(el);
            load(elements, scoreItem);
        }
    }

    Item* lostItem = createItem(m_rootItem);
    QVariantMap lostData;
    lostData["info"] = "Lost items";
    lostItem->setData(lostData);
    findAndAddLost(notpalettes, lostItem);

    endResetModel();

    updateInfo();
}

void EngravingElementsModel::load(const EngravingObjectSet& elements, Item* root)
{
    TRACEFUNC;
    for (const mu::engraving::EngravingObject* el : elements) {
        if (el == mu::engraving::gpaletteScore) {
            continue;
        }

        mu::engraving::EngravingObject* parent = el->parent();

        if (parent == root->element()) {
            Item* item = createItem(root);
            item->setElement(el);
            load(elements, item);
        }
    }
}

const EngravingElementsModel::Item* EngravingElementsModel::findItem(const mu::engraving::EngravingObject* el, const Item* root) const
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

void EngravingElementsModel::findAndAddLost(const EngravingObjectSet& elements, Item* lossRoot)
{
    TRACEFUNC;

    QSet<const mu::engraving::EngravingObject*> used;
    for (auto it = m_allItems.begin(); it != m_allItems.end(); ++it) {
        const Item* item = it.value();
        used.insert(item->element());
    }

    for (const mu::engraving::EngravingObject* el : elements) {
        if (used.contains(el)) {
            continue;
        }

        Item* item = createItem(lossRoot);
        item->setElement(el);
    }
}

void EngravingElementsModel::select(QModelIndex index, bool arg)
{
    Item* item = itemByModelIndex(index);
    if (!item) {
        return;
    }

    elementsProvider()->select(item->element(), arg);
    item->setData(QVariantMap());

    emit dataChanged(index, index, { rItemData });

    dispatcher()->dispatch("diagnostic-notationview-redraw");
}

void EngravingElementsModel::click1(QModelIndex index)
{
    //! NOTE For debugging purposes
    Item* item = itemByModelIndex(index);
    if (!item) {
        return;
    }

    const mu::engraving::EngravingObject* el = item->element();
    if (!el) {
        return;
    }

    const mu::engraving::EngravingObject* parent = el->parent();
    UNUSED(parent);

    const mu::engraving::EngravingObject* explicitParent = el->explicitParent();
    UNUSED(explicitParent);

    size_t children = el->children().size();
    UNUSED(children);
}

void EngravingElementsModel::updateInfo()
{
    const EngravingObjectSet& elements = elementsProvider()->elements();
    QHash<QString, int> els;
    for (const mu::engraving::EngravingObject* el : elements) {
        els[el->typeName()] += 1;
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
