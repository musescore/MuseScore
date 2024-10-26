/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "DragController_p.h"
#include "Logging_p.h"
#include "Utils_p.h"
#include "WidgetResizeHandler_p.h"
#include "Config.h"
#include "WindowZOrder_x11_p.h"

#include "core/DockRegistry.h"
#include "core/Window_p.h"
#include "core/MDILayout.h"
#include "core/DropArea.h"
#include "core/TitleBar.h"
#include "core/Platform.h"
#include "core/Group.h"
#include "core/FloatingWindow.h"
#include "core/DockWidget_p.h"
#include "core/ScopedValueRollback_p.h"

#ifdef KDDW_FRONTEND_QT
#include "../qtcommon/DragControllerWayland_p.h"
#ifdef KDDW_FRONTEND_QTWIDGETS
#include "kddockwidgets/qtcommon/Platform.h"
#include <QWidget>
#include <QApplication>
#endif
#endif

#include <algorithm>
#include <cstdlib>

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

using namespace KDDockWidgets;
using namespace KDDockWidgets::Core;

namespace KDDockWidgets::Core {
///@brief Custom mouse grabber, for platforms that don't support grabbing the mouse
class FallbackMouseGrabber : public Core::Object, /// clazy:exclude=missing-qobject-macro
                             public EventFilterInterface /// clazy:exclude=missing-qobject-macro
{
public:
    explicit FallbackMouseGrabber(Core::Object *parent)
        : Core::Object(parent)
    {
    }

    ~FallbackMouseGrabber() override;

    void grabMouse(View *target)
    {
        m_target = target;
        m_guard = target;
        Platform::instance()->installGlobalEventFilter(this);
    }

    void releaseMouse()
    {
        // Ungrab harder if QtQuick.
        // QtQuick has the habit of grabbing the MouseArea internally, then doesn't ungrab it since
        // we're consuming the events. So explicitly ungrab if any QQuickWindow::mouseGrabberItem()
        // is still set. Done via platform now, so it's generic. Should be a no-op for QtWidgets.
        Platform::instance()->ungrabMouse();

        m_target = nullptr;
        m_guard.clear();
        Platform::instance()->removeGlobalEventFilter(this);
    }

    bool onMouseEvent(View *, MouseEvent *me) override
    {
        if (m_reentrancyGuard || !m_guard)
            return false;

        m_reentrancyGuard = true;
        Platform::instance()->sendEvent(m_target, me);
        m_reentrancyGuard = false;
        return true;
    }

    bool m_reentrancyGuard = false;
    View *m_target = nullptr;
    ViewGuard m_guard = nullptr;
};

FallbackMouseGrabber::~FallbackMouseGrabber()
{
}

}

State::State(MinimalStateMachine *parent)
    : Core::Object(parent)
    , m_machine(parent)
{
}

State::~State() = default;

bool State::isCurrentState() const
{
    return m_machine->currentState() == this;
}

MinimalStateMachine::MinimalStateMachine(Core::Object *parent)
    : Core::Object(parent)
{
}

template<typename Signal>
void State::addTransition(Signal &signal, State *dest)
{
    signal.connect([this, dest] {
        if (isCurrentState()) {
            m_machine->setCurrentState(dest);
        }
    });
}

State *MinimalStateMachine::currentState() const
{
    return m_currentState;
}

void MinimalStateMachine::setCurrentState(State *state)
{
    if (state != m_currentState) {
        if (m_currentState)
            m_currentState->onExit();

        m_currentState = state;

        if (state)
            state->onEntry();

        currentStateChanged.emit();
    }
}

StateBase::StateBase(DragController *parent)
    : State(parent)
    , q(parent)
{
}

bool StateBase::isActiveState() const
{
    return q->activeState() == this;
}

StateBase::~StateBase() = default;

StateNone::StateNone(DragController *parent)
    : StateBase(parent)
{
}

