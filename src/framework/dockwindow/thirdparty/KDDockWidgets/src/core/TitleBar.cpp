/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "TitleBar.h"
#include "TitleBar_p.h"
#include "Config.h"
#include "ViewFactory.h"
#include "View.h"
#include "WindowBeingDragged_p.h"
#include "Utils_p.h"
#include "Logging_p.h"
#include "Group_p.h"

#include "views/TitleBarViewInterface.h"
#include "DockWidget_p.h"
#include "FloatingWindow.h"
#include "DockRegistry.h"
#include "FloatingWindow_p.h"
#include "TabBar.h"
#include "MainWindow.h"
#include "MDILayout.h"
#include "Stack.h"

#ifdef KDDW_FRONTEND_QT
#include <QTimer>
#endif

#include <utility>

using namespace KDDockWidgets;
using namespace KDDockWidgets::Core;


TitleBar::TitleBar(Group *parent)
    : Controller(
          ViewType::TitleBar,
          Config::self().viewFactory()->createTitleBar(this, parent ? parent->view() : nullptr))
    , Draggable(view())
    , d(new Private())
    , m_group(parent)
    , m_floatingWindow(nullptr)
    , m_supportsAutoHide((Config::self().flags() & Config::Flag_AutoHideSupport) == Config::Flag_AutoHideSupport)
    , m_isStandalone(false)
{
    init();
    d->numDockWidgetsChangedConnection = m_group->dptr()->numDockWidgetsChanged.connect([this] {
        updateCloseButton();
        d->numDockWidgetsChanged.emit();
    });

    d->isFocusedChangedConnection = m_group->dptr()->isFocusedChanged.connect([this] {
        d->isFocusedChanged.emit();
    });

    d->isInMainWindowChangedConnection = m_group->dptr()->isInMainWindowChanged.connect([this] {
        updateAutoHideButton();
    });
}

TitleBar::TitleBar(FloatingWindow *parent)
    : Controller(
          ViewType::TitleBar,
          Config::self().viewFactory()->createTitleBar(this, parent ? parent->view() : nullptr))
    , Draggable(view())
    , d(new Private())
    , m_group(nullptr)
    , m_floatingWindow(parent)
    , m_supportsAutoHide((Config::self().flags() & Config::Flag_AutoHideSupport) == Config::Flag_AutoHideSupport)
    , m_isStandalone(false)
{
    init();
    auto fwPrivate = m_floatingWindow->dptr();
    fwPrivate->numGroupsChanged.connect([this] { updateButtons(); });
    fwPrivate->numDockWidgetsChanged.connect([this] { d->numDockWidgetsChanged.emit(); });
    fwPrivate->windowStateChanged.connect([this] { updateMaximizeButton(); });
    fwPrivate->activatedChanged.connect([this] { d->isFocusedChanged.emit(); });
}

TitleBar::TitleBar(Core::View *view)
    : Controller(ViewType::TitleBar, view)
    , Draggable(view, /*enabled=*/false)
    , d(new Private())
    , m_group(nullptr)
    , m_floatingWindow(nullptr)
    , m_supportsAutoHide(false)
    , m_isStandalone(true)
{
}

void TitleBar::init()
{
    view()->init();

    d->isFocusedChanged.connect([this] {
        // repaint
        view()->update();
    });

    updateButtons();

#ifdef KDDW_FRONTEND_QT
    // Auto-hide not supported in flutter yet
    QTimer::singleShot(0, this, &TitleBar::updateAutoHideButton); // have to wait after the group is
                                                                  // constructed
#endif
}

TitleBar::~TitleBar()
{
    delete d;
}

bool TitleBar::titleBarIsFocusable() const
{
    return Config::self().flags() & Config::Flag_TitleBarIsFocusable;
}


MainWindow *TitleBar::mainWindow() const
{
    if (m_floatingWindow || m_isStandalone)
        return nullptr;

    if (m_group)
        return m_group->mainWindow();

    KDDW_ERROR("null group and null floating window");
    return nullptr;
}

bool TitleBar::isMDI() const
{
    auto p = view()->asWrapper();
    while (p) {
        if (p->is(ViewType::MDILayout))
            return true;

        if (p->is(ViewType::DropArea)) {
            // Note that the TitleBar can be inside a DropArea that's inside a MDIArea
            // so we need this additional check
            return false;
        }

        p = p->parentView();
    }

    return false;
}

QString TitleBar::title() const
{
    return m_title;
}

Icon TitleBar::icon() const
{
    return m_icon;
}

