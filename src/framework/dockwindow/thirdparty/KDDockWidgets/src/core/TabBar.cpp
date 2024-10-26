/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "TabBar.h"
#include "TabBar_p.h"
#include "core/Draggable_p.h"
#include "Controller.h"
#include "core/ScopedValueRollback_p.h"
#include "core/Stack.h"
#include "core/FloatingWindow.h"
#include "core/DockWidget_p.h"
#include "core/Logging_p.h"
#include "views/TabBarViewInterface.h"
#include "Platform.h"

#include "core/DragController_p.h"
#include "core/Utils_p.h"
#include "Config.h"
#include "core/ViewFactory.h"

#include <cstdlib>

using namespace KDDockWidgets;
using namespace KDDockWidgets::Core;

Core::TabBar::TabBar(Stack *stack)
    : Controller(ViewType::TabBar, Config::self().viewFactory()->createTabBar(this, stack->view()))
    , Draggable(view())
    , d(new Private(stack))
{
    view()->init();
    if (auto tvi = dynamic_cast<Core::TabBarViewInterface *>(view()))
        tvi->setTabsAreMovable(tabsAreMovable());
}

Core::TabBar::~TabBar()
{
    delete d;
}

bool Core::TabBar::tabsAreMovable() const
{
    return Config::self().flags() & Config::Flag_AllowReorderTabs;
}

bool Core::TabBar::dragCanStart(Point pressPos, Point pos) const
{
    // Here we allow the user to re-order tabs instead of dragging them off.
    // To do that we just return false here, and QTabBar will handle the mouse event, assuming
    // QTabBar::isMovable.

    const bool defaultResult = Draggable::dragCanStart(pressPos, pos);

    if (!defaultResult || !tabsAreMovable()) {
        // Nothing more to do. If the drag wouldn't start anyway, return false.
        // And if the tabs aren't movable, just return the default result, which just considers
        // startDragDistance
        return defaultResult;
    }

    const int index =
        dynamic_cast<Core::TabBarViewInterface *>(view())->tabAt(view()->mapFromGlobal(pos));
    if (index == -1)
        return defaultResult;

    const int deltaX = std::abs(pos.x() - pressPos.x());
    const int deltaY = std::abs(pos.y() - pressPos.y());

    const int startDragDistance = Platform::instance()->startDragDistance();

    if (deltaY > 5 * startDragDistance) {
        // Moving up or down too much results in a detach. No tab re-ordering allowed.
        return true;
    } else if (deltaY > startDragDistance && deltaX < startDragDistance) {
        // Moved a bit up or down, but not left/right, then detach too.
        // Only if it's going considerably left/right we allow to re-order tabs.
        return true;
    }

    return false;
}

Core::DockWidget *Core::TabBar::dockWidgetAt(int index) const
{
    if (index < 0 || index >= numDockWidgets())
        return nullptr;

    return const_cast<DockWidget *>(d->m_dockWidgets.value(index));
}

Core::DockWidget *Core::TabBar::dockWidgetAt(Point localPos) const
{
    if (auto tvi = dynamic_cast<Core::TabBarViewInterface *>(view()))
        return dockWidgetAt(tvi->tabAt(localPos));

    return nullptr;
}

int TabBar::indexOfDockWidget(const Core::DockWidget *dw) const
{
    return d->m_dockWidgets.indexOf(dw);
}

void TabBar::removeDockWidget(Core::DockWidget *dw)
{
    if (m_inDtor)
        return;

    auto it = d->aboutToDeleteConnections.find(dw);
    if (it != d->aboutToDeleteConnections.end())
        d->aboutToDeleteConnections.erase(it);

    const bool wasCurrent = dw == d->m_currentDockWidget;
    const int index = d->m_dockWidgets.indexOf(dw);

    if (wasCurrent) {
        const bool isLast = index == d->m_dockWidgets.count() - 1;
        const int newCurrentIndex = isLast ? index - 1 : index + 1;
        setCurrentIndex(newCurrentIndex);
    }

    d->m_removeGuard = true;
    // The view might call setCurrenteIndex() before our d->m_dockWidgets reflectig the state.
    // d->m_removeGuard protects against that.
    if (auto tvi = dynamic_cast<Core::TabBarViewInterface *>(view()))
        tvi->removeDockWidget(dw);
    d->m_removeGuard = false;

    d->m_dockWidgets.removeOne(dw);
    group()->onDockWidgetCountChanged();
}

void TabBar::insertDockWidget(int index, Core::DockWidget *dw, const Icon &icon,
                              const QString &title)
{
    if (auto oldGroup = dw->dptr()->group()) {
        if (auto oldTabBar = oldGroup->tabBar()) {
            if (oldTabBar != this) {
                oldTabBar->removeDockWidget(dw);
            }
        }
    }

    d->m_dockWidgets.insert(index, dw);
    KDBindings::ScopedConnection conn = dw->d->aboutToDelete.connect([this, dw] {
        removeDockWidget(dw);
    });
    d->aboutToDeleteConnections[dw] = std::move(conn);

    dynamic_cast<Core::TabBarViewInterface *>(view())->insertDockWidget(index, dw, icon, title);
    if (!d->m_currentDockWidget)
        setCurrentDockWidget(dw);

    group()->onDockWidgetCountChanged();
}

