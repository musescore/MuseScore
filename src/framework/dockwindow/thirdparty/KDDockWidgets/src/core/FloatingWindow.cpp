/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "FloatingWindow.h"
#include "FloatingWindow_p.h"
#include "MainWindow.h"
#include "core/Logging_p.h"
#include "TitleBar.h"
#include "Group.h"
#include "Platform.h"
#include "KDDockWidgets.h"
#include "core/WindowBeingDragged_p.h"
#include "core/Utils_p.h"
#include "core/Controller_p.h"
#include "core/WidgetResizeHandler_p.h"
#include "DockRegistry.h"
#include "Config.h"
#include "Layout_p.h"
#include "core/ViewFactory.h"
#include "core/DelayedCall_p.h"
#include "core/DragController_p.h"
#include "core/LayoutSaver_p.h"
#include "DockWidget_p.h"
#include "DropArea.h"
#include "core/ScopedValueRollback_p.h"
#include "core/layouting/Item_p.h"
#include "View.h"
#include "core/View_p.h"

#include "kdbindings/signal.h"

#ifdef KDDW_FRONTEND_QT
#include <QTimer>
#ifdef Q_OS_WIN
#include <QGuiApplication>
#include <Windows.h>
#endif
#endif

#include <limits>

using namespace KDDockWidgets;
using namespace KDDockWidgets::Core;

static FloatingWindowFlags floatingWindowFlagsForGroup(Group *group)
{
    if (!group)
        return FloatingWindowFlag::FromGlobalConfig;

    const auto dockwidgets = group->dockWidgets();
    if (!dockwidgets.isEmpty())
        return dockwidgets.first()->floatingWindowFlags();

    return FloatingWindowFlag::FromGlobalConfig;
}

/** static */
Qt::WindowFlags FloatingWindow::s_windowFlagsOverride = {};

static Qt::WindowFlags windowFlagsToUse(FloatingWindowFlags requestedFlags)
{
    if (requestedFlags & FloatingWindowFlag::UseQtTool) {
        // User has explicitly chosen Qt::Tool for this FloatingWindow
        return Qt::Tool;
    }

    if (requestedFlags & FloatingWindowFlag::UseQtWindow) {
        // User has explicitly chosen Qt::Window for this FloatingWindow
        return Qt::Window;
    }

    if (FloatingWindow::s_windowFlagsOverride) {
        // User overridden the default for all FloatingWindows
        return FloatingWindow::s_windowFlagsOverride;
    }

    if (KDDockWidgets::usesNativeDraggingAndResizing())
        return Qt::Window;

    if (Config::self().internalFlags()
        & Config::InternalFlag_DontUseQtToolWindowsForFloatingWindows)
        return Qt::Window;

    return Qt::Tool;
}

static MainWindow *hackFindParentHarder(Core::Group *group, MainWindow *candidateParent)
{
    const FloatingWindowFlags requestedFlags =
        group ? group->requestedFloatingWindowFlags() : FloatingWindowFlag::FromGlobalConfig;
    if (requestedFlags & FloatingWindowFlag::DontUseParentForFloatingWindows) {
        // User explicitly requested no parent for this floating window
        return nullptr;
    }

    if (Config::self().internalFlags() & Config::InternalFlag_DontUseParentForFloatingWindows) {
        return nullptr;
    }

    // Using a parent helps the floating windows stay in front of the main window always.
    // We're not receiving the parent via ctor argument as the app can have multiple-main windows,
    // so use a hack here.
    // Not quite clear what to do if the app supports multiple main windows though.

    if (candidateParent)
        return candidateParent;

    const MainWindow::List windows = DockRegistry::self()->mainwindows();

    if (windows.isEmpty())
        return nullptr;

    if (windows.size() == 1)
        return windows.first();

    const Vector<QString> affinities = group ? group->affinities() : Vector<QString>();
    const MainWindow::List mainWindows =
        DockRegistry::self()->mainWindowsWithAffinity(affinities);

    if (mainWindows.isEmpty()) {
        KDDW_ERROR("No window with affinity={} found", affinities, "found");
        return nullptr;
    }

    return mainWindows.first();
}

