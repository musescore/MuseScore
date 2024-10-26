/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "MDILayout.h"
#include "core/layouting/ItemFreeContainer_p.h"
#include "Config.h"
#include "core/ViewFactory.h"
#include "core/Group_p.h"
#include "core/DockWidget_p.h"
#include "core/Logging_p.h"

using namespace KDDockWidgets;
using namespace KDDockWidgets::Core;

MDILayout::MDILayout(View *parent)
    : Layout(ViewType::MDILayout, Config::self().viewFactory()->createMDILayout(this, parent))
    , m_rootItem(new Core::ItemFreeContainer(asLayoutingHost()))
{
    setRootItem(m_rootItem);
}

MDILayout::~MDILayout()
{
}

void MDILayout::addDockWidget(Core::DockWidget *dw, Point localPt,
                              const InitialOption &addingOption)
{
    if (!dw) {
        KDDW_ERROR("Refusing to add null dock widget");
        return;
    }

    const Size dwSize = dw->size();

    auto group = object_cast<Core::Group *>(dw->d->group());
    if (itemForGroup(group) != nullptr) {
        // Item already exists, remove it. See also comment in MultiSplitter::addWidget().
        group->setParentView(nullptr);
        group->setLayoutItem(nullptr);
    }

    Core::Item *newItem = new Core::Item(asLayoutingHost());
    if (group) {
        newItem->setGuest(group->asLayoutingGuest());
    } else {
        group = new Core::Group();
        group->addTab(dw, addingOption);

        newItem->setGuest(group->asLayoutingGuest());
    }

    newItem->setSize(dwSize.expandedTo(newItem->minSize()));

    assert(!newItem->geometry().isEmpty());
    m_rootItem->addDockWidget(newItem, localPt);

    if (addingOption.startsHidden()) {
        delete group;
    }
}

void MDILayout::setDockWidgetGeometry(Core::Group *group, Rect geometry)
{
    if (!group)
        return;

    Core::Item *item = itemForGroup(group);
    if (!item) {
        KDDW_ERROR("Group not found in the layout {}", ( void * )group);
        return;
    }

    item->setGeometry(geometry);
}

void MDILayout::moveDockWidget(Core::DockWidget *dw, Point pos)
{
    moveDockWidget(dw->d->group(), pos);
}

void MDILayout::moveDockWidget(Core::Group *group, Point pos)
{
    if (!group)
        return;

    Core::Item *item = itemForGroup(group);
    if (!item) {
        KDDW_ERROR("Group not found in the layout {}.", ( void * )group);
        return;
    }

    Rect geo = item->geometry();
    geo.moveTopLeft(pos);
    item->setGeometry(geo);
}

void MDILayout::resizeDockWidget(Core::DockWidget *dw, Size size)
{
    resizeDockWidget(dw->d->group(), size);
}

void MDILayout::resizeDockWidget(Core::Group *group, Size size)
{
    if (!group)
        return;

    Core::Item *item = itemForGroup(group);
    if (!item) {
        KDDW_ERROR("Group not found in the layout {} isMDI={}, isMDIWrapper={}", ( void * )group, group->isMDI(), group->isMDIWrapper());
        return;
    }

    item->setSize(size.expandedTo(group->view()->minSize()));
}
