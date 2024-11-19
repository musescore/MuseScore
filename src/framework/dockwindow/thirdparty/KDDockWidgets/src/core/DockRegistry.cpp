/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "DockRegistry.h"
#include "DockRegistry_p.h"
#include "DelayedCall_p.h"
#include "Config.h"
#include "core/Logging_p.h"
#include "core/Position_p.h"
#include "core/Utils_p.h"
#include "core/Platform_p.h"
#include "core/WidgetResizeHandler_p.h"
#include "core/WindowBeingDragged_p.h"
#include "core/layouting/Item_p.h"
#include "core/layouting/LayoutingHost_p.h"
#include "core/DockWidget_p.h"
#include "core/ObjectGuard_p.h"
#include "core/views/MainWindowViewInterface.h"
#include "core/FloatingWindow.h"
#include "core/SideBar.h"
#include "core/MainWindow.h"
#include "core/DockWidget.h"
#include "core/DropArea.h"
#include "core/Platform.h"
#include "core/Window_p.h"

#include "kdbindings/signal.h"

#include <set>
#include <utility>

using namespace KDDockWidgets;
using namespace KDDockWidgets::Core;

namespace KDDockWidgets::Core {

// Helper class to help implement Config::Flag_AutoHideAsTabGroups
class SideBarGroupings
{
public:
    void addGrouping(const DockWidget::List &);
    void removeGrouping(const DockWidget::List &);
    DockWidget::List groupingFor(DockWidget *) const;
    void removeFromGroupings(DockWidget *);

private:
    DockWidget::List &groupingByRef(DockWidget *);
    Vector<DockWidget::List> m_groupings;
};

}

DockRegistry::DockRegistry(Core::Object *parent)
    : Core::Object(parent)
    , d(new Private())
    , m_sideBarGroupings(new SideBarGroupings())
{
    Platform::instance()->installGlobalEventFilter(this);

    d->m_connection = Platform::instance()->d->focusedViewChanged.connect(
        &DockRegistry::onFocusedViewChanged, this);
}

DockRegistry::~DockRegistry()
{
    delete m_sideBarGroupings;
    Platform::instance()->removeGlobalEventFilter(this);
    d->m_connection.disconnect();
    delete d;
}

void DockRegistry::maybeDelete()
{
    // We delete the singleton just to make LSAN happy.
    // We could also simply ask the user do call something like KDDockWidgets::deinit() in the future,
    // Also, please don't change this to be deleted at static dtor time with Q_GLOBAL_STATIC.
    if (isEmpty() && d->m_numLayoutSavers == 0 && m_groups.isEmpty())
        delete this;
}

void DockRegistry::onFocusedViewChanged(std::shared_ptr<View> view)
{
    auto p = view;
    while (p && !p->isNull()) {
        if (auto group = p->asGroupController()) {
            // Special case: The focused widget is inside the group but not inside the dockwidget.
            // For example, it's a line edit in the QTabBar. We still need to send the signal for
            // the current dw in the tab group
            if (auto dw = group->currentDockWidget()) {
                setFocusedDockWidget(dw);
            }

            return;
        }

        if (auto dw = p->asDockWidgetController()) {
            DockRegistry::self()->setFocusedDockWidget(dw);
            return;
        }
        p = p->parentView();
    }

    setFocusedDockWidget(nullptr);
}

void DockRegistry::setFocusedDockWidget(Core::DockWidget *dw)
{
    if (d->m_focusedDockWidget.data() == dw)
        return;

    auto old = d->m_focusedDockWidget;
    d->m_focusedDockWidget = dw;

    if (old)
        old->d->isFocusedChanged.emit(false);

    if (dw)
        dw->d->isFocusedChanged.emit(true);
}

bool DockRegistry::isEmpty(bool excludeBeingDeleted) const
{
    if (!m_dockWidgets.isEmpty() || !m_mainWindows.isEmpty())
        return false;

    return excludeBeingDeleted ? !hasFloatingWindows() : m_floatingWindows.isEmpty();
}