MainWindow *actualParent(MainWindow *candidate)
{
    return (Config::self().internalFlags() & Config::InternalFlag_DontUseParentForFloatingWindows)
        ? nullptr
        : candidate;
}

FloatingWindow::FloatingWindow(Rect suggestedGeometry, MainWindow *parent,
                               FloatingWindowFlags requestedFlags)
    : Controller(ViewType::FloatingWindow,
                 Config::self().viewFactory()->createFloatingWindow(
                     this, actualParent(parent), windowFlagsToUse(requestedFlags)))
    , Draggable(view(),
                KDDockWidgets::usesNativeDraggingAndResizing()) // FloatingWindow is only draggable
                                                                // when using a native title bar.
                                                                // Otherwise the
                                                                // KDDockWidgets::TitleBar is the
                                                                // draggable
    , d(new Private(requestedFlags, this))
    , m_titleBar(new Core::TitleBar(this))
{
    view()->init();
    if (!suggestedGeometry.isNull())
        view()->setGeometry(suggestedGeometry);

#if defined(Q_OS_WIN) && defined(KDDW_FRONTEND_QTWIDGETS)
    // For QtQuick we do it a bit later, once we have the QQuickWindow
    if (Platform::instance()->isQtWidgets()) {
        view()->createPlatformWindow(); // QWidget::create

        // Handle WM_NCHITTEST
        m_nchittestFilter = new NCHITTESTEventFilter(view());
        qGuiApp->installNativeEventFilter(m_nchittestFilter);

        // Enables native drop-shadow
        WidgetResizeHandler::setupWindow(view()->window());

        // WM_NCCALCSIZE is handled in the views's nativeEvent(), not here.
    }
#endif

    DockRegistry::self()->registerFloatingWindow(this);

    if (d->m_flags & FloatingWindowFlag::KeepAboveIfNotUtilityWindow)
        view()->setFlag(Qt::WindowStaysOnTopHint, true);

    if (Platform::instance()->isQtWidgets()) {
        // QtQuick will do it a bit later, once it has a QWindow
        maybeCreateResizeHandler();
    }

    updateTitleBarVisibility();

    d->m_visibleWidgetCountConnection =
        d->m_dropArea->d_ptr()->visibleWidgetCountChanged.connect([this](int count) {
            onFrameCountChanged(count);
            d->numGroupsChanged.emit();
            onVisibleFrameCountChanged(count);
        });

    view()->d->closeRequested.connect([this](CloseEvent *ev) { onCloseEvent(ev); });

    view()->d->layoutInvalidated.connect([this] { updateSizeConstraints(); });

    d->m_layoutDestroyedConnection = d->m_dropArea->Controller::dptr()->aboutToBeDeleted.connect(&FloatingWindow::scheduleDeleteLater, this);

    d->numGroupsChanged.connect([this] {
        d->numDockWidgetsChanged.emit();
    });
}