void StateNone::onEntry()
{
    KDDW_DEBUG("StateNone entered");
    q->m_pressPos = Point();
    q->m_offset = Point();
    q->m_draggable = nullptr;
    q->m_draggableGuard.clear();
    q->m_windowBeingDragged.reset();

    WidgetResizeHandler::s_disableAllHandlers = false; // Re-enable resize handlers

    q->m_nonClientDrag = false;
    q->m_inProgrammaticDrag = false;

    if (q->m_currentDropArea) {
        q->m_currentDropArea->removeHover();
        q->m_currentDropArea = nullptr;
    }

    /// Note that although this is unneedesly emitted at startup, there's nobody connected
    /// to it, since we're in DragController ctor, so it's fine.
    q->isDraggingChanged.emit();
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
bool StateNone::handleMouseButtonPress(Draggable *draggable, Point globalPos, Point pos)
{
    KDDW_DEBUG("StateNone::handleMouseButtonPress: draggable={} ; globalPos={}", ( void * )draggable,
               globalPos);

    if (!draggable) {
        KDDW_ERROR("StateNone::handleMouseButtonPress: null draggable");
        return false;
    }

    if (!q->m_inProgrammaticDrag && !draggable->isPositionDraggable(pos))
        return false;

    q->m_draggable = draggable;
    q->m_draggableGuard = draggable->asView();
    q->m_pressPos = globalPos;
    q->m_offset = draggable->mapToWindow(pos);
    q->mousePressed.emit();

    return false;
}

StateNone::~StateNone() = default;


StatePreDrag::StatePreDrag(DragController *parent)
    : StateBase(parent)
{
}

StatePreDrag::~StatePreDrag() = default;

void StatePreDrag::onEntry()
{
    KDDW_DEBUG("StatePreDrag entered {}", q->m_draggableGuard.isNull());
    WidgetResizeHandler::s_disableAllHandlers = true; // Disable the resize handler during dragging
}

bool StatePreDrag::handleMouseMove(Point globalPos)
{
    if (!q->m_draggableGuard) {
        KDDW_ERROR("Draggable was destroyed, canceling the drag");
        q->dragCanceled.emit();
        return false;
    }

    if (!q->m_draggable->dragCanStart(q->m_pressPos, globalPos))
        return false;

    if (auto func = Config::self().dragAboutToStartFunc()) {
        if (!func(q->m_draggable))
            return false;
    }

    if (q->m_draggable->isMDI())
        q->manhattanLengthMoveMDI.emit();
    else
        q->manhattanLengthMove.emit();

    return true;
}

bool StatePreDrag::handleMouseButtonRelease(Point)
{
    q->dragCanceled.emit();
    return false;
}

bool StatePreDrag::handleMouseDoubleClick()
{
    // This is only needed for QtQuick.
    // With QtQuick, when double clicking, we get: Press, Release, Press, Double-click. and never
    // receive the last Release event.
    q->dragCanceled.emit();
    return false;
}

StateDragging::StateDragging(DragController *parent)
    : StateBase(parent)
{
#if defined(KDDW_FRONTEND_QT_WINDOWS) && !defined(DOCKS_DEVELOPER_MODE)
    m_maybeCancelDrag.setInterval(100);
    QObject::connect(&m_maybeCancelDrag, &QTimer::timeout, this, [this] {
        // Workaround bug #166 , where Qt doesn't agree with Window's mouse button state.
        // Looking in the Qt bug tracker there's many hits, so do a quick workaround here:

        const bool mouseButtonIsReallyDown = (GetKeyState(VK_LBUTTON) & 0x8000);
        if (!mouseButtonIsReallyDown && Platform::instance()->isLeftMouseButtonPressed()) {
            KDDW_DEBUG("Canceling drag, Qt thinks mouse button is pressed"
                       "but Windows knows it's not");
            handleMouseButtonRelease(Platform::instance()->cursorPos());
            q->dragCanceled.emit();
        }
    });
#endif
}

StateDragging::~StateDragging() = default;

void StateDragging::onEntry()
{
#if defined(KDDW_FRONTEND_QT_WINDOWS) && !defined(DOCKS_DEVELOPER_MODE)
    m_maybeCancelDrag.start();
#endif

    if (!q->m_draggableGuard) {
        KDDW_ERROR("Draggable was destroyed, canceling the drag");
        q->dragCanceled.emit();
        return;
    }

    if (DockWidget *dw = q->m_draggable->singleDockWidget()) {
        // When we start to drag a floating window which has a single dock widget, we save the
        // position
        if (dw->isFloating())
            dw->d->saveLastFloatingGeometry();
    }

    const bool needsUndocking = !q->m_draggable->isWindow();
    q->m_windowBeingDragged = q->m_draggable->makeWindow();
    if (q->m_windowBeingDragged) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0) && defined(KDDW_FRONTEND_QT_WINDOWS)
        if (!q->m_nonClientDrag && KDDockWidgets::usesNativeDraggingAndResizing()) {
            // Started as a client move, as the dock widget was docked,
            // but now that we're dragging it as a floating window, switch to native drag, so we can
            // still get aero-snap
            FloatingWindow *fw = q->m_windowBeingDragged->floatingWindow();
            q->m_nonClientDrag = true;
            q->m_windowBeingDragged.reset();
            q->m_windowBeingDragged = fw->makeWindow();

            Window::Ptr window = fw->view()->window();

            if (needsUndocking) {
                // Position the window before the drag start, otherwise if you move mouse too fast
                // there will be an offset Only required when we've undocked/detached a window.
                const Point cursorPos = Platform::instance()->cursorPos();
                window->setPosition(cursorPos - q->m_offset);
            }

            // Start the native move
            window->startSystemMove();
        }
#else
        KDDW_UNUSED(needsUndocking);
#endif

        KDDW_DEBUG("StateDragging entered. m_draggable={}; m_windowBeingDragged={}", ( void * )q->m_draggable, ( void * )q->m_windowBeingDragged->floatingWindow());

        auto fw = q->m_windowBeingDragged->floatingWindow();
#ifdef Q_OS_LINUX
        if (fw->view()->isMaximized()) {
            // When dragging a maximized window on linux we need to restore its normal size
            // On Windows this works already. On macOS I don't see this feature at all

            const Rect normalGeometry = fw->view()->normalGeometry();

            // distance to the left edge of the window:
            const int leftOffset = q->m_offset.x();

            // distance to the right edge of the window:
            const int rightOffset = fw->width() - q->m_offset.x();

            const bool leftEdgeIsNearest = leftOffset <= rightOffset;

            fw->view()->showNormal();

            if (!normalGeometry.contains(q->m_pressPos)) {
                if ((leftEdgeIsNearest && leftOffset > normalGeometry.width())
                    || (!leftEdgeIsNearest && rightOffset > normalGeometry.width())) {
                    // Case #1: The window isn't under the cursor anymore
                    // Let's just put its middle under the cursor
                    q->m_offset.setX(normalGeometry.width() / 2);
                } else if (!leftEdgeIsNearest) {
                    // Case #2: The new geometry is still under the cursor, but instead of moving
                    // its right edge left we'll move the left edge right, since initially the press
                    // position was closer to the right edge
                    q->m_offset.setX(normalGeometry.width() - rightOffset);
                }
            }
        } else
#endif

            if (!fw->geometry().contains(q->m_pressPos)) {
            // The window shrunk when the drag started, this can happen if it has max-size
            // constraints we make the floating window smaller. Has the downside that it might not
            // be under the mouse cursor anymore, so make the change
            if (fw->width() < q->m_offset.x()) { // make sure it shrunk
                q->m_offset.setX(fw->width() / 2);
            }
        }
    } else {
        // Shouldn't happen
        KDDW_ERROR("No window being dragged for {} {}", ( void * )q->m_draggable, ( void * )q->m_draggable->asController());
        q->dragCanceled.emit();
    }

    q->isDraggingChanged.emit();
}