bool DockRegistry::affinitiesMatch(const Vector<QString> &affinities1,
                                   const Vector<QString> &affinities2) const
{
    if (affinities1.isEmpty() && affinities2.isEmpty())
        return true;

    for (const QString &a1 : affinities1) {
        for (const QString &a2 : affinities2) {
            if (a1 == a2)
                return true;
        }
    }

    return false;
}

Vector<QString> DockRegistry::mainWindowsNames() const
{
    Vector<QString> names;
    names.reserve(m_mainWindows.size());
    for (auto mw : m_mainWindows)
        names.push_back(mw->uniqueName());

    return names;
}

Vector<QString> DockRegistry::dockWidgetNames() const
{
    Vector<QString> names;
    names.reserve(m_dockWidgets.size());
    for (auto dw : m_dockWidgets)
        names.push_back(dw->uniqueName());

    return names;
}

bool DockRegistry::isProbablyObscured(Core::Window::Ptr window,
                                      Core::FloatingWindow *exclude) const
{
    if (!window)
        return false;

    const Rect geo = window->geometry();
    for (Core::FloatingWindow *fw : m_floatingWindows) {
        Window::Ptr fwWindow = fw->view()->window();
        if (fw == exclude || fwWindow->equals(window))
            continue;

        if (fwWindow->geometry().intersects(geo)) {
            // fw might be below, but we don't have a way to check. So be conservative and return
            // true.
            return true;
        }
    }

    // Floating windows are Tool (keep above), unless we disabled it in Config
    auto fw = floatingWindowForHandle(window);
    const bool targetIsToolWindow =
        fw && fw->isUtilityWindow();

    for (Core::MainWindow *mw : m_mainWindows) {
        Window::Ptr mwWindow = mw->view()->window();

        if (mwWindow && !mwWindow->equals(window) && !targetIsToolWindow
            && mwWindow->geometry().intersects(geo)) {
            // Two main windows that intersect. Return true. If the target is a tool window it will
            // be above, so we don't care.
            return true;
        }
    }

    return false;
}

bool DockRegistry::isProbablyObscured(Core::Window::Ptr target, WindowBeingDragged *exclude) const
{
    Core::FloatingWindow *fw =
        exclude ? exclude->floatingWindow() : nullptr; // It's null on Wayland. On wayland obscuring
                                                       // never happens anyway, so not a problem.

    return isProbablyObscured(target, fw);
}

SideBarLocation DockRegistry::sideBarLocationForDockWidget(const Core::DockWidget *dw) const
{
    if (Core::SideBar *sb = sideBarForDockWidget(dw))
        return sb->location();

    return SideBarLocation::None;
}

Core::SideBar *DockRegistry::sideBarForDockWidget(const Core::DockWidget *dw) const
{
    for (auto mw : m_mainWindows) {
        if (Core::SideBar *sb = mw->sideBarForDockWidget(dw))
            return sb;
    }

    return nullptr;
}

Core::Group *DockRegistry::groupInMDIResize() const
{
    for (auto mw : m_mainWindows) {
        if (!mw->isMDI())
            continue;

        Layout *layout = mw->layout();
        const Vector<Core::Group *> groups = layout->groups();
        for (Core::Group *group : groups) {
            if (WidgetResizeHandler *wrh = group->resizeHandler()) {
                if (wrh->isResizing())
                    return group;
            }
        }
    }

    return nullptr;
}

void DockRegistry::setCurrentCloseReason(CloseReason reason)
{
    d->m_currentCloseReason = reason;
}

CloseReason DockRegistry::currentCloseReason()
{
    return d->m_currentCloseReason;
}

DockRegistry::Private *DockRegistry::dptr() const
{
    return d;
}

Core::MainWindow::List
DockRegistry::mainWindowsWithAffinity(const Vector<QString> &affinities) const
{
    Core::MainWindow::List result;
    result.reserve(m_mainWindows.size());

    for (auto mw : m_mainWindows) {
        const Vector<QString> mwAffinities = mw->affinities();
        if (affinitiesMatch(mwAffinities, affinities))
            result.push_back(mw);
    }

    return result;
}

Core::Layout *DockRegistry::layoutForItem(const Item *item) const
{
    return Layout::fromLayoutingHost(item->host());
}