FloatingWindow::FloatingWindow(Core::Group *group, Rect suggestedGeometry,
                               MainWindow *parent)
    : FloatingWindow({}, hackFindParentHarder(group, parent), floatingWindowFlagsForGroup(group))
{
    ScopedValueRollback guard(m_disableSetVisible, true);

    if (group->hasNestedMDIDockWidgets()) {
        // When using DockWidget::MDINestable, the docked MDI widget is wrapped by a drop area so we
        // can drop things into it. When floating it, we can delete that helper drop area, as
        // FloatingWindow already has one

        if (group->dockWidgetCount() == 0) {
            // doesn't happen
            KDDW_ERROR("Unexpected empty group");
            return;
        }

        DockWidget *dwMDIWrapper = group->dockWidgetAt(0);
        DropArea *dropAreaMDIWrapper = dwMDIWrapper->d->mdiDropAreaWrapper();

        if (dropAreaMDIWrapper->hasSingleGroup()) {
            Core::Group *innerFrame = dropAreaMDIWrapper->groups().constFirst();
            if (innerFrame->hasSingleDockWidget()) {
                // When pressing the unfloat button, the dock widgets gets docked to the previous
                // position it was at. Core::DockWidget::Private::m_lastPosition stores that
                // location, however, when having nested MDI, we have an extra Dock Widget, the
                // wrapper, and it contains the last position. So, when floating, we need to
                // transfer that and not lose it.
                DockWidget *dw = innerFrame->dockWidgetAt(0);
                dw->d->lastPosition() = dwMDIWrapper->d->lastPosition();
            }
        }

        d->m_dropArea->addMultiSplitter(dropAreaMDIWrapper, Location_OnTop);
        dwMDIWrapper->setVisible(false);
        if (!DragController::instance()->isIdle()) {
            // We're dragging a MDI window and we reached the border, detaching it, and making it
            // float. We can't delete the wrapper group just yet, as that would delete the title bar
            // which is currently being dragged. Delete it once the drag finishes
            d->m_currentStateChangedConnection = DragController::instance()->currentStateChanged.connect([this, dwMDIWrapper] {
                if (DragController::instance()->isIdle()) {
                    d->m_currentStateChangedConnection = KDBindings::ScopedConnection();
                    delete dwMDIWrapper;
                }
            });
        } else {
            dwMDIWrapper->destroyLater();
        }

    } else {
        // Adding a widget will trigger onFrameCountChanged, which triggers a setVisible(true).
        // The problem with setVisible(true) will forget about or requested geometry and place the
        // window at 0,0 So disable the setVisible(true) call while in the ctor.
        d->m_dropArea->addWidget(group->view(), KDDockWidgets::Location_OnTop, {});
    }

    if (!suggestedGeometry.isNull())
        view()->setGeometry(suggestedGeometry);
}

FloatingWindow::~FloatingWindow()
{
    m_inDtor = true;
    view()->d->setAboutToBeDestroyed();

    if (auto da = dropArea()) {
        // Avoid a bunch of QML warnings and constraints being violated at destruction.
        // Also simply avoiding unneeded work, as QML is destroying stuff 1 by 1
        da->view()->d->setAboutToBeDestroyed();
    }

    d->m_layoutDestroyedConnection = KDBindings::ScopedConnection();

#ifdef KDDW_FRONTEND_QT_WINDOWS
    delete m_nchittestFilter;
#endif

    DockRegistry::self()->unregisterFloatingWindow(this);
    delete m_titleBar;
    delete d;
}

void FloatingWindow::maybeCreateResizeHandler()
{
    if (!KDDockWidgets::usesNativeDraggingAndResizing()) {
        view()->setFlag(Qt::FramelessWindowHint, true);
        // EGLFS can't have different mouse cursors per window, needs global filter hack to unset
        // when cursor leaves
        const auto filterMode = isEGLFS() ? WidgetResizeHandler::EventFilterMode::Global
                                          : WidgetResizeHandler::EventFilterMode::Local;
        setWidgetResizeHandler(
            new WidgetResizeHandler(filterMode, WidgetResizeHandler::WindowMode::TopLevel, view()));
    }
}

DropArea *FloatingWindow::dropArea() const
{
    return d->m_dropArea;
}

std::unique_ptr<WindowBeingDragged> FloatingWindow::makeWindow()
{
    return std::make_unique<WindowBeingDragged>(this, this);
}

Core::DockWidget *FloatingWindow::singleDockWidget() const
{
    const Core::Group::List groups = this->groups();
    if (groups.size() == 1) {
        Core::Group *group = groups.first();
        if (group->hasSingleDockWidget())
            return group->dockWidgetAt(0);
    }

    return nullptr;
}

Core::DockWidget::List FloatingWindow::dockWidgets() const
{
    return d->m_dropArea->dockWidgets();
}

Core::Group::List FloatingWindow::groups() const
{
    assert(d->m_dropArea);
    return d->m_dropArea->groups();
}