void StateDragging::onExit()
{
#if defined(KDDW_FRONTEND_QT_WINDOWS) && !defined(DOCKS_DEVELOPER_MODE)
    m_maybeCancelDrag.stop();
#endif

    if (auto callback = Config::self().dragEndedFunc()) {
        // this user is interested in knowing the drag ended
        callback();
    }
}

bool StateDragging::handleMouseButtonRelease(Point globalPos)
{
    KDDW_DEBUG("StateDragging: handleMouseButtonRelease");

    FloatingWindow *floatingWindow = q->m_windowBeingDragged->floatingWindow();
    if (!floatingWindow) {
        // It was deleted externally
        KDDW_DEBUG("StateDragging: Bailling out, deleted externally");
        q->dragCanceled.emit();
        return true;
    }

    if (floatingWindow->anyNonDockable()) {
        KDDW_DEBUG("StateDragging: Ignoring floating window with non dockable widgets");
        q->dragCanceled.emit();
        return true;
    }

    if (q->m_currentDropArea) {
        if (q->m_currentDropArea->drop(q->m_windowBeingDragged.get(), globalPos)) {
            q->dropped.emit();
        } else {
            KDDW_DEBUG("StateDragging: Bailling out, drop not accepted");
            q->dragCanceled.emit();
        }
    } else {
        KDDW_DEBUG("StateDragging: Bailling out, not over a drop area");
        q->dragCanceled.emit();
    }
    return true;
}