bool TitleBar::onDoubleClicked()
{
    if ((Config::self().flags() & Config::Flag_DoubleClickMaximizes) && m_floatingWindow) {
        // Not using isFloating(), as that can be a dock widget nested in a floating window. By
        // convention it's floating, but it's not the title bar of the top-level window.
        toggleMaximized();
        return true;
    } else if (supportsFloatUnfloat()) {
        onFloatClicked();
        return true;
    }

    return false;
}

bool TitleBar::floatButtonVisible() const
{
    return m_floatButtonVisible;
}

bool TitleBar::maximizeButtonVisible() const
{
    return m_maximizeButtonVisible;
}

bool TitleBar::supportsFloatUnfloat() const
{
    if (m_isStandalone)
        return false; // not applicable

    if (DockWidget *dw = singleDockWidget()) {
        // Don't show the dock/undock button if the window is not dockable
        if (dw->options() & DockWidgetOption_NotDockable)
            return false;
    }

    // If we have a floating window with nested dock widgets we can't re-attach, because we don't
    // know where to
    return !m_floatingWindow || m_floatingWindow->hasSingleGroup();
}

bool TitleBar::supportsFloatingButton() const
{
    auto flags = Config::self().flags();
    if (flags & Config::Flag_TitleBarHasMaximizeButton) {
        // Apps having a maximize/restore button traditionally don't have a floating one,
        // QDockWidget style only has floating and no maximize/restore.
        // We can add an option later if we need them to co-exist
        return false;
    }

    if (flags & Config::Flag_TitleBarNoFloatButton) {
        // Was explicitly disabled
        return false;
    }

    return supportsFloatUnfloat();
}

bool TitleBar::supportsMaximizeButton() const
{
    return m_floatingWindow && m_floatingWindow->supportsMaximizeButton();
}

bool TitleBar::supportsMinimizeButton() const
{
    return m_floatingWindow && m_floatingWindow->supportsMinimizeButton();
}

bool TitleBar::supportsAutoHideButton() const
{
    // Only dock widgets docked into the MainWindow can minimize
    return m_supportsAutoHide && m_group && (m_group->isInMainWindow() || m_group->isOverlayed());
}

#ifdef DOCKS_TESTING_METHODS
bool TitleBar::isFloatButtonVisible() const
{
    return dynamic_cast<Core::TitleBarViewInterface *>(view())->isFloatButtonVisible();
}

bool TitleBar::isCloseButtonVisible() const
{
    return dynamic_cast<Core::TitleBarViewInterface *>(view())->isCloseButtonVisible();
}

bool TitleBar::isCloseButtonEnabled() const
{
    return dynamic_cast<Core::TitleBarViewInterface *>(view())->isCloseButtonEnabled();
}
#endif

bool TitleBar::hasIcon() const
{
    return !m_icon.isNull();
}

Core::Group *TitleBar::group() const
{
    return m_group;
}

Core::FloatingWindow *TitleBar::floatingWindow() const
{
    return m_floatingWindow;
}

void TitleBar::focus(Qt::FocusReason reason)
{
    if (!(Config::self().flags() & Config::Flag_TitleBarIsFocusable))
        return;

    if (m_group) {
        m_group->FocusScope::focus(reason);
    } else if (m_floatingWindow) {
        m_floatingWindow->focus(reason);
    }
}

void TitleBar::updateButtons()
{
    updateCloseButton();
    updateFloatButton();
    updateMaximizeButton();

    const bool isEnabled = true;
    const bool minimizeVisible = supportsMinimizeButton() && !buttonIsUserHidden(TitleBarButtonType::Minimize, isEnabled);
    d->minimizeButtonChanged.emit(minimizeVisible, isEnabled);

    updateAutoHideButton();
}

void TitleBar::updateAutoHideButton()
{
    TitleBarButtonType type = TitleBarButtonType::AutoHide;

    if (const Core::Group *group = this->group()) {
        if (group->isOverlayed())
            type = TitleBarButtonType::UnautoHide;
    }

    const bool isEnabled = true;
    const bool visible = m_supportsAutoHide && !buttonIsUserHidden(type, isEnabled) && !m_floatingWindow;
    d->autoHideButtonChanged.emit(visible, isEnabled, type);
}

