//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "abstractinstrumentpaneltreeitem.h"

#include "log.h"

using namespace mu::instruments;
using namespace mu::notation;

AbstractInstrumentPanelTreeItem::AbstractInstrumentPanelTreeItem(const InstrumentTreeItemType::ItemType& type,
                                                                 notation::INotationParts* notationParts, QObject* parent)
    : QObject(parent), m_notationParts(notationParts)
{
    setType(type);

    m_canChangeVisibility = isSelectable();
}

AbstractInstrumentPanelTreeItem::~AbstractInstrumentPanelTreeItem()
{
    for (AbstractInstrumentPanelTreeItem* child : m_children) {
        child->deleteLater();
    }
}

bool AbstractInstrumentPanelTreeItem::canAcceptDrop(const int type) const
{
    return static_cast<InstrumentTreeItemType::ItemType>(type) == m_type;
}

void AbstractInstrumentPanelTreeItem::appendNewItem()
{
}

QString AbstractInstrumentPanelTreeItem::id() const
{
    return m_id;
}

bool AbstractInstrumentPanelTreeItem::canChangeVisibility() const
{
    return m_canChangeVisibility;
}

QString AbstractInstrumentPanelTreeItem::title() const
{
    return m_title;
}

int AbstractInstrumentPanelTreeItem::type() const
{
    return static_cast<int>(m_type);
}

bool AbstractInstrumentPanelTreeItem::isVisible() const
{
    return m_isVisible;
}

bool AbstractInstrumentPanelTreeItem::isSelectable() const
{
    return m_type == InstrumentTreeItemType::ItemType::PART
           || m_type == InstrumentTreeItemType::ItemType::INSTRUMENT
           || m_type == InstrumentTreeItemType::ItemType::STAFF;
}

AbstractInstrumentPanelTreeItem* AbstractInstrumentPanelTreeItem::parentItem() const
{
    return m_parent;
}

void AbstractInstrumentPanelTreeItem::setParentItem(AbstractInstrumentPanelTreeItem* parent)
{
    m_parent = parent;
}

QList<AbstractInstrumentPanelTreeItem*> AbstractInstrumentPanelTreeItem::childrenItems() const
{
    return m_children;
}

AbstractInstrumentPanelTreeItem* AbstractInstrumentPanelTreeItem::childAtId(const QString& id) const
{
    for (AbstractInstrumentPanelTreeItem* item: m_children) {
        if (item->id() == id) {
            return item;
        }
    }

    return nullptr;
}

AbstractInstrumentPanelTreeItem* AbstractInstrumentPanelTreeItem::childAtRow(const int row) const
{
    if (row < 0 || row >= childCount()) {
        return nullptr;
    }

    return m_children.at(row);
}

void AbstractInstrumentPanelTreeItem::appendChild(AbstractInstrumentPanelTreeItem* child)
{
    if (!child) {
        return;
    }

    child->setParentItem(this);

    m_children.append(child);
}

void AbstractInstrumentPanelTreeItem::moveChildren(const int sourceRow, const int count, AbstractInstrumentPanelTreeItem* destinationParent,
                                                   const int destinationRow)
{
    QList<AbstractInstrumentPanelTreeItem*> chilrenToMove;
    for (int i = sourceRow; i < sourceRow + count; ++i) {
        chilrenToMove << childAtRow(i);
    }

    int destinationRow_ = destinationRow;
    for (AbstractInstrumentPanelTreeItem* child: chilrenToMove) {
        destinationParent->insertChild(child, destinationRow_);
        destinationRow_++;
    }

    int childToRemoveIndex = sourceRow > destinationRow ? sourceRow + count : 0;
    for (int i = 0; i < count; ++i) {
        removeChildren(childToRemoveIndex);
    }
}

void AbstractInstrumentPanelTreeItem::insertChild(AbstractInstrumentPanelTreeItem* child, const int beforeRow)
{
    if (!child) {
        return;
    }

    child->setParentItem(this);

    m_children.insert(beforeRow, child);
}

void AbstractInstrumentPanelTreeItem::replaceChild(AbstractInstrumentPanelTreeItem* child, const int row)
{
    if (!child) {
        return;
    }

    child->setParentItem(this);

    m_children.replace(row, child);
}

void AbstractInstrumentPanelTreeItem::removeChildren(const int row, const int count, const bool deleteChild)
{
    for (int i = row + count - 1; i >= row; --i) {
        AbstractInstrumentPanelTreeItem* child = m_children.at(i);

        m_children.removeAt(i);

        if (deleteChild) {
            child->deleteLater();
        }
    }
}

int AbstractInstrumentPanelTreeItem::childCount() const
{
    return m_children.size();
}

int AbstractInstrumentPanelTreeItem::row() const
{
    if (!parentItem()) {
        return 0;
    }

    return parentItem()->childrenItems().indexOf(const_cast<AbstractInstrumentPanelTreeItem*>(this));
}

void AbstractInstrumentPanelTreeItem::setType(InstrumentTreeItemType::ItemType type)
{
    if (m_type == type) {
        return;
    }

    m_type = type;
    emit typeChanged(m_type);
    emit isSelectableChanged(isSelectable());
}

void AbstractInstrumentPanelTreeItem::setTitle(QString title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;
    emit titleChanged(m_title);
}

void AbstractInstrumentPanelTreeItem::setIsVisible(bool isVisible)
{
    if (m_isVisible == isVisible) {
        return;
    }

    m_isVisible = isVisible;

    for (int i = 0; i < childCount(); ++i) {
        childAtRow(i)->setIsVisible(isVisible);
    }

    emit isVisibleChanged(m_isVisible);
}

void AbstractInstrumentPanelTreeItem::setId(const QString& id)
{
    m_id = id;
}

void AbstractInstrumentPanelTreeItem::setCanChangeVisibility(bool value)
{
    if (m_canChangeVisibility == value) {
        return;
    }

    m_canChangeVisibility = value;
    emit canChangeVisibilityChanged(value);
}

INotationParts* AbstractInstrumentPanelTreeItem::notationParts() const
{
    IF_ASSERT_FAILED(m_notationParts) {
        return nullptr;
    }

    return m_notationParts;
}