bool StateDragging::handleMouseMove(Point globalPos)
{
    FloatingWindow *fw = q->m_windowBeingDragged->floatingWindow();
    if (!fw) {
        KDDW_DEBUG("Canceling drag, window was deleted");
        q->dragCanceled.emit();
        return true;
    }

    if (fw->beingDeleted()) {
        // Ignore, we're in the middle of recurrency. We're inside
        // StateDragging::handleMouseButtonRelease too
        return true;
    }

#ifdef Q_OS_LINUX
    if (fw->lastWindowManagerState() == WindowState::Maximized) {
        // The window was maximized, we dragged it, which triggers a show normal.
        // But we can only start moving the window *after* the (async) window manager acknowledges.
        // See QTBUG-102430.
        // Since #286 was only implemented and needed on Linux, then this counter-part is also
        // ifdefed for Linux, Probably the ifdef could be removed, but don't want to be testing N
        // platforms, who's undocumented behaviour can change between releases, so narrow the scope
        // and workaround for linux only.
        return true;
    }
#endif

    if (!q->m_nonClientDrag)
        fw->view()->window()->setFramePosition(globalPos - q->m_offset);

    if (fw->anyNonDockable()) {
        KDDW_DEBUG("StateDragging: Ignoring non dockable floating window");
        return true;
    }

    DropArea *dropArea = q->dropAreaUnderCursor();
    if (q->m_currentDropArea && dropArea != q->m_currentDropArea)
        q->m_currentDropArea->removeHover();

    if (dropArea) {
        if (FloatingWindow *targetFw = dropArea->floatingWindow()) {
            if (targetFw->anyNonDockable()) {
                KDDW_DEBUG("StateDragging: Ignoring non dockable target floating window");
                return false;
            }
        }

        dropArea->hover(q->m_windowBeingDragged.get(), globalPos);
    }

    q->m_currentDropArea = dropArea;

    return true;
}

bool StateDragging::handleMouseDoubleClick()
{
    // See comment in StatePreDrag::handleMouseDoubleClick().
    // Very unlikely that we're in this state though, due to manhattan length
    q->dragCanceled.emit();
    return false;
}

StateInternalMDIDragging::StateInternalMDIDragging(DragController *parent)
    : StateBase(parent)
{
}

StateInternalMDIDragging::~StateInternalMDIDragging()
{
}

void StateInternalMDIDragging::onEntry()
{
    KDDW_DEBUG("StateInternalMDIDragging entered. draggable={}", ( void * )q->m_draggable);

    if (!q->m_draggableGuard) {
        KDDW_ERROR("Draggable was destroyed, canceling the drag");
        q->dragCanceled.emit();
        return;
    }

    // Raise the dock widget being dragged
    if (auto tb = q->m_draggable->asView()->asTitleBarController()) {
        if (Group *f = tb->group())
            f->view()->raise();
    }

    q->isDraggingChanged.emit();
}

bool StateInternalMDIDragging::handleMouseButtonRelease(Point)
{
    q->dragCanceled.emit();
    return false;
}

bool StateInternalMDIDragging::handleMouseMove(Point globalPos)
{
    if (!q->m_draggableGuard) {
        KDDW_ERROR("Draggable was destroyed, canceling the drag");
        q->dragCanceled.emit();
        return false;
    }

    // for MDI we only support dragging via the title bar, other cases don't make sense conceptually
    auto tb = q->m_draggable->asView()->asTitleBarController();
    if (!tb) {
        KDDW_ERROR("expected a title bar, not {}", ( void * )q->m_draggable);
        q->dragCanceled.emit();
        return false;
    }

    Group *group = tb->group();
    if (!group) {
        // Doesn't happen.
        KDDW_ERROR("null group.");
        q->dragCanceled.emit();
        return false;
    }

    const Size parentSize = group->view()->d->parentSize();
    const Point oldPos = group->mapToGlobal(Point(0, 0));
    const Point delta = globalPos - oldPos;
    const Point newLocalPos = group->pos() + delta - q->m_offset;

    // Let's not allow the MDI window to go outside of its parent

    Point newLocalPosBounded = { std::max(0, newLocalPos.x()), std::max(0, newLocalPos.y()) };
    newLocalPosBounded.setX(std::min(newLocalPosBounded.x(), parentSize.width() - group->width()));
    newLocalPosBounded.setY(std::min(newLocalPosBounded.y(), parentSize.height() - group->height()));

    auto layout = group->mdiLayout();
    assert(layout);
    layout->moveDockWidget(group, newLocalPosBounded);

    // Check if we need to pop out the MDI window (make it float)
    // If we drag the window against an edge, and move behind the edge some threshold, we float it
    const int threshold = Config::self().mdiPopupThreshold();
    if (threshold != -1) {
        const Point overflow = newLocalPosBounded - newLocalPos;
        if (std::abs(overflow.x()) > threshold || std::abs(overflow.y()) > threshold)
            q->mdiPopOut.emit();
    }

    return false;
}