Size FloatingWindow::maxSizeHint() const
{
    Size result = Core::Item::hardcodedMaximumSize;

    if (!d->m_dropArea) {
        // Still early, no layout set
        return result;
    }

    const Core::Group::List groups = this->groups();
    if (groups.size() == 1) {
        // Let's honour max-size when we have a single-group.
        // multi-group cases are more complicated and we're not sure if we want the window to
        // bounce around. single-group is the most common case, like floating a dock widget, so
        // let's do that first, it's also easy.
        Core::Group *group = groups[0];
        if (group->dockWidgetCount() == 1) { // We don't support if there's tabbing
            const Size waste =
                (view()->minSize() - group->view()->minSize()).expandedTo(Size(0, 0));
            result = group->view()->maxSizeHint() + waste;
        }
    }

    // Semantically the result is fine, but bound it so we don't get:
    // QWidget::setMaximumSize: (/KDDockWidgets::FloatingWindowWidget) The largest allowed size is
    // (16777215,16777215)
    return result.boundedTo(Core::Item::hardcodedMaximumSize);
}

void FloatingWindow::setSuggestedGeometry(Rect suggestedRect, SuggestedGeometryHints hint)
{
    const Size maxSize = maxSizeHint();
    const bool hasMaxSize = maxSize != Core::Item::hardcodedMaximumSize;
    if (hasMaxSize) {
        // Resize to new size but preserve center
        const Point originalCenter = suggestedRect.center();
        suggestedRect.setSize(maxSize.boundedTo(suggestedRect.size()));

        if ((hint & SuggestedGeometryHint_GeometryIsFromDocked)
            && (d->m_flags & FloatingWindowFlag::NativeTitleBar)) {
            const auto margins = contentMargins();
            suggestedRect.setHeight(suggestedRect.height() - m_titleBar->view()->height()
                                    + margins.top() + margins.bottom());
        }

        if (hint & SuggestedGeometryHint_PreserveCenter)
            suggestedRect.moveCenter(originalCenter);
    }

    ensureRectIsOnScreen(suggestedRect);

    view()->setGeometry(suggestedRect);
}

void FloatingWindow::scheduleDeleteLater()
{
    m_deleteScheduled = true;
    view()->d->setAboutToBeDestroyed();
    DockRegistry::self()->unregisterFloatingWindow(this);
    destroyLater();
}

Core::DropArea *FloatingWindow::multiSplitter() const
{
    return d->m_dropArea;
}

Layout *FloatingWindow::layout() const
{
    return d->m_dropArea;
}

bool FloatingWindow::isInDragArea(Point globalPoint) const
{
#ifdef KDDW_FRONTEND_QT_WINDOWS
    // A click near the border will still send a Qt::NonClientMousePressEvent. We shouldn't
    // interpret that as a drag, as it's for a native resize.
    // Keep track of how we handled the WM_NCHITTEST
    if (usesAeroSnapWithCustomDecos())
        return m_lastHitTest == HTCAPTION;
#endif

    return dragRect().contains(globalPoint);
}

bool FloatingWindow::anyNonClosable() const
{
    const auto groups = this->groups();
    for (Core::Group *group : groups) {
        if (group->anyNonClosable())
            return true;
    }
    return false;
}

bool FloatingWindow::anyNonDockable() const
{
    const auto groups = this->groups();
    for (Core::Group *group : groups) {
        if (group->anyNonDockable())
            return true;
    }
    return false;
}

bool FloatingWindow::hasSingleGroup() const
{
    return d->m_dropArea->hasSingleGroup();
}

bool FloatingWindow::hasSingleDockWidget() const
{
    const Core::Group::List groups = this->groups();
    if (groups.size() != 1)
        return false;

    Core::Group *group = groups.first();
    return group->dockWidgetCount() == 1;
}

Core::Group *FloatingWindow::singleFrame() const
{
    const Core::Group::List groups = this->groups();
    return groups.isEmpty() ? nullptr : groups.first();
}

bool FloatingWindow::beingDeleted() const
{
    if (m_deleteScheduled || m_inDtor)
        return true;

    const auto groups = this->groups();
    for (Core::Group *f : groups) {
        if (f->beingDeletedLater())
            return true;
    }

    return false;
}

