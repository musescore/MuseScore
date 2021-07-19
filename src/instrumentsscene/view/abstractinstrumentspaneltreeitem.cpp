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
#include "abstractinstrumentspaneltreeitem.h"

#include "log.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;

AbstractInstrumentsPanelTreeItem::AbstractInstrumentsPanelTreeItem(const InstrumentsTreeItemType::ItemType& type,
                                                                   INotationPartsPtr notationParts, QObject* parent)
    : QObject(parent), m_notationParts(notationParts)
{
    setType(type);

    m_canChangeVisibility = isSelectable();
}

AbstractInstrumentsPanelTreeItem::~AbstractInstrumentsPanelTreeItem()
{
    for (AbstractInstrumentsPanelTreeItem* child : m_children) {
        child->deleteLater();
    }
}

bool AbstractInstrumentsPanelTreeItem::canAcceptDrop(const int type) const
{
    return static_cast<InstrumentsTreeItemType::ItemType>(type) == m_type;
}

void AbstractInstrumentsPanelTreeItem::appendNewItem()
{
}

QString AbstractInstrumentsPanelTreeItem::id() const
{
    return m_id;
}

bool AbstractInstrumentsPanelTreeItem::canChangeVisibility() const
{
    return m_canChangeVisibility;
}

QString AbstractInstrumentsPanelTreeItem::title() const
{
    return m_title;
}

int AbstractInstrumentsPanelTreeItem::type() const
{
    return static_cast<int>(m_type);
}

bool AbstractInstrumentsPanelTreeItem::isVisible() const
{
    return m_isVisible;
}

bool AbstractInstrumentsPanelTreeItem::isSelectable() const
{
    return m_type == InstrumentsTreeItemType::ItemType::PART
           || m_type == InstrumentsTreeItemType::ItemType::INSTRUMENT
           || m_type == InstrumentsTreeItemType::ItemType::STAFF;
}

AbstractInstrumentsPanelTreeItem* AbstractInstrumentsPanelTreeItem::parentItem() const
{
    return m_parent;
}

void AbstractInstrumentsPanelTreeItem::setParentItem(AbstractInstrumentsPanelTreeItem* parent)
{
    m_parent = parent;
}

QList<AbstractInstrumentsPanelTreeItem*> AbstractInstrumentsPanelTreeItem::childrenItems() const
{
    return m_children;
}

bool AbstractInstrumentsPanelTreeItem::isEmpty() const
{
    return m_children.isEmpty();
}

AbstractInstrumentsPanelTreeItem* AbstractInstrumentsPanelTreeItem::childAtId(const QString& id) const
{
    for (AbstractInstrumentsPanelTreeItem* item: m_children) {
        if (item->id() == id) {
            return item;
        }
    }

    return nullptr;
}

AbstractInstrumentsPanelTreeItem* AbstractInstrumentsPanelTreeItem::childAtRow(const int row) const
{
    if (row < 0 || row >= childCount()) {
        return nullptr;
    }

    return m_children.at(row);
}

void AbstractInstrumentsPanelTreeItem::appendChild(AbstractInstrumentsPanelTreeItem* child)
{
    if (!child) {
        return;
    }

    child->setParentItem(this);

    m_children.append(child);
}

void AbstractInstrumentsPanelTreeItem::moveChildren(const int sourceRow, const int count,
                                                    AbstractInstrumentsPanelTreeItem* destinationParent,
                                                    const int destinationRow)
{
    QList<AbstractInstrumentsPanelTreeItem*> childrenToMove;
    for (int i = sourceRow; i < sourceRow + count; ++i) {
        childrenToMove << childAtRow(i);
    }

    int destinationRow_ = destinationRow;
    for (AbstractInstrumentsPanelTreeItem* child: childrenToMove) {
        destinationParent->insertChild(child, destinationRow_);
        destinationRow_++;
    }

    int childToRemoveIndex = sourceRow;

    if (sourceRow > destinationRow && destinationParent == this) {
        childToRemoveIndex = sourceRow + count;
    }

    AbstractInstrumentsPanelTreeItem::removeChildren(childToRemoveIndex, count);
}

void AbstractInstrumentsPanelTreeItem::insertChild(AbstractInstrumentsPanelTreeItem* child, const int beforeRow)
{
    if (!child) {
        return;
    }

    child->setParentItem(this);

    m_children.insert(beforeRow, child);
}

void AbstractInstrumentsPanelTreeItem::replaceChild(AbstractInstrumentsPanelTreeItem* child, const int row)
{
    if (!child) {
        return;
    }

    child->setParentItem(this);

    m_children.replace(row, child);
}

void AbstractInstrumentsPanelTreeItem::removeChildren(const int row, const int count, const bool deleteChild)
{
    for (int i = row + count - 1; i >= row; --i) {
        AbstractInstrumentsPanelTreeItem* child = m_children.at(i);

        m_children.removeAt(i);

        if (deleteChild) {
            child->deleteLater();
        }
    }
}

int AbstractInstrumentsPanelTreeItem::childCount() const
{
    return m_children.size();
}

int AbstractInstrumentsPanelTreeItem::row() const
{
    if (!parentItem()) {
        return 0;
    }

    return parentItem()->childrenItems().indexOf(const_cast<AbstractInstrumentsPanelTreeItem*>(this));
}

void AbstractInstrumentsPanelTreeItem::setType(InstrumentsTreeItemType::ItemType type)
{
    if (m_type == type) {
        return;
    }

    m_type = type;
    emit typeChanged(m_type);
    emit isSelectableChanged(isSelectable());
}

void AbstractInstrumentsPanelTreeItem::setTitle(QString title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;
    emit titleChanged(m_title);
}

void AbstractInstrumentsPanelTreeItem::setIsVisible(bool isVisible)
{
    if (m_isVisible == isVisible) {
        return;
    }

    m_isVisible = isVisible;
    emit isVisibleChanged(isVisible);

    for (auto child : m_children) {
        child->setIsVisible(isVisible);
    }
}

void AbstractInstrumentsPanelTreeItem::setId(const QString& id)
{
    m_id = id;
}

void AbstractInstrumentsPanelTreeItem::setCanChangeVisibility(bool value)
{
    if (m_canChangeVisibility == value) {
        return;
    }

    m_canChangeVisibility = value;
    emit canChangeVisibilityChanged(value);
}

INotationPartsPtr AbstractInstrumentsPanelTreeItem::notationParts() const
{
    IF_ASSERT_FAILED(m_notationParts) {
        return nullptr;
    }

    return m_notationParts;
}