bool DockRegistry::itemIsInMainWindow(const Item *item) const
{
    if (Core::Layout *layout = layoutForItem(item)) {
        return layout->isInMainWindow(/*honourNesting=*/true);
    }

    return false;
}

DockRegistry *DockRegistry::self()
{
    static ObjectGuard<DockRegistry> s_dockRegistry;

    if (!s_dockRegistry) {
        s_dockRegistry = new DockRegistry();
    }

    return s_dockRegistry;
}

void DockRegistry::registerDockWidget(Core::DockWidget *dock)
{
    if (dock->uniqueName().isEmpty()) {
        KDDW_ERROR("DockWidget doesn't have an ID");
    } else if (auto other = dockByName(dock->uniqueName())) {
        KDDW_ERROR("Another DockWidget {} with name {} already exists.", ( void * )other, dock->uniqueName(), ( void * )dock);
    }

    m_dockWidgets.push_back(dock);
}

void DockRegistry::unregisterDockWidget(Core::DockWidget *dock)
{
    if (d->m_focusedDockWidget == dock)
        d->m_focusedDockWidget = nullptr;

    m_dockWidgets.removeOne(dock);
    m_sideBarGroupings->removeFromGroupings(dock);

    maybeDelete();
}

void DockRegistry::registerMainWindow(Core::MainWindow *mainWindow)
{
    if (mainWindow->uniqueName().isEmpty()) {
        KDDW_ERROR("MainWindow doesn't have an ID");
    } else if (auto other = mainWindowByName(mainWindow->uniqueName())) {
        KDDW_ERROR("Another MainWindow {} with name {} already exists {}", ( void * )other, mainWindow->uniqueName(), ( void * )mainWindow);
    }

    m_mainWindows.push_back(mainWindow);
    Platform::instance()->onMainWindowCreated(mainWindow);
}

void DockRegistry::unregisterMainWindow(Core::MainWindow *mainWindow)
{
    m_mainWindows.removeOne(mainWindow);
    Platform::instance()->onMainWindowDestroyed(mainWindow);
    maybeDelete();
}

void DockRegistry::registerFloatingWindow(Core::FloatingWindow *fw)
{
    m_floatingWindows.push_back(fw);
    Platform::instance()->onFloatingWindowCreated(fw);
}

void DockRegistry::unregisterFloatingWindow(Core::FloatingWindow *fw)
{
    m_floatingWindows.removeOne(fw);
    Platform::instance()->onFloatingWindowDestroyed(fw);
    maybeDelete();
}

void DockRegistry::registerGroup(Core::Group *group)
{
    m_groups.push_back(group);
}

void DockRegistry::unregisterGroup(Core::Group *group)
{
    m_groups.removeOne(group);
    maybeDelete();
}

void DockRegistry::registerLayoutSaver()
{
    d->m_numLayoutSavers++;
}

void DockRegistry::unregisterLayoutSaver()
{
    d->m_numLayoutSavers--;
    maybeDelete();
}

Core::DockWidget *DockRegistry::focusedDockWidget() const
{
    return d->m_focusedDockWidget;
}

bool DockRegistry::containsDockWidget(const QString &uniqueName) const
{
    return dockByName(uniqueName) != nullptr;
}

bool DockRegistry::containsMainWindow(const QString &uniqueName) const
{
    return mainWindowByName(uniqueName) != nullptr;
}

Core::DockWidget *DockRegistry::dockByName(const QString &name, DockByNameFlags flags) const
{
    for (auto dock : std::as_const(m_dockWidgets)) {
        if (dock->uniqueName() == name)
            return dock;
    }

    if (flags.testFlag(DockByNameFlag::ConsultRemapping)) {
        // Name doesn't exist, let's check if it was remapped during a layout restore.
        auto it = m_dockWidgetIdRemapping.find(name);
        const QString newName = it == m_dockWidgetIdRemapping.cend() ? QString() : it->second;
        if (!newName.isEmpty())
            return dockByName(newName);
    }

    if (flags.testFlag(DockByNameFlag::CreateIfNotFound)) {
        // DockWidget doesn't exist, ask to create it
        if (auto factoryFunc = Config::self().dockWidgetFactoryFunc()) {
            auto dw = factoryFunc(name);
            if (dw && dw->uniqueName() != name) {
                // Very special case
                // The user's factory function returned a dock widget with a different ID.
                // We support it. Save the mapping though.
                m_dockWidgetIdRemapping[name] = dw->uniqueName();
            }
            return dw;
        } else if (!flags.testFlag(DockByNameFlag::SilentIfNotFound)) {
            KDDW_ERROR("Couldn't find dock widget named={}", name);
        }
    }

    return nullptr;
}