void FloatingWindow::onFrameCountChanged(int count)
{
    if (count == 0) {
        scheduleDeleteLater();
    } else {
        updateTitleBarVisibility();
        if (count == 1) // if something was removed, then our single dock widget is floating, we
                        // need to check the Action
            dropArea()->updateFloatingActions();
    }
}

void FloatingWindow::onVisibleFrameCountChanged(int count)
{
    if (m_disableSetVisible)
        return;

    updateSizeConstraints();
    setVisible(count > 0);
}

WindowState FloatingWindow::windowStateOverride() const
{
    WindowState state = WindowState::None;

    if (view()->isMaximized())
        state = WindowState::Maximized;
    else if (view()->isMinimized())
        state = WindowState::Minimized;

    return state;
}

void FloatingWindow::updateTitleBarVisibility()
{
    if (m_updatingTitleBarVisibility)
        return; // Break recursion

    ScopedValueRollback guard(m_updatingTitleBarVisibility, true);
    updateTitleAndIcon();

    bool visible = true;

    const auto groups = this->groups();
    for (Core::Group *group : groups)
        group->updateTitleBarVisibility();

    if (KDDockWidgets::usesClientTitleBar()) {
        if ((d->m_flags & FloatingWindowFlag::HideTitleBarWhenTabsVisible)
            && !(d->m_flags & FloatingWindowFlag::AlwaysTitleBarWhenFloating)) {
            if (hasSingleGroup()) {
                visible = !groups.first()->hasTabsVisible();
            }
        }

        m_titleBar->updateButtons();
    } else {
        visible = false;
    }

    m_titleBar->setVisible(visible);
}

Vector<QString> FloatingWindow::affinities() const
{
    auto groups = this->groups();
    return groups.isEmpty() ? Vector<QString>() : groups.constFirst()->affinities();
}

void FloatingWindow::updateTitleAndIcon()
{
    QString title;
    Icon icon;
    if (hasSingleGroup()) {
        const Core::Group *group = groups().constFirst();
        title = group->title();
        icon = group->icon();
    } else {
        title = Platform::instance()->applicationName();
    }
    m_titleBar->setTitle(title);
    m_titleBar->setIcon(icon);

    // Even without a native title bar it's nice to set the window title/icon, so it shows
    // in the taskbar (when minimization is supported), or Alt-Tab (in supporting Window Managers)
    view()->setWindowTitle(title);
    view()->setWindowIcon(icon);
}

void FloatingWindow::onCloseEvent(CloseEvent *e)
{
    if (e->spontaneous() && anyNonClosable()) {
        // Event from the window system won't close us
        e->ignore();
        return;
    }

    d->m_dropArea->onCloseEvent(e);
}

bool FloatingWindow::deserialize(const LayoutSaver::FloatingWindow &fw)
{
    if (dropArea()->deserialize(fw.multiSplitterLayout)) {
        updateTitleBarVisibility();

        if (int(fw.windowState) & int(WindowState::Maximized)) {
            view()->showMaximized();
        } else if (int(fw.windowState) & int(WindowState::Minimized)) {
#ifdef KDDW_FRONTEND_QT_WINDOWS
            if (Platform::instance()->isQtQuick()) {
                // We'll minimized it after the 1st frameSwap(), so it appears in alt-tab and taskbar thumbnails.
                // Also fixes non-client area size, due to Qt not honouring WM_NCCALCSIZE correctly when showing minimized without a show normal before
                d->m_minimizationPending = true;
            } else {
                // Workaround not implemented for QtWidgets, needs to be tested there.
                view()->showMinimized();
            }
#else
            view()->showMinimized();
#endif
        } else {
            view()->showNormal();
        }

        d->numDockWidgetsChanged.emit();
        return true;
    }

    return false;
}

