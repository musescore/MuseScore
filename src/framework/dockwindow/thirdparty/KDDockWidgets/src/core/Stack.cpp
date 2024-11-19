/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "Stack.h"
#include "Stack_p.h"
#include "Config.h"
#include "ViewFactory.h"
#include "Logging_p.h"
#include "Utils_p.h"
#include "WindowBeingDragged_p.h"
#include "DockWidget_p.h"
#include "TabBar.h"
#include "Group.h"
#include "FloatingWindow.h"
#include "ObjectGuard_p.h"

#include "views/StackViewInterface.h"

using namespace KDDockWidgets;
using namespace KDDockWidgets::Core;

Stack::Stack(Group *group, StackOptions options)
    : Controller(ViewType::Stack, Config::self().viewFactory()->createStack(this, group->view()))
    , Draggable(view(),
                Config::self().flags()
                    & (Config::Flag_HideTitleBarWhenTabsVisible | Config::Flag_AlwaysShowTabs))
    , d(new Private(group, options, this))

{
    // needs to be initialized out of Private(), as tabbar view's init will call into stack's private
    d->m_tabBar = new TabBar(this);

    view()->init();
}

Stack::~Stack()
{
    delete d->m_tabBar;
    delete d;
}

StackOptions Stack::options() const
{
    return d->m_options;
}

bool Stack::isPositionDraggable(Point p) const
{
    if (auto svi = dynamic_cast<Core::StackViewInterface *>(view()))
        return svi->isPositionDraggable(p);

    return false;
}

void Stack::addDockWidget(DockWidget *dock)
{
    insertDockWidget(dock, numDockWidgets());
}

bool Stack::insertDockWidget(DockWidget *dock, int index)
{
    assert(dock);

    if (index < 0)
        index = 0;
    if (index > numDockWidgets())
        index = numDockWidgets();

    if (contains(dock)) {
        KDDW_ERROR("Refusing to add already existing widget");
        return false;
    }

    ObjectGuard<Group> oldFrame = dock->d->group();

    d->m_tabBar->insertDockWidget(index, dock, dock->icon(IconPlace::TabBar), dock->title());
    d->m_tabBar->setCurrentIndex(index);

    if (oldFrame && oldFrame->beingDeletedLater()) {
        // give it a push and delete it immediately.
        // Having too many deleteLater() puts us in an inconsistent state. For example if
        // LayoutSaver::saveState() would to be called while the Frame hadn't been deleted yet it
        // would count with that group unless hacks. Also the unit-tests are full of
        // waitForDeleted() due to deleteLater.

        // Ideally we would just remove the deleteLater from Group.cpp, but QStack::insertTab()
        // would crash, as it accesses the old tab-widget we're stealing from

        delete oldFrame;
    }

    return true;
}

bool Stack::contains(DockWidget *dw) const
{
    return d->m_tabBar->indexOfDockWidget(dw) != -1;
}

Group *Stack::group() const
{
    return d->m_group;
}

std::unique_ptr<WindowBeingDragged> Stack::makeWindow()
{
    // This is called when using Flag_HideTitleBarWhenTabsVisible
    // For detaching individual tabs, TabBar::makeWindow() is called.

    if (auto fw = view()->rootView()->asFloatingWindowController()) {
        if (fw->hasSingleGroup()) {
            // We're already in a floating window, and it only has 1 dock widget.
            // So there's no detachment to be made, we just move the window.
            return std::make_unique<WindowBeingDragged>(fw, this);
        }
    }

    Rect r = d->m_group->view()->geometry();

    const Point globalPoint = view()->mapToGlobal(Point(0, 0));

    auto floatingWindow = new FloatingWindow(d->m_group, {});
    r.moveTopLeft(globalPoint);
    floatingWindow->setSuggestedGeometry(r, SuggestedGeometryHint_GeometryIsFromDocked);
    floatingWindow->view()->show();

    return std::make_unique<WindowBeingDragged>(floatingWindow, this);
}

bool Stack::isWindow() const
{
    if (auto fw = view()->rootView()->asFloatingWindowController()) {
        // Case of dragging via the tab widget when the title bar is hidden
        return fw->hasSingleGroup();
    }

    return false;
}

Core::DockWidget *Stack::singleDockWidget() const
{
    if (d->m_group->hasSingleDockWidget()) {
        const auto dockWidgets = d->m_group->dockWidgets();
        return dockWidgets.first();
    }

    return nullptr;
}

bool Stack::isMDI() const
{
    return d->m_group && d->m_group->isMDI();
}

bool Stack::onMouseDoubleClick(Point localPos)
{
    // User clicked the empty space of the tab widget and we don't have title bar
    // We float the entire group.

    if (!(Config::self().flags() & Config::Flag_HideTitleBarWhenTabsVisible)
        || tabBar()->dockWidgetAt(localPos))
        return false;

    Group *group = this->group();

    // When using MainWindowOption_HasCentralFrame. The central group is never detachable.
    if (group->isCentralGroup())
        return false;

    if (FloatingWindow *fw = group->floatingWindow()) {
        if (!fw->hasSingleGroup()) {
            makeWindow();
            return true;
        }
    } else if (group->isInMainWindow()) {
        makeWindow();
        return true;
    }

    return false;
}

void Stack::setTabBarAutoHide(bool is)
{
    if (is == d->m_tabBarAutoHide)
        return;

    d->m_tabBarAutoHide = is;
    d->tabBarAutoHideChanged.emit(is);
}

bool Stack::tabBarAutoHide() const
{
    return d->m_tabBarAutoHide;
}

Core::TabBar *Stack::tabBar() const
{
    return d->m_tabBar;
}

int Stack::numDockWidgets() const
{
    return d->m_tabBar->numDockWidgets();
}

void Stack::setDocumentMode(bool is)
{
    dynamic_cast<Core::StackViewInterface *>(view())->setDocumentMode(is);
}

void Stack::setHideDisabledButtons(TitleBarButtonTypes types)
{
    if (d->m_buttonsToHideIfDisabled != types) {
        d->m_buttonsToHideIfDisabled = types;
        d->buttonsToHideIfDisabledChanged.emit();
    }
}

bool Stack::buttonHidesIfDisabled(TitleBarButtonType type) const
{
    return d->m_buttonsToHideIfDisabled & type;
}
