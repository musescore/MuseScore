/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "ItemFreeContainer_p.h"
#include "LayoutingHost_p.h"

#include "core/View.h"
#include "core/Logging_p.h"
#include "core/Utils_p.h"

using namespace KDDockWidgets::Core;

ItemFreeContainer::ItemFreeContainer(LayoutingHost *hostWidget, ItemContainer *parent)
    : ItemContainer(hostWidget, parent)
{
}

ItemFreeContainer::ItemFreeContainer(LayoutingHost *hostWidget)
    : ItemContainer(hostWidget)
{
}

ItemFreeContainer::~ItemFreeContainer()
{
}

void ItemFreeContainer::addDockWidget(Item *item, Point localPt)
{
    assert(item != this);
    if (contains(item)) {
        KDDW_ERROR("Item already exists");
        return;
    }
    item->setIsVisible(true); // Use OptionStartHidden here too ?

    m_children.append(item);
    item->setParentContainer(this);
    item->setPos(localPt);

    itemsChanged.emit();

    if (item->isVisible())
        numVisibleItemsChanged.emit(numVisibleChildren());

    numItemsChanged.emit();
}

void ItemFreeContainer::clear()
{
    deleteAll(m_children);
    m_children.clear();
}

void ItemFreeContainer::removeItem(Item *item, bool hardRemove)
{
    const bool wasVisible = item->isVisible();

    if (hardRemove) {
        m_children.removeOne(item);
        delete item;
    } else {
        item->setIsVisible(false);
        item->setGuest(nullptr);
    }

    if (wasVisible)
        numVisibleItemsChanged.emit(numVisibleChildren());

    itemsChanged.emit();
}

void ItemFreeContainer::restore(Item *child)
{
    child->setIsVisible(true);
}

void ItemFreeContainer::onChildMinSizeChanged(Item *)
{
    // Nothing here either, although we could update the size constraints
}

void ItemFreeContainer::onChildVisibleChanged(Item *, bool)
{
    // Nothing needed to do in this layout type
}