Core::MainWindow *DockRegistry::mainWindowByName(const QString &name) const
{
    for (auto mainWindow : std::as_const(m_mainWindows)) {
        if (mainWindow->uniqueName() == name)
            return mainWindow;
    }

    return nullptr;
}

bool DockRegistry::isSane() const
{
    std::set<QString> names;
    for (auto dock : std::as_const(m_dockWidgets)) {
        const QString name = dock->uniqueName();
        if (name.isEmpty()) {
            KDDW_ERROR("DockRegistry::isSane: DockWidget is missing a name");
            return false;
        } else if (names.find(name) != names.cend()) {
            KDDW_ERROR("DockRegistry::isSane: dockWidgets with duplicate names: {}", name);
            return false;
        } else {
            names.insert(name);
        }
    }

    names.clear();
    for (auto mainwindow : std::as_const(m_mainWindows)) {
        const QString name = mainwindow->uniqueName();
        if (name.isEmpty()) {
            KDDW_ERROR("DockRegistry::isSane: MainWindow is missing a name");
            return false;
        } else if (names.find(name) != names.cend()) {
            KDDW_ERROR("DockRegistry::isSane: mainWindow with duplicate names: {}", name);
            return false;
        } else {
            names.insert(name);
        }

        if (!mainwindow->layout()->checkSanity())
            return false;
    }

    return true;
}

Core::DockWidget::List DockRegistry::dockwidgets() const
{
    return m_dockWidgets;
}

Core::DockWidget::List DockRegistry::dockWidgets(const Vector<QString> &names)
{
    Core::DockWidget::List result;
    result.reserve(names.size());

    for (auto dw : std::as_const(m_dockWidgets)) {
        if (names.contains(dw->uniqueName()))
            result.push_back(dw);
    }

    return result;
}

Core::MainWindow::List DockRegistry::mainWindows(const Vector<QString> &names)
{
    Core::MainWindow::List result;
    result.reserve(names.size());

    for (auto mw : std::as_const(m_mainWindows)) {
        if (names.contains(mw->uniqueName()))
            result.push_back(mw);
    }

    return result;
}

::DockWidget::List DockRegistry::closedDockwidgets(bool honourSkipped) const
{
    Core::DockWidget::List result;
    result.reserve(m_dockWidgets.size());

    for (Core::DockWidget *dw : m_dockWidgets) {
        const bool shouldSkip = honourSkipped && (dw->layoutSaverOptions() & LayoutSaverOption::Skip);
        if (!shouldSkip && dw->parent() == nullptr && !dw->isVisible())
            result.push_back(dw);
    }

    return result;
}

Core::MainWindow::List DockRegistry::mainwindows() const
{
    return m_mainWindows;
}

Vector<Core::MainWindowViewInterface *> DockRegistry::mainDockingAreas() const
{
    Vector<Core::MainWindowViewInterface *> areas;

    for (auto mw : m_mainWindows) {
        if (View *view = mw->view()) {
            auto viewInterface = dynamic_cast<Core::MainWindowViewInterface *>(view);
            areas.push_back(viewInterface);
        }
    }

    return areas;
}

Core::Group::List DockRegistry::groups() const
{
    return m_groups;
}