std::unique_ptr<WindowBeingDragged> Core::TabBar::makeWindow()
{
    auto dock = d->m_lastPressedDockWidget;
    d->m_lastPressedDockWidget = nullptr;

    const bool hideTitleBarWhenTabsVisible =
        Config::self().flags() & Config::Flag_HideTitleBarWhenTabsVisible;
    const bool alwaysShowTabs = Config::self().flags() & Config::Flag_AlwaysShowTabs;

    if (hideTitleBarWhenTabsVisible) {
        if (dock) {
            if (alwaysShowTabs && hasSingleDockWidget()) {
                // Case #1. User is dragging a tab but there's only 1 tab (and tabs are always
                // visible), so drag everything instead, no detaching happens
                return d->m_stack->makeWindow();
            }
        } else {
            // Case #2. User is dragging on the QTabBar background, not on an actual tab.
            // As Flag_HideTitleBarWhenTabsVisible is set, we let the user drag through the tab
            // widget background.
            return d->m_stack->makeWindow();
        }
    } else {
        if (dock && hasSingleDockWidget() && alwaysShowTabs) {
            // Case #3. window with title bar and single tab, no detaching should happen, just use
            // the title bar.
            return {};
        }
    }

    if (!dock)
        return {};

    FloatingWindow *floatingWindow = group()->detachTab(dock);
    if (!floatingWindow)
        return {};

    auto draggable = KDDockWidgets::usesNativeTitleBar() ? static_cast<Draggable *>(floatingWindow)
                                                         : static_cast<Draggable *>(this);
    return std::make_unique<WindowBeingDragged>(floatingWindow, draggable);
}

bool Core::TabBar::isWindow() const
{
    // Same semantics as tab widget, no need to duplicate logic
    return d->m_stack->isWindow();
}

void Core::TabBar::onMousePress(Point localPos)
{
    d->m_lastPressedDockWidget = dockWidgetAt(localPos);
    Group *group = this->group();
    if (Config::self().flags() & Config::Flag_TitleBarIsFocusable) {
        // User clicked on a tab which was already focused
        // A tab changing also counts as a change of scope
        group->FocusScope::focus(Qt::MouseFocusReason);
    }
}

void Core::TabBar::onMouseDoubleClick(Point localPos)
{
    if (DockWidget *dw = dockWidgetAt(localPos))
        dw->setFloating(true);
}

bool Core::TabBar::hasSingleDockWidget() const
{
    return numDockWidgets() == 1;
}

int Core::TabBar::numDockWidgets() const
{
    return d->m_dockWidgets.size();
}

Core::DockWidget *Core::TabBar::singleDockWidget() const
{
    return d->m_stack->singleDockWidget();
}

bool Core::TabBar::isMDI() const
{
    Group *f = group();
    return f && f->isMDI();
}

Group *Core::TabBar::group() const
{
    return d->m_stack->group();
}

Stack *TabBar::stack() const
{
    return d->m_stack;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters
void Core::TabBar::Private::moveTabTo(int from, int to)
{
    auto fromDw = m_dockWidgets.takeAt(from);
    m_dockWidgets.insert(to, fromDw);
}

void Core::TabBar::moveTabTo(int from, int to)
{
    ScopedValueRollback guard(d->m_isMovingTab, true);

    d->moveTabTo(from, to);

    // Tell GUI:
    if (auto tvi = dynamic_cast<Core::TabBarViewInterface *>(view()))
        tvi->moveTabTo(from, to);
}

QString Core::TabBar::text(int index) const
{
    if (auto tvi = dynamic_cast<Core::TabBarViewInterface *>(view()))
        return tvi->text(index);

    return {};
}

Rect Core::TabBar::rectForTab(int index) const
{
    if (auto tvi = dynamic_cast<Core::TabBarViewInterface *>(view()))
        return tvi->rectForTab(index);

    return {};
}

DockWidget *TabBar::currentDockWidget() const
{
    return d->m_currentDockWidget;
}

void TabBar::setCurrentDockWidget(DockWidget *dw)
{
    if (d->m_removeGuard) // We're in the middle of a remove.
        return;

    if (dw == d->m_currentDockWidget)
        return;

    setCurrentIndex(indexOfDockWidget(dw));
}

int TabBar::currentIndex() const
{
    if (!d->m_currentDockWidget)
        return -1;

    return d->m_dockWidgets.indexOf(d->m_currentDockWidget);
}

void TabBar::setCurrentIndex(int index)
{
    if (d->m_removeGuard) // We're in the middle of a remove.
        return;

    auto newCurrentDw = dockWidgetAt(index);
    if (newCurrentDw == d->m_currentDockWidget)
        return;

    if (d->m_currentDockWidget) {
        d->m_currentDockWidget->d->isCurrentTabChanged.emit(false);
    }

    d->m_currentDockWidget = newCurrentDw;
    d->currentDockWidgetChanged.emit(newCurrentDw);
    if (auto tvi = dynamic_cast<Core::TabBarViewInterface *>(view()))
        tvi->setCurrentIndex(index);

    if (newCurrentDw)
        newCurrentDw->d->isCurrentTabChanged.emit(true);
}

void TabBar::renameTab(int index, const QString &text)
{
    if (auto tvi = dynamic_cast<Core::TabBarViewInterface *>(view()))
        tvi->renameTab(index, text);
}

void TabBar::changeTabIcon(int index, const Icon &icon)
{
    if (auto tvi = dynamic_cast<Core::TabBarViewInterface *>(view()))
        tvi->changeTabIcon(index, icon);
}

bool TabBar::isMovingTab() const
{
    return d->m_isMovingTab;
}

TabBar::Private *TabBar::dptr() const
{
    return d;
}