bool StateInternalMDIDragging::handleMouseDoubleClick()
{
    q->dragCanceled.emit();
    return false;
}

namespace {

StateDragging *createDraggingState(DragController *parent)
{
#ifdef KDDW_FRONTEND_QT
    return isWayland() ? new StateDraggingWayland(parent) : new StateDragging(parent);
#else
    return new StateDragging(parent);
#endif
}

}

DragController::DragController(Core::Object *parent)
    : MinimalStateMachine(parent)
    , m_stateNone(new StateNone(this))
    , m_statePreDrag(new StatePreDrag(this))
    , m_stateDragging(createDraggingState(this))
    , m_stateDraggingMDI(new StateInternalMDIDragging(this))
{
    KDDW_TRACE("DragController CTOR");

    m_stateNone->addTransition(mousePressed, m_statePreDrag);
    m_statePreDrag->addTransition(dragCanceled, m_stateNone);
    m_statePreDrag->addTransition(manhattanLengthMove, m_stateDragging);
    m_statePreDrag->addTransition(manhattanLengthMoveMDI, m_stateDraggingMDI);
    m_stateDragging->addTransition(dragCanceled, m_stateNone);
    m_stateDragging->addTransition(dropped, m_stateNone);

    m_stateDraggingMDI->addTransition(dragCanceled, m_stateNone);
    m_stateDraggingMDI->addTransition(mdiPopOut, m_stateDragging);

    if (Platform::instance()->usesFallbackMouseGrabber())
        enableFallbackMouseGrabber();

    setCurrentState(m_stateNone);
}

DragController *DragController::instance()
{
    static DragController dragController;
    return &dragController;
}

void DragController::registerDraggable(Draggable *drg)
{
    m_draggables.push_back(drg);
    drg->asView()->installViewEventFilter(this);
}

void DragController::unregisterDraggable(Draggable *drg)
{
    m_draggables.removeOne(drg);
    drg->asView()->removeViewEventFilter(this);
}

bool DragController::isDragging() const
{
    return m_windowBeingDragged != nullptr || activeState() == m_stateDraggingMDI;
}

bool DragController::isInNonClientDrag() const
{
    return isDragging() && m_nonClientDrag;
}

bool DragController::isInClientDrag() const
{
    return isDragging() && !m_nonClientDrag;
}

bool DragController::isInProgrammaticDrag() const
{
    return m_inProgrammaticDrag;
}

bool DragController::isIdle() const
{
    return activeState() == m_stateNone;
}

void DragController::grabMouseFor(View *target)
{
    if (isWayland())
        return; // No grabbing supported on wayland

    if (m_fallbackMouseGrabber) {
        m_fallbackMouseGrabber->grabMouse(target);
    } else {
        target->grabMouse();
    }
}

void DragController::releaseMouse(View *target)
{
    if (isWayland())
        return; // No grabbing supported on wayland

    if (m_fallbackMouseGrabber) {
        m_fallbackMouseGrabber->releaseMouse();
    } else {
        target->releaseMouse();
    }
}

FloatingWindow *DragController::floatingWindowBeingDragged() const
{
    return m_windowBeingDragged ? m_windowBeingDragged->floatingWindow() : nullptr;
}

void DragController::enableFallbackMouseGrabber()
{
    if (!m_fallbackMouseGrabber)
        m_fallbackMouseGrabber = new FallbackMouseGrabber(this);
}

WindowBeingDragged *DragController::windowBeingDragged() const
{
    return m_windowBeingDragged.get();
}

bool DragController::onDnDEvent(View *view, Event *e)
{
    if (!isWayland())
        return false;

    // Wayland is very different. It uses QDrag for the dragging of a window.
    if (view) {
        KDDW_DEBUG("DragController::onDnDEvent: ev={}, dropArea=", int(e->type()), ( void * )view->asDropAreaController());
        if (auto dropArea = view->asDropAreaController()) {
            switch (int(e->type())) {
            case Event::DragEnter:
                if (activeState()->handleDragEnter(static_cast<DragMoveEvent *>(e), dropArea))
                    return true;
                break;
            case Event::DragLeave:
                if (activeState()->handleDragLeave(dropArea))
                    return true;
                break;
            case Event::DragMove:
                if (activeState()->handleDragMove(static_cast<DragMoveEvent *>(e), dropArea))
                    return true;
                break;
            case Event::Drop:
                if (activeState()->handleDrop(static_cast<DropEvent *>(e), dropArea))
                    return true;
                break;
            }
        }
    } else if (e->type() == Event::DragEnter && isDragging()) {
        // We're dragging a window. Be sure user code doesn't accept DragEnter events.
        KDDW_DEBUG("DragController::onDnDEvent: Eating DragEnter.");
        return true;
    } else {
        KDDW_DEBUG("DragController::onDnDEvent: No view. ev={}", int(e->type()));
    }

    return false;
}

