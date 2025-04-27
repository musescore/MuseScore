/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "abstractlayoutpaneltreeitem.h"

#include "log.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace muse;

AbstractLayoutPanelTreeItem::AbstractLayoutPanelTreeItem(LayoutPanelItemType::ItemType type,
                                                         IMasterNotationPtr masterNotation,
                                                         INotationPtr notation,
                                                         QObject* parent)
    : QObject(parent), m_type(type), m_masterNotation(masterNotation), m_notation(notation)
{
}

AbstractLayoutPanelTreeItem::~AbstractLayoutPanelTreeItem()
{
    for (AbstractLayoutPanelTreeItem* child : m_children) {
        child->deleteLater();
    }
}

muse::ID AbstractLayoutPanelTreeItem::id() const
{
    return m_id;
}

QString AbstractLayoutPanelTreeItem::idStr() const
{
    return m_id.toQString();
}

QString AbstractLayoutPanelTreeItem::title() const
{
    return m_title;
}

int AbstractLayoutPanelTreeItem::typeInt() const
{
    return static_cast<int>(m_type);
}

LayoutPanelItemType::ItemType AbstractLayoutPanelTreeItem::type() const
{
    return m_type;
}

bool AbstractLayoutPanelTreeItem::isVisible() const
{
    return m_isVisible;
}

bool AbstractLayoutPanelTreeItem::isSelectable() const
{
    return m_isSelectable;
}

bool AbstractLayoutPanelTreeItem::isSelected() const
{
    return m_isSelected;
}

bool AbstractLayoutPanelTreeItem::isExpandable() const
{
    return m_isExpandable;
}

bool AbstractLayoutPanelTreeItem::isLinked() const
{
    return m_isLinked;
}

bool AbstractLayoutPanelTreeItem::settingsAvailable() const
{
    return m_settingsAvailable;
}

bool AbstractLayoutPanelTreeItem::settingsEnabled() const
{
    return m_settingsEnabled;
}

bool AbstractLayoutPanelTreeItem::isRemovable() const
{
    return m_isRemovable;
}

bool AbstractLayoutPanelTreeItem::canAcceptDrop(const QVariant& obj) const
{
    auto item = dynamic_cast<const AbstractLayoutPanelTreeItem*>(obj.value<QObject*>());
    if (!item) {
        return false;
    }

    return item->m_parent == m_parent && item->m_type == m_type;
}

void AbstractLayoutPanelTreeItem::appendNewItem()
{
}

MoveParams AbstractLayoutPanelTreeItem::buildMoveParams(int, int, AbstractLayoutPanelTreeItem*, int) const
{
    UNREACHABLE;
    return MoveParams();
}

void AbstractLayoutPanelTreeItem::moveChildren(int sourceRow, int count,
                                               AbstractLayoutPanelTreeItem* destinationParent,
                                               int destinationRow, bool)
{
    QList<AbstractLayoutPanelTreeItem*> childrenToMove;
    for (int i = sourceRow; i < sourceRow + count; ++i) {
        childrenToMove << childAtRow(i);
    }

    int destinationRow_ = destinationRow;
    for (AbstractLayoutPanelTreeItem* child: childrenToMove) {
        destinationParent->insertChild(child, destinationRow_);
        destinationRow_++;
    }

    int childToRemoveIndex = sourceRow;

    if (sourceRow > destinationRow && destinationParent == this) {
        childToRemoveIndex = sourceRow + count;
    }

    AbstractLayoutPanelTreeItem::removeChildren(childToRemoveIndex, count);
}

void AbstractLayoutPanelTreeItem::moveChildrenOnScore(const MoveParams&)
{
}

void AbstractLayoutPanelTreeItem::removeChildren(int row, int count, bool deleteChild)
{
    for (int i = row + count - 1; i >= row; --i) {
        AbstractLayoutPanelTreeItem* child = m_children.at(i);

        m_children.removeAt(i);

        if (deleteChild) {
            child->deleteLater();
        }
    }
}

void AbstractLayoutPanelTreeItem::onScoreChanged(const mu::engraving::ScoreChangesRange&)
{
}

AbstractLayoutPanelTreeItem* AbstractLayoutPanelTreeItem::parentItem() const
{
    return m_parent;
}