void TitleBar::updateMaximizeButton()
{
    m_maximizeButtonVisible = false;
    m_maximizeButtonType = TitleBarButtonType::Maximize;

    if (auto fw = floatingWindow()) {
        m_maximizeButtonType =
            fw->view()->isMaximized() ? TitleBarButtonType::Normal : TitleBarButtonType::Maximize;
        m_maximizeButtonVisible = supportsMaximizeButton();
    }
    const bool isEnabled = true;
    m_maximizeButtonVisible = m_maximizeButtonVisible && !buttonIsUserHidden(m_maximizeButtonType, isEnabled);
    d->maximizeButtonChanged.emit(m_maximizeButtonVisible, isEnabled, m_maximizeButtonType);
}

void TitleBar::updateCloseButton()
{
    const bool anyNonClosable = group()
        ? group()->anyNonClosable()
        : (floatingWindow() ? floatingWindow()->anyNonClosable() : false);

    const bool isEnabled = !anyNonClosable;
    setCloseButtonEnabled(isEnabled);
    setCloseButtonVisible(!buttonIsUserHidden(TitleBarButtonType::Close, isEnabled));
}

void TitleBar::toggleMaximized()
{
    if (!m_floatingWindow)
        return;

    if (m_floatingWindow->view()->isMaximized())
        m_floatingWindow->view()->showNormal();
    else
        m_floatingWindow->view()->showMaximized();
}

bool TitleBar::isOverlayed() const
{
    return m_group && m_group->isOverlayed();
}

void TitleBar::setCloseButtonEnabled(bool enabled)
{
    if (enabled != m_closeButtonEnabled) {
        m_closeButtonEnabled = enabled;
        d->closeButtonChanged.emit(m_closeButtonVisible, enabled);
    }
}

void TitleBar::setCloseButtonVisible(bool visible)
{
    if (visible != m_closeButtonVisible) {
        m_closeButtonVisible = visible;
        d->closeButtonChanged.emit(m_closeButtonVisible, m_closeButtonEnabled);
    }
}

void TitleBar::setFloatButtonVisible(bool visible)
{
    if (visible != m_floatButtonVisible) {
        m_floatButtonVisible = visible;
        d->floatButtonVisibleChanged.emit(visible);
    }
}

void TitleBar::setFloatButtonToolTip(const QString &tip)
{
    if (tip != m_floatButtonToolTip) {
        m_floatButtonToolTip = tip;
        d->floatButtonToolTipChanged.emit(tip);
    }
}

void TitleBar::setTitle(const QString &title)
{
    if (title != m_title) {
        m_title = title;
        view()->update();
        d->titleChanged.emit();
    }
}

void TitleBar::setIcon(const Icon &icon)
{
    m_icon = icon;
    d->iconChanged.emit();
}

void TitleBar::onCloseClicked()
{
    CloseReasonSetter reason(CloseReason::TitleBarCloseButton);

    const bool closeOnlyCurrentTab = Config::self().flags() & Config::Flag_CloseOnlyCurrentTab;

    if (m_group) {
        if (closeOnlyCurrentTab) {
            if (auto dw = m_group->currentDockWidget()) {
                dw->view()->close();
            } else {
                // Doesn't happen
                KDDW_ERROR("Group with no dock widgets");
            }
        } else {
            if (m_group->isTheOnlyGroup() && m_group->isInFloatingWindow()) {
                m_group->view()->d->closeRootView();
            } else {
                m_group->view()->close();
            }
        }
    } else if (m_floatingWindow) {

        if (closeOnlyCurrentTab) {
            if (Group *f = m_floatingWindow->singleFrame()) {
                if (DockWidget *dw = f->currentDockWidget()) {
                    dw->view()->close();
                } else {
                    // Doesn't happen
                    KDDW_ERROR("Group with no dock widgets");
                }
            } else {
                m_floatingWindow->view()->close();
            }
        } else {
            m_floatingWindow->view()->close();
        }
    } else if (m_isStandalone) {
        view()->d->closeRootView();
    }
}