bool DragController::onMoveEvent(View *)
{
    if (m_nonClientDrag) {
        // On Windows, non-client mouse moves are only sent at the end, so we must fake it:
        KDDW_TRACE("DragController::onMoveEvent");
        activeState()
            ->handleMouseMove(Platform::instance()->cursorPos());
    }

    return false;
}

bool DragController::onMouseEvent(View *w, MouseEvent *me)
{
    if (!w)
        return false;

    KDDW_TRACE("DragController::onMouseEvent e={} ; nonClientDrag={}", int(me->type()), m_nonClientDrag);

    switch (me->type()) {
    case Event::NonClientAreaMouseButtonPress: {
        if (auto fw = w->asFloatingWindowController()) {
            if (KDDockWidgets::usesNativeTitleBar()
                || fw->isInDragArea(Qt5Qt6Compat::eventGlobalPos(me))) {
                m_nonClientDrag = true;
                return activeState()->handleMouseButtonPress(
                    draggableForView(w), Qt5Qt6Compat::eventGlobalPos(me), me->pos());
            }
        }
        return false;
    }
    case Event::MouseButtonPress:
        // We don't care about the secondary button
        if (me->buttons() & Qt::RightButton)
            break;

        // For top-level windows that support native dragging all goes through the NonClient*
        // events. This also forbids dragging a FloatingWindow simply by pressing outside of the
        // title area, in the background
        if (KDDockWidgets::usesNativeDraggingAndResizing() && w->isRootView())
            break;

        assert(activeState());
        return activeState()->handleMouseButtonPress(
            draggableForView(w), Qt5Qt6Compat::eventGlobalPos(me), me->pos());

    case Event::MouseButtonRelease:
    case Event::NonClientAreaMouseButtonRelease: {
        ViewGuard guard(w);
        const bool inProgrammaticDrag = m_inProgrammaticDrag;
        const bool result = activeState()->handleMouseButtonRelease(Qt5Qt6Compat::eventGlobalPos(me));

        if (!guard) {
            // Always consume the event if the view was deleted during a DND. For example
            // tabbing A into B will destroy tabwidget A. Qt would then try to deliver event to A and crash.
            return true;
        }

        // In normal KDDW operation, we consume the mouse release (true is returned), however,
        // if using programmattic drag (via DockWidget::startDragging()), we do not want to consume the release event.
        // User might have clicked a button to start the drag. Button needs to be released when it's over, otherwise
        // it will look visually pressed.
        return result && !inProgrammaticDrag;
    }

    case Event::NonClientAreaMouseMove:
    case Event::MouseMove:
        return activeState()->handleMouseMove(Qt5Qt6Compat::eventGlobalPos(me));
    case Event::MouseButtonDblClick:
    case Event::NonClientAreaMouseButtonDblClick:
        return activeState()->handleMouseDoubleClick();
    default:
        break;
    }

    return false;
}

StateBase *DragController::activeState() const
{
    return static_cast<StateBase *>(currentState());
}

DropLocation DragController::currentDropLocation() const
{
    if (auto dropArea = dropAreaUnderCursor())
        return dropArea->currentDropLocation();

    return DropLocation_None;
}

bool DragController::programmaticStartDrag(Draggable *draggable, Point globalPos, Point offset)
{
    // Here we manually force state machine states instead of having a 2nd/parallel API.
    // As sharing 99.99% of the code path gives us some comfort.

    if (!draggable) {
        KDDW_WARN("DragController::programmaticStartDrag: draggable is null");
        return false;
    }

    if (isDragging()) {
        KDDW_WARN("DragController::programmaticStartDrag: Dragging already ongoing");
        return false;
    }

    m_inProgrammaticDrag = true;
    m_stateNone->handleMouseButtonPress(draggable, globalPos, offset);
    if (activeState() != m_statePreDrag) {
        m_inProgrammaticDrag = false;
        KDDW_WARN("DragController::programmaticStartDrag: Expected to be in pre-drag state");
        return false;
    }

    if (auto func = Config::self().dragAboutToStartFunc()) {
        if (!func(m_draggable))
            return false;
    }

    manhattanLengthMove.emit();
    if (activeState() != m_stateDragging && !isWayland()) { // wayland blocks on a QDrag::exec(). When it reaches here we're already done
        KDDW_WARN("DragController::programmaticStartDrag: Expected to be in drag state");
        return false;
    }

    // Also fake the 1st mouse move, so code that positions the frame gets run
    m_stateDragging->handleMouseMove(globalPos);

    return true;
}