void AbstractLayoutPanelTreeItem::setParentItem(AbstractLayoutPanelTreeItem* parent)
{
    m_parent = parent;
}

AbstractLayoutPanelTreeItem* AbstractLayoutPanelTreeItem::childAtId(const ID& id, LayoutPanelItemType::ItemType type) const
{
    for (AbstractLayoutPanelTreeItem* item: m_children) {
        if (item->m_id == id && item->m_type == type) {
            return item;
        }
    }

    return nullptr;
}

AbstractLayoutPanelTreeItem* AbstractLayoutPanelTreeItem::childAtRow(int row) const
{
    if (row < 0 || row >= m_children.size()) {
        return nullptr;
    }

    return m_children.at(row);
}

const QList<AbstractLayoutPanelTreeItem*>& AbstractLayoutPanelTreeItem::childItems() const
{
    return m_children;
}

LayoutPanelItemType::ItemType AbstractLayoutPanelTreeItem::childType(int row) const
{
    const AbstractLayoutPanelTreeItem* child = childAtRow(row);
    return child ? child->type() : LayoutPanelItemType::UNDEFINED;
}

int AbstractLayoutPanelTreeItem::indexOf(const AbstractLayoutPanelTreeItem* item) const
{
    IF_ASSERT_FAILED(item) {
        return -1;
    }

    return m_children.indexOf(const_cast<AbstractLayoutPanelTreeItem*>(item));
}

void AbstractLayoutPanelTreeItem::appendChild(AbstractLayoutPanelTreeItem* child)
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

void AbstractLayoutPanelTreeItem::insertChild(AbstractLayoutPanelTreeItem* child, int beforeRow)
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

bool AbstractLayoutPanelTreeItem::isEmpty() const
{
    return m_children.isEmpty();
}

int AbstractLayoutPanelTreeItem::childCount() const
{
    return m_children.size();
}

int AbstractLayoutPanelTreeItem::row() const
{
    return m_parent ? m_parent->indexOf(this) : 0;
}

void AbstractLayoutPanelTreeItem::setTitle(QString title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;
    emit titleChanged(m_title);
}

void AbstractLayoutPanelTreeItem::setIsVisible(bool isVisible, bool setChildren)
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

void AbstractLayoutPanelTreeItem::setId(const ID& id)
{
    m_id = id;
}

void AbstractLayoutPanelTreeItem::setIsExpandable(bool expandable)
{
    if (m_isExpandable == expandable) {
        return;
    }

    m_isExpandable = expandable;
    emit isExpandableChanged(expandable);
}

void AbstractLayoutPanelTreeItem::setIsLinked(bool linked)
{
    if (m_isLinked == linked) {
        return;
    }

    m_isLinked = linked;
    emit isLinkedChanged(linked);
}
void AbstractLayoutPanelTreeItem::setSettingsAvailable(bool available)
{
    if (m_settingsAvailable == available) {
        return;
    }

    m_settingsAvailable = available;
    emit settingsAvailableChanged(available);

    if (!available) {
        setSettingsEnabled(false);
    }
}

void AbstractLayoutPanelTreeItem::setSettingsEnabled(bool enabled)
{
    if (m_settingsEnabled == enabled) {
        return;
    }

    m_settingsEnabled = enabled;
    emit settingsEnabledChanged(enabled);
}

void AbstractLayoutPanelTreeItem::setIsRemovable(bool removable)
{
    if (m_isRemovable == removable) {
        return;
    }

    m_isRemovable = removable;
    emit isRemovableChanged(removable);
}

void AbstractLayoutPanelTreeItem::setIsSelectable(bool selectable)
{
    if (m_isSelectable == selectable) {
        return;
    }

    m_isSelectable = selectable;
    emit isSelectableChanged(selectable);
}

void AbstractLayoutPanelTreeItem::setIsSelected(bool selected)
{
    if (m_isSelected == selected) {
        return;
    }

    for (AbstractLayoutPanelTreeItem* child: m_children) {
        child->setIsSelected(selected);
    }

    m_isSelected = selected;
    emit isSelectedChanged(selected);
}

IMasterNotationPtr AbstractLayoutPanelTreeItem::masterNotation() const
{
    return m_masterNotation;
}

INotationPtr AbstractLayoutPanelTreeItem::notation() const
{
    return m_notation;
}
