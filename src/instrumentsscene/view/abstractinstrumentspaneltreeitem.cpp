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
#include "abstractinstrumentspaneltreeitem.h"

#include "log.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace muse;

AbstractInstrumentsPanelTreeItem::AbstractInstrumentsPanelTreeItem(const InstrumentsTreeItemType::ItemType& type,
                                                                   IMasterNotationPtr masterNotation,
                                                                   INotationPtr notation,
                                                                   QObject* parent)
    : QObject(parent), m_masterNotation(masterNotation), m_notation(notation)
{
    setType(type);
}

AbstractInstrumentsPanelTreeItem::~AbstractInstrumentsPanelTreeItem()
{
    for (AbstractInstrumentsPanelTreeItem* child : m_children) {
        child->deleteLater();
    }
}

muse::ID AbstractInstrumentsPanelTreeItem::id() const
{
    return m_id;
}

QString AbstractInstrumentsPanelTreeItem::idStr() const
{
    return m_id.toQString();
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
    return false;
}

bool AbstractInstrumentsPanelTreeItem::isSelected() const
{
    return m_isSelected;
}

bool AbstractInstrumentsPanelTreeItem::isExpandable() const
{
    return m_isExpandable;
}

bool AbstractInstrumentsPanelTreeItem::isEditable() const
{
    return m_isEditable;
}

bool AbstractInstrumentsPanelTreeItem::isRemovable() const
{
    return m_isRemovable;
}

bool AbstractInstrumentsPanelTreeItem::canAcceptDrop(const QVariant& obj) const
{
    auto item = dynamic_cast<const AbstractInstrumentsPanelTreeItem*>(obj.value<QObject*>());
    if (!item) {
        return false;
    }

    return item->m_parent == m_parent && item->m_type == m_type;
}

void AbstractInstrumentsPanelTreeItem::appendNewItem()
{
}

MoveParams AbstractInstrumentsPanelTreeItem::buildMoveParams(int, int, AbstractInstrumentsPanelTreeItem*, int) const
{
    UNREACHABLE;
    return MoveParams();
}

void AbstractInstrumentsPanelTreeItem::moveChildren(int sourceRow, int count,
                                                    AbstractInstrumentsPanelTreeItem* destinationParent,
                                                    int destinationRow, bool)
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

void AbstractInstrumentsPanelTreeItem::removeChildren(int row, int count, bool deleteChild)
{
    for (int i = row + count - 1; i >= row; --i) {
        AbstractInstrumentsPanelTreeItem* child = m_children.at(i);

        m_children.removeAt(i);

        if (deleteChild) {
            child->deleteLater();
        }
    }
}

AbstractInstrumentsPanelTreeItem* AbstractInstrumentsPanelTreeItem::parentItem() const
{
    return m_parent;
}

void AbstractInstrumentsPanelTreeItem::setParentItem(AbstractInstrumentsPanelTreeItem* parent)
{
    m_parent = parent;
}

AbstractInstrumentsPanelTreeItem* AbstractInstrumentsPanelTreeItem::childAtId(const ID& id) const
{
    for (AbstractInstrumentsPanelTreeItem* item: m_children) {
        if (item->id() == id) {
            return item;
        }
    }

    return nullptr;
}

AbstractInstrumentsPanelTreeItem* AbstractInstrumentsPanelTreeItem::childAtRow(int row) const
{
    if (row < 0 || row >= m_children.size()) {
        return nullptr;
    }

    return m_children.at(row);
}

const QList<AbstractInstrumentsPanelTreeItem*>& AbstractInstrumentsPanelTreeItem::childItems() const
{
    return m_children;
}

int AbstractInstrumentsPanelTreeItem::indexOf(const AbstractInstrumentsPanelTreeItem* item) const
{
    IF_ASSERT_FAILED(item) {
        return -1;
    }

    return m_children.indexOf(const_cast<AbstractInstrumentsPanelTreeItem*>(item));
}

void AbstractInstrumentsPanelTreeItem::appendChild(AbstractInstrumentsPanelTreeItem* child)
{
    if (!child) {
        return;
    }

    child->setParentItem(this);

    m_children.append(child);

    if (isSelected()) {
        child->setIsSelected(true);
    }
}

void AbstractInstrumentsPanelTreeItem::insertChild(AbstractInstrumentsPanelTreeItem* child, int beforeRow)
{
    if (!child) {
        return;
    }

    child->setParentItem(this);

    m_children.insert(beforeRow, child);

    if (isSelected()) {
        child->setIsSelected(true);
    }
}

bool AbstractInstrumentsPanelTreeItem::isEmpty() const
{
    return m_children.isEmpty();
}

int AbstractInstrumentsPanelTreeItem::childCount() const
{
    return m_children.size();
}

int AbstractInstrumentsPanelTreeItem::row() const
{
    return m_parent ? m_parent->indexOf(this) : 0;
}

void AbstractInstrumentsPanelTreeItem::setType(InstrumentsTreeItemType::ItemType type)
{
    if (m_type == type) {
        return;
    }

    m_type = type;
    emit typeChanged(m_type);
}

void AbstractInstrumentsPanelTreeItem::setTitle(QString title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;
    emit titleChanged(m_title);
}

void AbstractInstrumentsPanelTreeItem::setIsVisible(bool isVisible, bool setChildren)
{
    if (m_isVisible == isVisible) {
        return;
    }

    m_isVisible = isVisible;
    emit isVisibleChanged(isVisible);

    if (setChildren) {
        for (auto child : m_children) {
            child->setIsVisible(isVisible);
        }
    }
}

void AbstractInstrumentsPanelTreeItem::setId(const ID& id)
{
    m_id = id;
}

void AbstractInstrumentsPanelTreeItem::setIsExpandable(bool expandable)
{
    if (m_isExpandable == expandable) {
        return;
    }

    m_isExpandable = expandable;
    emit isExpandableChanged(expandable);
}

void AbstractInstrumentsPanelTreeItem::setIsEditable(bool editable)
{
    if (m_isEditable == editable) {
        return;
    }

    m_isEditable = editable;
    emit isEditableChanged(editable);
}

void AbstractInstrumentsPanelTreeItem::setIsRemovable(bool removable)
{
    if (m_isRemovable == removable) {
        return;
    }

    m_isRemovable = removable;
    emit isRemovableChanged(removable);
}

void AbstractInstrumentsPanelTreeItem::setIsSelected(bool selected)
{
    if (m_isSelected == selected) {
        return;
    }

    for (AbstractInstrumentsPanelTreeItem* child: m_children) {
        child->setIsSelected(selected);
    }

    m_isSelected = selected;
    emit isSelectedChanged(selected);
}

IMasterNotationPtr AbstractInstrumentsPanelTreeItem::masterNotation() const
{
    return m_masterNotation;
}

INotationPtr AbstractInstrumentsPanelTreeItem::notation() const
{
    return m_notation;
}