void DragController::programmaticStopDrag()
{
    dragCanceled.emit();
}

#if defined(KDDW_FRONTEND_QT_WINDOWS)
static std::shared_ptr<View> qtTopLevelForHWND(HWND hwnd)
{
    const Window::List windows = Platform::instance()->windows();
    for (Window::Ptr window : windows) {
        if (!window->isVisible())
            continue;

        if (hwnd == ( HWND )window->handle()) {
            if (auto result = window->rootView())
                return result;
#ifdef KDDW_FRONTEND_QTWIDGETS
            if (Platform::instance()->isQtWidgets()) {
                // It's not a KDDW window, but we still return something, as the KDDW main window
                // might be embedded into another non-kddw QMainWindow
                // Case not supported for QtQuick.
                const QWidgetList widgets = qApp->topLevelWidgets();
                for (QWidget *widget : widgets) {
                    if (!widget->window()) {
                        // Don't call winId on windows that don't have it, as that will force all
                        // its children to have it, and that's not very stable. a top level might
                        // not have one because it's being destroyed, or because it's a top-level
                        // just because it has't been reparented I guess.
                        continue;
                    }
                    if (hwnd == ( HWND )widget->winId()) {
                        return QtCommon::Platform_qt::instance()->qobjectAsView(widget);
                    }
                }
            }
#endif
        }
    }

    KDDW_TRACE("Couldn't find hwnd for top-level hwnd={}", ( void * )hwnd);
    return nullptr;
}

#endif

static std::shared_ptr<View> qtTopLevelUnderCursor_impl(Point globalPos,
                                                        const Window::List &windows,
                                                        View *rootViewBeingDragged)
{
    for (auto i = windows.size() - 1; i >= 0; --i) {
        const Window::Ptr &window = windows.at(i);
        auto tl = window->rootView();

        if (!tl->isVisible() || tl->equals(rootViewBeingDragged) || tl->isMinimized())
            continue;

        if (rootViewBeingDragged && rootViewBeingDragged->window()->equals(window))
            continue;

        if (window->geometry().contains(globalPos)) {
            KDDW_TRACE("Found top-level {}", ( void * )tl.get());
            return tl;
        }
    }

    return nullptr;
}