LayoutSaver::FloatingWindow FloatingWindow::serialize() const
{
    LayoutSaver::FloatingWindow fw;

    fw.geometry = geometry();
    fw.normalGeometry = view()->normalGeometry();
    fw.isVisible = isVisible();
    fw.multiSplitterLayout = dropArea()->serialize();
    fw.screenIndex = Platform::instance()->screenNumberForView(view());
    fw.screenSize = Platform::instance()->screenSizeFor(view());
    fw.affinities = affinities();
    fw.windowState = windowStateOverride();
    fw.flags = d->m_flags;

    Window::Ptr transientParentWindow = view()->d->transientWindow();
    auto transientMainWindow = DockRegistry::self()->mainWindowForHandle(transientParentWindow);
    fw.parentIndex =
        transientMainWindow ? DockRegistry::self()->mainwindows().indexOf(transientMainWindow) : -1;

    return fw;
}

Rect FloatingWindow::dragRect() const
{
    Rect rect;
    if (m_titleBar->isVisible()) {
        rect = m_titleBar->rect();
        rect.moveTopLeft(m_titleBar->view()->mapToGlobal(Point(0, 0)));
    } else if (hasSingleGroup()) {
        rect = groups().constFirst()->dragRect();
    } else {
        KDDW_ERROR("Expected a title bar");
    }

    return rect;
}

bool FloatingWindow::allDockWidgetsHave(DockWidgetOption option) const
{
    const Core::Group::List groups = this->groups();
    return std::all_of(groups.begin(), groups.end(), [option](Core::Group *group) {
        return group->allDockWidgetsHave(option);
    });
}

bool FloatingWindow::anyDockWidgetsHas(DockWidgetOption option) const
{
    const Core::Group::List groups = this->groups();
    return std::any_of(groups.begin(), groups.end(), [option](Core::Group *group) {
        return group->anyDockWidgetsHas(option);
    });
}

bool FloatingWindow::allDockWidgetsHave(LayoutSaverOption option) const
{
    const Core::Group::List groups = this->groups();
    return std::all_of(groups.begin(), groups.end(), [option](Core::Group *group) {
        return group->allDockWidgetsHave(option);
    });
}

bool FloatingWindow::anyDockWidgetsHas(LayoutSaverOption option) const
{
    const Core::Group::List groups = this->groups();
    return std::any_of(groups.begin(), groups.end(), [option](Core::Group *group) {
        return group->anyDockWidgetsHas(option);
    });
}

void FloatingWindow::addDockWidget(Core::DockWidget *dw, Location location,
                                   Core::DockWidget *relativeTo, const InitialOption &option)
{
    d->m_dropArea->addDockWidget(dw, location, relativeTo, option);
}

bool FloatingWindow::isMDI() const
{
    return false;
}

bool FloatingWindow::isWindow() const
{
    return true;
}

MainWindow *FloatingWindow::mainWindow() const
{
    return view()->parentView()->asMainWindowController();
}

Margins FloatingWindow::contentMargins() const
{
    return { 4, 4, 4, 4 };
}

WindowState FloatingWindow::lastWindowManagerState() const
{
    return m_lastWindowManagerState;
}

int FloatingWindow::userType() const
{
    if (Core::Group *f = singleFrame())
        return f->userType();
    return 0;
}

void FloatingWindow::updateSizeConstraints()
{
#ifdef KDDW_FRONTEND_QT
    // Doing a delayed call to make sure the layout has completed any ongoing operation.
    QTimer::singleShot(0, this, [this] {
        // Not simply using layout's max-size support because
        // 1) that's not portable to QtQuick
        // 2) QStackedLayout (from tab-widget) doesn't propagate size constraints up
        // Doing it manually instead.
        view()->setMaximumSize(maxSizeHint());
    });
#endif
}