void TitleBar::onFloatClicked()
{
    const DockWidget::List dockWidgets = this->dockWidgets();
    if (isFloating()) {
        // Let's dock it

        if (dockWidgets.isEmpty()) {
            KDDW_ERROR("TitleBar::onFloatClicked: empty list. Shouldn't happen");
            return;
        }

        if (dockWidgets.size() == 1) {
            // Case 1: Single dockwidget floating
            dockWidgets[0]->setFloating(false);
        } else {
            // Case 2: Multiple dockwidgets are tabbed together and floating
            // Possible improvement: Just reuse the whole group and put it back.
            // The group currently doesn't remember the position in the main window
            // so use an hack for now

            if (!dockWidgets.isEmpty()) { // could be empty during destruction, maybe
                if (!dockWidgets.constFirst()->hasPreviousDockedLocation()) {
                    // Don't attempt, there's no previous docked location
                    return;
                }

                // suppress "isFloatingChanged" signals, as we're doing the float/unfloat hack
                Group::s_inFloatHack = true;

                int i = 0;
                DockWidget *current = nullptr;
                for (auto dock : std::as_const(dockWidgets)) {
                    if (!current && dock->isCurrentTab())
                        current = dock;

                    dock->setFloating(true);
                    dock->dptr()->m_lastPosition->m_tabIndex = i;
                    dock->setFloating(false);
                    ++i;
                }
                Group::s_inFloatHack = false;

                // Restore the current tab
                if (current)
                    current->setAsCurrentTab();
            }
        }
    } else {
        // Let's float it
        if (dockWidgets.size() == 1) {
            // If there's a single dock widget, just call DockWidget::setFloating(true). The only
            // difference is that it has logic for using the last used geometry for the floating
            // window
            dockWidgets[0]->setFloating(true);
        } else {
            makeWindow();
        }
    }
}

void TitleBar::onMaximizeClicked()
{
    toggleMaximized();
}

void TitleBar::onMinimizeClicked()
{
    if (!m_floatingWindow)
        return;

    if (m_floatingWindow->isUtilityWindow()) {
        // Qt::Tool windows don't appear in the task bar.
        // Unless someone tells me a good reason to allow this situation.
        return;
    }

    m_floatingWindow->view()->showMinimized();
}

void TitleBar::onAutoHideClicked()
{
    if (!m_group) {
        // Doesn't happen
        KDDW_ERROR("Minimize not supported on floating windows");
        return;
    }

    const auto &dockwidgets = m_group->dockWidgets();
    if (isOverlayed() && dockwidgets.size() != 1) {
        // Doesn't happen
        KDDW_ERROR("TitleBar::onAutoHideClicked: There can only be a single dock widget per titlebar overlayed");
        return;
    }

    const bool groupedAutoHide = Config::hasFlag(Config::Flag_AutoHideAsTabGroups);
    const auto currentDw = m_group->currentDockWidget();
    auto registry = DockRegistry::self();

    if (isOverlayed()) { // Restore it:
        auto dw = dockwidgets.first();
        MainWindow *mainWindow = dw->mainWindow();
        auto sideBarGroup = groupedAutoHide ? registry->sideBarGroupingFor(dw) : DockWidget::List();
        if (sideBarGroup.isEmpty()) {
            mainWindow->restoreFromSideBar(dw);
        } else {
            // Config::Flag_AutoHideAsTabGroups case. Restore its friends too
            for (auto it = sideBarGroup.rbegin(); it != sideBarGroup.rend(); ++it) {
                mainWindow->restoreFromSideBar(*it);
            }
            dw->setAsCurrentTab();
            registry->removeSideBarGrouping(sideBarGroup);
        }
    } else { // Send it to sidebar:
        if (groupedAutoHide)
            registry->addSideBarGrouping(dockwidgets);


        CloseReasonSetter reason(CloseReason::MovedToSideBar);
        for (DockWidget *dw : dockwidgets) {
            if (groupedAutoHide || dw == currentDw)
                dw->moveToSideBar();
        }
    }
}

bool TitleBar::closeButtonEnabled() const
{
    return m_closeButtonEnabled;
}

std::unique_ptr<WindowBeingDragged> TitleBar::makeWindow()
{
    if (m_isStandalone)
        return {}; // not applicable

    if (!isVisible() && view()->rootView()->controller()->isVisible()
        && !(Config::self().flags() & Config::Flag_ShowButtonsOnTabBarIfTitleBarHidden)) {

        // When using Flag_ShowButtonsOnTabBarIfTitleBarHidden we forward the call from the tab
        // bar's buttons to the title bar's buttons, just to reuse logic

        KDDW_ERROR("TitleBar::makeWindow shouldn't be called on invisible title bar this={}, root.isVisible={}", ( void * )this, view()->rootView()->isVisible());
        if (m_group) {
            KDDW_ERROR("this={}; actual={}", ( void * )this, ( void * )m_group->actualTitleBar());
        } else if (m_floatingWindow) {
            KDDW_ERROR("Has floating window with titlebar={}, isVisible={}", ( void * )m_floatingWindow->titleBar(), m_floatingWindow->isVisible());
        }

        assert(false);
        return {};
    }

    if (m_floatingWindow) {
        // We're already a floating window, no detach needed
        return std::make_unique<WindowBeingDragged>(m_floatingWindow, this);
    }

    if (FloatingWindow *fw = floatingWindow()) { // Already floating
        if (m_group->isTheOnlyGroup()) { // We don't detach. This one drags the entire window
                                         // instead.
            KDDW_DEBUG("TitleBar::makeWindow no detach needed");
            return std::make_unique<WindowBeingDragged>(fw, this);
        }
    }

    Rect r = m_group->view()->geometry();
    r.moveTopLeft(m_group->mapToGlobal(Point(0, 0)));

    auto floatingWindow = new Core::FloatingWindow(m_group, {});
    floatingWindow->setSuggestedGeometry(r, SuggestedGeometryHint_GeometryIsFromDocked);
    floatingWindow->view()->show();

    auto draggable = KDDockWidgets::usesNativeTitleBar() ? static_cast<Draggable *>(floatingWindow)
                                                         : static_cast<Draggable *>(this);
    return std::make_unique<WindowBeingDragged>(floatingWindow, draggable);
}