std::shared_ptr<View> DragController::qtTopLevelUnderCursor() const
{
    Point globalPos = Platform::instance()->cursorPos();

    if (KDDockWidgets::isWindows()) { // So -platform offscreen on Windows doesn't use this
#if defined(KDDW_FRONTEND_QT_WINDOWS)
        POINT globalNativePos;
        if (!GetCursorPos(&globalNativePos))
            return nullptr;

        // There might be windows that don't belong to our app in between, so use win32 to travel by
        // z-order. Another solution is to set a parent on all top-levels. But this code is
        // orthogonal.
        HWND hwnd = HWND(m_windowBeingDragged->floatingWindow()->view()->window()->handle());
        while (hwnd) {
            hwnd = GetWindow(hwnd, GW_HWNDNEXT);
            RECT r;
            if (!GetWindowRect(hwnd, &r) || !IsWindowVisible(hwnd))
                continue;

            if (!PtInRect(&r, globalNativePos)) // Check if window is under cursor
                continue;

            if (auto tl = qtTopLevelForHWND(hwnd)) {
                const Rect windowGeometry = tl->d->windowGeometry();

                if (windowGeometry.contains(globalPos)
                    && tl->viewName() != QStringLiteral("_docks_IndicatorWindow_Overlay")) {
                    KDDW_TRACE("Found top-level {}", ( void * )tl.get());
                    return tl;
                }
            } else {
#ifdef KDDW_FRONTEND_QTWIDGETS
                if (Platform::instance()->isQtWidgets()) {
                    // Maybe it's embedded in a QWinWidget:
                    auto topLevels = qApp->topLevelWidgets();
                    for (auto topLevel : topLevels) {
                        if (QLatin1String(topLevel->metaObject()->className())
                            == QLatin1String("QWinWidget")) {
                            if (hwnd == GetParent(HWND(topLevel->window()->winId()))) {
                                if (topLevel->rect().contains(topLevel->mapFromGlobal(globalPos))
                                    && topLevel->objectName()
                                        != QStringLiteral("_docks_IndicatorWindow_Overlay")) {
                                    KDDW_TRACE("Found top-level {}", ( void * )topLevel);
                                    return QtCommon::Platform_qt::instance()->qobjectAsView(topLevel);
                                }
                            }
                        }
                    }
                }
#endif // QtWidgets A window belonging to another app is below the cursor
                KDDW_TRACE("Window from another app is under cursor {}", ( void * )hwnd);
                return nullptr;
            }
        }
#endif // KDDW_FRONTEND_QT_WINDOWS
    } else if (linksToXLib() && isXCB()) {
        bool ok = false;
        const Window::List orderedWindows = KDDockWidgets::orderedWindows(ok);
        FloatingWindow *tlwBeingDragged = m_windowBeingDragged->floatingWindow();
        if (auto tl =
                qtTopLevelUnderCursor_impl(globalPos, orderedWindows, tlwBeingDragged->view()))
            return tl;

        if (!ok) {
            KDDW_TRACE("No top-level found. Some windows weren't seen by XLib");
        }
    } else {
        // !Windows: Linux, macOS, offscreen (offscreen on Windows too), etc.

        // On Linux we don't have API to check the z-order of top-levels. So first check the
        // floating windows and check the MainWindow last, as the MainWindow will have lower z-order
        // as it's a parent (TODO: How will it work with multiple MainWindows ?) The floating window
        // list is sorted by z-order, as we catch QEvent::Expose and move it to last of the list

        View *tlwBeingDragged = m_windowBeingDragged->floatingWindow()->view();
        if (auto tl = qtTopLevelUnderCursor_impl(
                globalPos, DockRegistry::self()->floatingQWindows(), tlwBeingDragged))
            return tl;

        return qtTopLevelUnderCursor_impl(
            globalPos, DockRegistry::self()->topLevels(/*excludeFloatingDocks=*/true), tlwBeingDragged);
    }

    KDDW_TRACE("No top-level found");
    return nullptr;
}

static DropArea *deepestDropAreaInTopLevel(std::shared_ptr<View> topLevel, Point globalPos,
                                           const Vector<QString> &affinities)
{
    const auto localPos = topLevel->mapFromGlobal(globalPos);
    auto view = topLevel->childViewAt(localPos);

    while (view) {
        if (auto dt = view->asDropAreaController()) {
            if (DockRegistry::self()->affinitiesMatch(dt->affinities(), affinities))
                return dt;
        }
        view = view->parentView();
    }

    return nullptr;
}

DropArea *DragController::dropAreaUnderCursor() const
{
    if (!m_windowBeingDragged)
        return nullptr;

    std::shared_ptr<View> topLevel = qtTopLevelUnderCursor();
    if (!topLevel) {
        KDDW_DEBUG("DragController::dropAreaUnderCursor: No drop area under cursor");
        return nullptr;
    }

    const Vector<QString> affinities = m_windowBeingDragged->floatingWindow()->affinities();

    if (auto fw = topLevel->asFloatingWindowController()) {
        if (DockRegistry::self()->affinitiesMatch(fw->affinities(), affinities)) {
            KDDW_DEBUG("DragController::dropAreaUnderCursor: Found drop area in floating window");
            return fw->dropArea();
        }
    }

    if (topLevel->viewName() == QStringLiteral("_docks_IndicatorWindow")) {
        KDDW_ERROR("Indicator window should be hidden {} isVisible={}", ( void * )topLevel.get(), topLevel->isVisible());
        assert(false);
    }

    if (auto dt = deepestDropAreaInTopLevel(topLevel, Platform::instance()->cursorPos(), affinities)) {
        KDDW_DEBUG("DragController::dropAreaUnderCursor: Found drop area {} {}", ( void * )dt, ( void * )dt->view()->rootView().get());
        return dt;
    }

    KDDW_DEBUG("DragController::dropAreaUnderCursor: null2");
    return nullptr;
}

Draggable *DragController::draggableForView(View *view) const
{
    for (auto draggable : m_draggables)
        if (draggable->asView()->equals(view)) {
            return draggable;
        }

    return nullptr;
}

bool DragController::isInQDrag() const
{
    return m_inQDrag;
}