Vector<Core::FloatingWindow *> DockRegistry::floatingWindows(bool includeBeingDeleted, bool honourSkipped) const
{
    // Returns all the FloatingWindow which aren't being deleted
    Vector<Core::FloatingWindow *> result;
    result.reserve(m_floatingWindows.size());
    for (Core::FloatingWindow *fw : m_floatingWindows) {
        if (!includeBeingDeleted && fw->beingDeleted())
            continue;

        if (honourSkipped && fw->allDockWidgetsHave(LayoutSaverOption::Skip))
            continue;

        result.push_back(fw);
    }

    return result;
}

Window::List DockRegistry::floatingQWindows() const
{
    Window::List windows;
    windows.reserve(m_floatingWindows.size());
    for (Core::FloatingWindow *fw : m_floatingWindows) {
        if (!fw->beingDeleted()) {
            if (Core::Window::Ptr window = fw->view()->window()) {
                windows.push_back(window);
            } else {
                KDDW_ERROR("FloatingWindow doesn't have QWindow");
            }
        }
    }

    return windows;
}

bool DockRegistry::hasFloatingWindows() const
{
    return std::any_of(m_floatingWindows.begin(), m_floatingWindows.end(),
                       [](Core::FloatingWindow *fw) { return !fw->beingDeleted(); });
}

Core::FloatingWindow *DockRegistry::floatingWindowForHandle(Core::Window::Ptr windowHandle) const
{
    for (Core::FloatingWindow *fw : m_floatingWindows) {
        if (fw->view()->window()->equals(windowHandle))
            return fw;
    }

    return nullptr;
}

Core::FloatingWindow *DockRegistry::floatingWindowForHandle(WId hwnd) const
{
    for (Core::FloatingWindow *fw : m_floatingWindows) {
        Window::Ptr window = fw->view()->window();
        if (window && window->handle() == hwnd)
            return fw;
    }

    return nullptr;
}

Core::MainWindow *DockRegistry::mainWindowForHandle(Core::Window::Ptr window) const
{
    if (!window)
        return nullptr;

    for (Core::MainWindow *mw : m_mainWindows) {
        if (mw->view()->d->isInWindow(window))
            return mw;
    }

    return nullptr;
}

Window::List DockRegistry::topLevels(bool excludeFloatingDocks) const
{
    Window::List windows;
    windows.reserve(m_floatingWindows.size() + m_mainWindows.size());

    if (!excludeFloatingDocks) {
        for (Core::FloatingWindow *fw : m_floatingWindows) {
            if (fw->isVisible()) {
                if (Core::Window::Ptr window = fw->view()->window()) {
                    windows.push_back(window);
                } else {
                    KDDW_ERROR("FloatingWindow doesn't have QWindow");
                }
            }
        }
    }

    for (Core::MainWindow *m : m_mainWindows) {
        if (m->isVisible()) {
            if (Core::Window::Ptr window = m->view()->window()) {
                windows.push_back(window);
            } else {
                KDDW_ERROR("MainWindow doesn't have QWindow");
            }
        }
    }

    return windows;
}

void DockRegistry::clear(const Vector<QString> &affinities)
{
    // Clears everything
    clear(m_dockWidgets, m_mainWindows, affinities);
}

void DockRegistry::clear(const Core::DockWidget::List &dockWidgets,
                         const Core::MainWindow::List &mainWindows,
                         const Vector<QString> &affinities)
{
    for (auto dw : std::as_const(dockWidgets)) {
        if (affinities.isEmpty() || affinitiesMatch(affinities, dw->affinities())) {
            dw->forceClose();
            dw->d->lastPosition()->removePlaceholders();
        }
    }

    for (auto mw : std::as_const(mainWindows)) {
        if (affinities.isEmpty() || affinitiesMatch(affinities, mw->affinities())) {
            mw->layout()->clearLayout();
        }
    }
}

void DockRegistry::ensureAllFloatingWidgetsAreMorphed()
{
    for (Core::DockWidget *dw : std::as_const(m_dockWidgets)) {
        if (dw->view()->rootView()->equals(dw->view()) && dw->isVisible())
            dw->d->morphIntoFloatingWindow();
    }
}