bool TitleBar::isWindow() const
{
    return m_floatingWindow != nullptr;
}

Core::DockWidget::List TitleBar::dockWidgets() const
{
    if (m_floatingWindow) {
        DockWidget::List result;
        const auto groups = m_floatingWindow->groups();
        for (Group *group : groups) {
            result.append(group->dockWidgets());
        }
        return result;
    }

    if (m_group)
        return m_group->dockWidgets();

    if (m_isStandalone)
        return {}; // not applicable

    KDDW_ERROR("TitleBar::dockWidget: shouldn't happen");
    return {};
}

Core::DockWidget *TitleBar::singleDockWidget() const
{
    const DockWidget::List dockWidgets = this->dockWidgets();
    return dockWidgets.isEmpty() ? nullptr : dockWidgets.first();
}

bool TitleBar::isFloating() const
{
    if (m_floatingWindow)
        return true;

    if (m_group)
        return m_group->isFloating();

    if (m_isStandalone)
        return false; // not applicable

    KDDW_ERROR("TitleBar::isFloating: shouldn't happen");
    return false;
}

bool TitleBar::isFocused() const
{
    if (m_group)
        return m_group->isFocused();
    else if (m_floatingWindow)
        return m_floatingWindow->view()->isActiveWindow();
    else if (m_isStandalone)
        return view()->isActiveWindow();

    return false;
}

void TitleBar::updateFloatButton()
{
    setFloatButtonToolTip(floatingWindow() ? tr("Dock window") : tr("Undock window"));
    setFloatButtonVisible(supportsFloatingButton() && !buttonIsUserHidden(TitleBarButtonType::Float, /*enabled=*/true));
}

QString TitleBar::floatButtonToolTip() const
{
    return m_floatButtonToolTip;
}

TabBar *TitleBar::tabBar() const
{
    if (m_floatingWindow && m_floatingWindow->hasSingleGroup()) {
        if (Group *group = m_floatingWindow->singleFrame()) {
            return group->stack()->tabBar();
        } else {
            // Shouldn't happen
            KDDW_ERROR("Expected a group");
        }

    } else if (m_group) {
        return m_group->stack()->tabBar();
    }

    return nullptr;
}

TitleBarButtonType TitleBar::maximizeButtonType() const
{
    return m_maximizeButtonType;
}

bool TitleBar::isStandalone() const
{
    return m_isStandalone;
}

bool TitleBar::buttonIsUserHidden(TitleBarButtonType type) const
{
    return d->m_userHiddenButtonTypes & type;
}

bool TitleBar::buttonIsUserHidden(TitleBarButtonType type, bool enabled) const
{
    if (buttonIsUserHidden(type))
        return true;

    if (!enabled)
        return buttonHidesIfDisabled(type);

    return false;
}

void TitleBar::setUserHiddenButtons(TitleBarButtonTypes types)
{
    if (d->m_userHiddenButtonTypes != types) {
        d->m_userHiddenButtonTypes = types;
        updateButtons();
    }
}

TitleBar::Private *TitleBar::dptr() const
{
    return d;
}

void TitleBar::setHideDisabledButtons(TitleBarButtonTypes types)
{
    if (d->m_buttonsToHideIfDisabled != types) {
        d->m_buttonsToHideIfDisabled = types;
        updateButtons();
    }
}

bool TitleBar::buttonHidesIfDisabled(TitleBarButtonType type) const
{
    return d->m_buttonsToHideIfDisabled & type;
}