void FloatingWindow::ensureRectIsOnScreen(Rect &geometry)
{
    const auto screens = Platform::instance()->screens();
    if (screens.empty())
        return;

    int nearestDistSq = std::numeric_limits<int>::max();
    int nearestIndex = -1;

    const int screenCount = screens.count();
    for (int i = 0; i < screenCount; i++) {
        const Rect scrGeom = screens[i]->geometry();

        // If the rectangle is visible at all, we need do nothing
        if (scrGeom.intersects(geometry))
            return;

        // Find the nearest screen, so we can move the geometry onto it
        const Point dist2D = geometry.center() - scrGeom.center();
        const int distSq = (dist2D.x() * dist2D.x()) + (dist2D.y() * dist2D.y());
        if (distSq < nearestDistSq) {
            nearestDistSq = distSq;
            nearestIndex = i;
        }
    }

    // Move the rectangle to the nearest vertical and/or horizontal screen edge
    auto scrGeom = screens[nearestIndex]->geometry();
    scrGeom.moveTopLeft(scrGeom.topLeft() - screens[nearestIndex]->virtualGeometry().topLeft());

    if (geometry.left() < scrGeom.left()) {
        geometry.moveLeft(scrGeom.left());
    } else if (geometry.left() > scrGeom.right()) {
        geometry.moveRight(scrGeom.right());
    }

    if (geometry.top() < scrGeom.top()) {
        geometry.moveTop(scrGeom.top());
    } else if (geometry.top() > scrGeom.bottom()) {
        geometry.moveBottom(scrGeom.bottom());
    }
}

void FloatingWindow::setLastWindowManagerState(WindowState state)
{
    m_lastWindowManagerState = state;
}

bool FloatingWindow::supportsMinimizeButton() const
{
    return d->m_flags & FloatingWindowFlag::TitleBarHasMinimizeButton;
}

bool FloatingWindow::supportsMaximizeButton() const
{
    return d->m_flags & FloatingWindowFlag::TitleBarHasMaximizeButton;
}

bool FloatingWindow::isUtilityWindow() const
{
    const bool dontUse = (d->m_flags & FloatingWindowFlag::DontUseParentForFloatingWindows) && (d->m_flags & FloatingWindowFlag::UseQtWindow);
    return !dontUse;
}

FloatingWindowFlags FloatingWindow::floatingWindowFlags() const
{
    return d->m_flags;
}

FloatingWindow::Private *FloatingWindow::dptr() const
{
    return d;
}

void FloatingWindow::focus(Qt::FocusReason reason)
{
    const auto groups = this->groups();
    if (groups.isEmpty())
        return; // doesn't really happen

    groups.constFirst()->focus(reason);
}

inline FloatingWindowFlags flagsForFloatingWindow(FloatingWindowFlags requestedFlags)
{
    if (!(requestedFlags & FloatingWindowFlag::FromGlobalConfig)) {
        // User requested specific flags for this floating window
        return requestedFlags;
    }

    // Use from KDDockWidgets::Config instead. This is app-wide and not per window.

    FloatingWindowFlags flags = {};

    if ((Config::self().flags() & Config::Flag_TitleBarHasMinimizeButton)
        == Config::Flag_TitleBarHasMinimizeButton)
        flags |= FloatingWindowFlag::TitleBarHasMinimizeButton;

    if (Config::self().flags() & Config::Flag_TitleBarHasMaximizeButton)
        flags |= FloatingWindowFlag::TitleBarHasMaximizeButton;

    if (Config::self().flags() & Config::Flag_KeepAboveIfNotUtilityWindow)
        flags |= FloatingWindowFlag::KeepAboveIfNotUtilityWindow;

    if (Config::self().flags() & Config::Flag_NativeTitleBar)
        flags |= FloatingWindowFlag::NativeTitleBar;

    if (Config::self().flags() & Config::Flag_HideTitleBarWhenTabsVisible)
        flags |= FloatingWindowFlag::HideTitleBarWhenTabsVisible;

    if (Config::self().flags() & Config::Flag_AlwaysTitleBarWhenFloating)
        flags |= FloatingWindowFlag::AlwaysTitleBarWhenFloating;

    if (Config::self().internalFlags() & Config::InternalFlag_DontUseParentForFloatingWindows)
        flags |= FloatingWindowFlag::DontUseParentForFloatingWindows;

    if (Config::self().internalFlags() & Config::InternalFlag_DontUseQtToolWindowsForFloatingWindows)
        flags |= FloatingWindowFlag::UseQtWindow;

    return flags;
}

FloatingWindow::Private::Private(FloatingWindowFlags requestedFlags, FloatingWindow *q)
    : m_flags(flagsForFloatingWindow(requestedFlags))
    , m_dropArea(new DropArea(q->view(), MainWindowOption_None))
{
}