bool DockRegistry::onMouseButtonPress(View *view, MouseEvent *event)
{
    if (!view)
        return false;

    if (!Config::hasMDIFlag(Config::MDIFlag_NoClickToRaise)) {
        // When clicking on a MDI Group we raise the window
        if (Controller *c = view->d->firstParentOfType(ViewType::Group)) {
            auto group = static_cast<Group *>(c);
            if (group->isMDI())
                group->view()->raise();
        }
    }

    // The following code is for hididng the overlay
    if (!(Config::self().flags() & Config::Flag_AutoHideSupport))
        return false;

    if (view->is(ViewType::Group)) {
        // break recursion
        return false;
    }

    auto p = view->asWrapper();
    while (p) {
        if (auto dw = p->asDockWidgetController())
            return onDockWidgetPressed(dw, event);

        if (auto layout = p->asLayout()) {
            if (auto mw = layout->mainWindow()) {
                // The user clicked somewhere in the main window's drop area, but outside of the
                // overlayed dock widget
                mw->clearSideBarOverlay();
                return false;
            }
        }

        p = p->parentView();
    }


    return false;
}

bool DockRegistry::onDockWidgetPressed(Core::DockWidget *dw, MouseEvent *ev)
{
    // Here we implement "auto-hide". If there's a overlayed dock widget, we hide it if some other
    // dock widget is clicked.

    // Don't be sending mouse events around if a popup is open, they are sensitive
    if (Platform::instance()->hasActivePopup())
        return false;

    Core::MainWindow *mainWindow = dw->mainWindow();
    if (!mainWindow) // Only docked widgets are interesting
        return false;

    if (Core::DockWidget *overlayedDockWidget = mainWindow->overlayedDockWidget()) {
        ev->ignore();
        Platform::instance()->sendEvent(overlayedDockWidget->d->group()->view(), ev);

        if (ev->isAccepted()) {
            // The Frame accepted it. It means the user is resizing it. We allow for 4px outside for
            // better resize.
            return true; // don't propagate the event further
        }
        if (dw != overlayedDockWidget) {
            // User clicked outside if the overlay, then we close the overlay.
            mainWindow->clearSideBarOverlay();
            return false;
        }
    }

    return false;
}

bool DockRegistry::onExposeEvent(Core::Window::Ptr window)
{
    if (Core::FloatingWindow *fw = floatingWindowForHandle(window)) {
        // This floating window was exposed
        m_floatingWindows.removeOne(fw);
        m_floatingWindows.append(fw);
    }

    return false;
}

void DockRegistry::addSideBarGrouping(const DockWidget::List &dws)
{
    m_sideBarGroupings->addGrouping(dws);
}

void DockRegistry::removeSideBarGrouping(const DockWidget::List &dws)
{
    m_sideBarGroupings->removeGrouping(dws);
}

DockWidget::List DockRegistry::sideBarGroupingFor(DockWidget *dw) const
{
    return m_sideBarGroupings->groupingFor(dw);
}

void SideBarGroupings::addGrouping(const DockWidget::List &dws)
{
    if (dws.size() < 2) {
        // Simplification: A single dock widget is not considered to be grouped.
        return;
    }

    m_groupings.push_back(dws);
}

void SideBarGroupings::removeGrouping(const DockWidget::List &dws)
{
    m_groupings.removeAll(dws);
}

DockWidget::List SideBarGroupings::groupingFor(DockWidget *dw) const
{
    return const_cast<SideBarGroupings *>(this)->groupingByRef(dw);
}

void SideBarGroupings::removeFromGroupings(DockWidget *dw)
{
    while (true) {
        auto &grouping = groupingByRef(dw);
        if (grouping.isEmpty())
            return;
        grouping.removeAll(dw);
    }
}

DockWidget::List &SideBarGroupings::groupingByRef(DockWidget *dw)
{
    static DockWidget::List empty;

    for (auto &grouping : m_groupings) {
        if (grouping.contains(dw))
            return grouping;
    }

    return empty;
}

CloseReasonSetter::CloseReasonSetter(CloseReason reason)
{
    DockRegistry::self()->setCurrentCloseReason(reason);
}

CloseReasonSetter::~CloseReasonSetter()
{
    DockRegistry::self()->setCurrentCloseReason(CloseReason::Unspecified);
}
