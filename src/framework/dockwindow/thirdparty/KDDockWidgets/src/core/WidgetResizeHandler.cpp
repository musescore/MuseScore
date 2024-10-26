/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "WidgetResizeHandler_p.h"
#include "DragController_p.h"
#include "Config.h"
#include "Utils_p.h"
#include "View_p.h"
#include "Logging_p.h"
#include "DockRegistry_p.h"

#include "kddockwidgets/core/DockRegistry.h"
#include "kddockwidgets/core/MDILayout.h"
#include "kddockwidgets/core/TitleBar.h"
#include "kddockwidgets/core/FloatingWindow.h"
#include "kddockwidgets/core/Platform.h"
#include "core/ScopedValueRollback_p.h"

#include <cstdlib>

#if defined(Q_OS_WIN)
#if defined(KDDW_FRONTEND_QTWIDGETS)
#include "../qtcommon/Platform.h"
#include <QWidget>
#endif

#if defined(KDDW_FRONTEND_QT)
#include "../qtcommon/Window_p.h"
#include <QGuiApplication>
#include <QtGui/private/qhighdpiscaling_p.h>
#endif

#endif

#if defined(Q_OS_WIN)

#include <windowsx.h>
#include <dwmapi.h>
#if defined(Q_CC_MSVC)
#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "User32.lib")
#endif
#endif

using namespace KDDockWidgets;
using namespace KDDockWidgets::Core;

#if defined(KDDW_FRONTEND_QTQUICK)
#include "qtcommon/View.h"
#endif

bool WidgetResizeHandler::s_disableAllHandlers = false;
WidgetResizeHandler::WidgetResizeHandler(EventFilterMode filterMode, WindowMode windowMode,
                                         View *target)
    : m_usesGlobalEventFilter(filterMode == EventFilterMode::Global)
    , m_isTopLevelWindowResizer(windowMode == WindowMode::TopLevel)
{
    setTarget(target);
}

WidgetResizeHandler::~WidgetResizeHandler()
{
    if (m_usesGlobalEventFilter) {
        Platform::instance()->removeGlobalEventFilter(this);
    } else if (mTargetGuard) {
        mTarget->removeViewEventFilter(this);
    }

    restoreMouseCursor();
}

void WidgetResizeHandler::setAllowedResizeSides(CursorPositions sides)
{
    mAllowedResizeSides = sides;
}

void WidgetResizeHandler::setResizeGap(int gap)
{
    m_resizeGap = gap;
}

bool WidgetResizeHandler::isMDI() const
{
    Core::Group *group = mTarget->asGroupController();
    return group && group->isMDI();
}

bool WidgetResizeHandler::isResizing() const
{
    return m_resizingInProgress;
}

int WidgetResizeHandler::widgetResizeHandlerMargin()
{
    return 4; // pixels
}

bool WidgetResizeHandler::onMouseEvent(View *widget, MouseEvent *e)
{
    if (s_disableAllHandlers || !widget || !mTargetGuard)
        return false;

    if (e->type() != Event::MouseButtonPress && e->type() != Event::MouseButtonRelease
        && e->type() != Event::MouseMove)
        return false;

    auto me = mouseEvent(e);
    if (!me)
        return false;

    if (m_isTopLevelWindowResizer) {
        // Case #1.0: Resizing FloatingWindow

        if (!widget->isRootView() || !widget->equals(mTarget)) {
            if (m_usesGlobalEventFilter) {
                // Case #1.1: FloatingWindows on EGLFS
                // EGLFS doesn't support storing mouse cursor shape per window, so we need to use
                // global filter do detect mouse leaving the window
                if (!m_resizingInProgress) {
                    const Point globalPos = Qt5Qt6Compat::eventGlobalPos(me);
                    updateCursor(cursorPosition(globalPos));
                }
            }

            // Case #1.2: FloatingWindows on all other platforms
            // Not needed to mess with the cursor, it gets set when moving over another window.
            return false;
        }
    } else if (isMDI()) {
        // Case #2: Resizing an embedded MDI "Window"

        // Each Group has a WidgetResizeHandler instance.
        // mTarget is the Group we want to resize.
        // but 'o' might not be mTarget, because we're using a global event filter.
        // The global event filter is required because we allow the cursor to be outside the group,
        // a few pixels so we have a nice resize margin. Here we deal with the case where our
        // mTarget, let's say "Group 1" is on top of "Group 2" but cursor is near "Group 2"'s
        // margins, and would show resize cursor. We only want to continue if the cursor is near the
        // margins of our own group (mTarget)

        auto f = widget->d->firstParentOfType(ViewType::Group);
        auto group = f ? f->view()->asGroupController() : nullptr;
        if (group && group->isMDIWrapper()) {
            // We don't care about the inner Option_MDINestable helper group
            group = group->mdiFrame();
        }

        if (group && !group->view()->equals(mTarget)) {
            auto groupParent =
                group->view()->d->aboutToBeDestroyed() ? nullptr : group->view()->parentView();
            auto targetParent = mTarget->d->aboutToBeDestroyed() ? nullptr : mTarget->parentView();
            const bool areSiblings = groupParent && groupParent->equals(targetParent);
            if (areSiblings) {
                if (cursorPosition(Qt5Qt6Compat::eventGlobalPos(e)) == CursorPosition_Undefined)
                    restoreMouseCursor();
                return false;
            }
        }
    }

    switch (e->type()) {
    case Event::MouseButtonPress: {
        if (mTarget->isMaximized())
            break;

        CursorPosition cursorPos = cursorPosition(Qt5Qt6Compat::eventGlobalPos(e));
        updateCursor(cursorPos);
        if (cursorPos == CursorPosition_Undefined)
            return false;

        const int m = widgetResizeHandlerMargin();
        const Rect widgetRect = mTarget->rect().marginsAdded(Margins(m, m, m, m));
        const Point cursorPoint = mTarget->mapFromGlobal(Qt5Qt6Compat::eventGlobalPos(e));
        if (!widgetRect.contains(cursorPoint) || e->button() != Qt::LeftButton)
            return false;

        m_resizingInProgress = true;
        if (isMDI())
            DockRegistry::self()->dptr()->groupInMDIResizeChanged.emit();
        mNewPosition = Qt5Qt6Compat::eventGlobalPos(e);
        mCursorPos = cursorPos;

        return true;
    }
    case Event::MouseButtonRelease: {
        m_resizingInProgress = false;
        if (isMDI()) {
            DockRegistry::self()->dptr()->groupInMDIResizeChanged.emit();
            // Usually in KDDW all geometry changes are done in the layout items, which propagate to
            // the widgets When resizing a MDI however, we're resizing the widget directly. So
            // update the corresponding layout item when we're finished.
            auto group = mTarget->asGroupController();
            group->mdiLayout()->setDockWidgetGeometry(group, group->geometry());
        }
        updateCursor(CursorPosition_Undefined);
        auto cursorPos = cursorPosition(Qt5Qt6Compat::eventGlobalPos(e));
        updateCursor(cursorPos);
        if (mTarget->isMaximized() || !m_resizingInProgress || e->button() != Qt::LeftButton)
            break;

        mTarget->releaseMouse();
        mTarget->releaseKeyboard();
        if (m_eventFilteringStartsManually)
            EventFilterInterface::setEnabled(false);

        return true;

        break;
    }
    case Event::MouseMove: {
        if (mTarget->isMaximized())
            break;

        if (isMDI()) {
            const Core::Group *groupBeingResized = DockRegistry::self()->groupInMDIResize();
            const bool otherGroupBeingResized =
                groupBeingResized && groupBeingResized->view() != mTarget;
            if (otherGroupBeingResized) {
                // only one at a time!
                return false;
            }
        }

        m_resizingInProgress = m_resizingInProgress && (e->buttons() & Qt::LeftButton);
        const bool consumed = mouseMoveEvent(e);
        return consumed;
    }
    default:
        break;
    }
    return false;
}

bool WidgetResizeHandler::mouseMoveEvent(MouseEvent *e)
{
    const Point globalPos = Qt5Qt6Compat::eventGlobalPos(e);
    if (!m_resizingInProgress) {
        const CursorPosition pos = cursorPosition(globalPos);
        updateCursor(pos);
        return pos != CursorPosition_Undefined;
    }

    const Rect oldGeometry = mTarget->d->globalGeometry();
    Rect newGeometry = oldGeometry;

    Rect parentGeometry;
    if (!mTarget->isRootView()) {
        auto parent = mTarget->parentView();
        parentGeometry = parent->d->globalGeometry();
    }

    {
        int deltaWidth = 0;
        int newWidth = 0;
        const int maxWidth = mTarget->maxSizeHint().width();
        const int minWidth = mTarget->minSize().width();

        switch (mCursorPos) {
        case CursorPosition_TopLeft:
        case CursorPosition_Left:
        case CursorPosition_BottomLeft: {
            parentGeometry = parentGeometry.adjusted(m_resizeGap, 0, 0, 0);
            deltaWidth = oldGeometry.left() - globalPos.x();
            newWidth = bound(minWidth, mTarget->width() + deltaWidth, maxWidth);
            deltaWidth = newWidth - mTarget->width();
            if (deltaWidth != 0) {
                newGeometry.setLeft(newGeometry.left() - deltaWidth);
            }

            break;
        }

        case CursorPosition_TopRight:
        case CursorPosition_Right:
        case CursorPosition_BottomRight: {
            parentGeometry = parentGeometry.adjusted(0, 0, -m_resizeGap, 0);
            deltaWidth = globalPos.x() - newGeometry.right();
            newWidth = bound(minWidth, mTarget->width() + deltaWidth, maxWidth);
            deltaWidth = newWidth - mTarget->width();
            if (deltaWidth != 0) {
                newGeometry.setRight(oldGeometry.right() + deltaWidth);
            }
            break;
        }
        default:
            break;
        }
    }

    {
        const int maxHeight = mTarget->maxSizeHint().height();
        const int minHeight = mTarget->minSize().height();
        int deltaHeight = 0;
        int newHeight = 0;
        switch (mCursorPos) {
        case CursorPosition_TopLeft:
        case CursorPosition_Top:
        case CursorPosition_TopRight: {
            parentGeometry = parentGeometry.adjusted(0, m_resizeGap, 0, 0);
            deltaHeight = oldGeometry.top() - globalPos.y();
            newHeight = bound(minHeight, mTarget->height() + deltaHeight, maxHeight);
            deltaHeight = newHeight - mTarget->height();
            if (deltaHeight != 0) {
                newGeometry.setTop(newGeometry.top() - deltaHeight);
            }

            break;
        }

        case CursorPosition_BottomLeft:
        case CursorPosition_Bottom:
        case CursorPosition_BottomRight: {
            parentGeometry = parentGeometry.adjusted(0, 0, 0, -m_resizeGap);
            deltaHeight = globalPos.y() - newGeometry.bottom();
            newHeight = bound(minHeight, mTarget->height() + deltaHeight, maxHeight);
            deltaHeight = newHeight - mTarget->height();
            if (deltaHeight != 0) {
                newGeometry.setBottom(oldGeometry.bottom() + deltaHeight);
            }
            break;
        }
        default:
            break;
        }
    }

    if (newGeometry == mTarget->geometry()) {
        // Nothing to do.
        return true;
    }

    if (!mTarget->isRootView()) {

        // Clip to parent's geometry.
        newGeometry = newGeometry.intersected(parentGeometry);

        // Back to local.
        newGeometry.moveTopLeft(mTarget->mapFromGlobal(newGeometry.topLeft()) + mTarget->pos());
    }

    mTarget->setGeometry(newGeometry);
    return true;
}

#ifdef KDDW_FRONTEND_QT_WINDOWS

void WidgetResizeHandler::requestNCCALCSIZE(HWND winId)
{
    SetWindowPos(winId, 0, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

/// Handler to enable Aero-snap
bool WidgetResizeHandler::handleWindowsNativeEvent(Core::FloatingWindow *fw,
                                                   const QByteArray &eventType, void *message,
                                                   Qt5Qt6Compat::qintptr *result)
{
    if (eventType != "windows_generic_MSG")
        return false;

    auto msg = static_cast<MSG *>(message);
    if (msg->message == WM_NCHITTEST) {
        if (DragController::instance()->isInClientDrag()) {
            // There's a non-native drag going on.
            *result = 0;
            return false;
        }

        const Rect htCaptionRect = fw->dragRect();
        const bool ret = handleWindowsNativeEvent(fw->view()->window(), msg, result, htCaptionRect);

        fw->setLastHitTest(*result);
        return ret;
    } else if (msg->message == WM_NCLBUTTONDBLCLK) {
        if ((Config::self().flags() & Config::Flag_DoubleClickMaximizes)) {
            return handleWindowsNativeEvent(fw->view()->window(), msg, result, {});
        } else {
            // Let the title bar handle it. It will re-dock the window.
            if (Core::TitleBar *titleBar = fw->titleBar()) {
                if (titleBar->isVisible()) { // can't be invisible afaik
                    titleBar->onDoubleClicked();
                }
            }

            return true;
        }
    }

    return handleWindowsNativeEvent(fw->view()->window(), msg, result, {});
}

bool WidgetResizeHandler::handleWindowsNativeEvent(Core::Window::Ptr w, MSG *msg,
                                                   Qt5Qt6Compat::qintptr *result,
                                                   const NativeFeatures &features)
{
    if (msg->message == WM_NCCALCSIZE && features.hasShadow()) {
        if (w->windowState() == WindowState::Minimized && w->hasBeenMinimizedDirectlyFromRestore()) {
            // Qt is buggy with custom WM_NCCALCSIZE if window is minimized.
            // Use full frame when minimized. We'll trigger WM_NCCALCSIZE when un-minimized.
            return false;
        }

        *result = 0;
        return true;
    } else if (msg->message == WM_NCHITTEST && (features.hasResize() || features.hasDrag())) {
        const int borderWidth = 8;
        const bool hasFixedWidth = w->minWidth() == w->maxWidth();
        const bool hasFixedHeight = w->minHeight() == w->maxHeight();
        const bool hasFixedWidthOrHeight = hasFixedWidth || hasFixedHeight;

        *result = 0;
        const int xPos = GET_X_LPARAM(msg->lParam);
        const int yPos = GET_Y_LPARAM(msg->lParam);
        RECT rect;
        GetWindowRect(reinterpret_cast<HWND>(w->handle()), &rect);

        if (!hasFixedWidthOrHeight && xPos >= rect.left && xPos <= rect.left + borderWidth && yPos <= rect.bottom
            && yPos >= rect.bottom - borderWidth && features.hasResize()) {
            *result = HTBOTTOMLEFT;
        } else if (!hasFixedWidthOrHeight && xPos < rect.right && xPos >= rect.right - borderWidth && yPos <= rect.bottom
                   && yPos >= rect.bottom - borderWidth && features.hasResize()) {
            *result = HTBOTTOMRIGHT;
        } else if (!hasFixedWidthOrHeight && xPos >= rect.left && xPos <= rect.left + borderWidth && yPos >= rect.top
                   && yPos <= rect.top + borderWidth && features.hasResize()) {
            *result = HTTOPLEFT;
        } else if (!hasFixedWidthOrHeight && xPos <= rect.right && xPos >= rect.right - borderWidth && yPos >= rect.top
                   && yPos < rect.top + borderWidth && features.hasResize()) {
            *result = HTTOPRIGHT;
        } else if (!hasFixedWidth && xPos >= rect.left && xPos <= rect.left + borderWidth
                   && features.hasResize()) {
            *result = HTLEFT;
        } else if (!hasFixedHeight && yPos >= rect.top && yPos <= rect.top + borderWidth
                   && features.hasResize()) {
            *result = HTTOP;
        } else if (!hasFixedHeight && yPos <= rect.bottom && yPos >= rect.bottom - borderWidth
                   && features.hasResize()) {
            *result = HTBOTTOM;
        } else if (!hasFixedWidth && xPos <= rect.right && xPos >= rect.right - borderWidth
                   && features.hasResize()) {
            *result = HTRIGHT;
        } else if (features.hasDrag()) {
            const Point globalPosQt = w->fromNativePixels(Point(xPos, yPos));
            // htCaptionRect is the rect on which we allow for Windows to do a native drag
            const Rect htCaptionRect = features.htCaptionRect;
            if (globalPosQt.y() >= htCaptionRect.top() && globalPosQt.y() <= htCaptionRect.bottom()
                && globalPosQt.x() >= htCaptionRect.left()
                && globalPosQt.x() <= htCaptionRect.right()) {
                if (!Platform::instance()->inDisallowedDragView(
                        globalPosQt)) { // Just makes sure the mouse isn't over the close button, we
                                        // don't allow drag in that case.
                    *result = HTCAPTION;
                }
            }
        }

        return *result != 0;
    } else if (msg->message == WM_NCLBUTTONDBLCLK && features.hasMaximize()) {
        // By returning false we accept Windows native action, a maximize.
        // We could also call titleBar->onDoubleClicked(); here which will maximize if
        // Flag_DoubleClickMaximizes is set, but there's a bug in QWidget::showMaximized() on
        // Windows when we're covering the native title bar, the window is maximized with an offset.
        // So instead, use a native maximize which works well
        return false;
    } else if (msg->message == WM_GETMINMAXINFO) {
        // Qt doesn't work well with windows that don't have title bar but have native frames.
        // When maximized they go out of bounds and the title bar is clipped, so catch
        // WM_GETMINMAXINFO and patch the size

        // According to microsoft docs it only works for the primary screen, but extrapolates for
        // the others
        auto screen = Platform::instance()->primaryScreen();
        if (!screen || w->screen() != screen) {
            return false;
        }

        DefWindowProc(msg->hwnd, msg->message, msg->wParam, msg->lParam);

        const Rect availableGeometry = screen->availableGeometry();

        auto mmi = reinterpret_cast<MINMAXINFO *>(msg->lParam);
        const double dpr = screen->devicePixelRatio();

        mmi->ptMaxSize.y = int(availableGeometry.height() * dpr);
        mmi->ptMaxSize.x =
            int(availableGeometry.width() * dpr) - 1; // -1 otherwise it gets bogus size
        mmi->ptMaxPosition.x = availableGeometry.x();
        mmi->ptMaxPosition.y = availableGeometry.y();

        mmi->ptMinTrackSize.x = int(w->minWidth() * dpr);
        mmi->ptMinTrackSize.y = int(w->minHeight() * dpr);

        *result = 0;
        return true;
    }

    return false;
}

#endif

void WidgetResizeHandler::setTarget(View *w)
{
    if (w) {
        mTarget = w;
        mTargetGuard = w;
        mTarget->setMouseTracking(true);
        if (m_usesGlobalEventFilter) {
            Platform::instance()->installGlobalEventFilter(this);
        } else {
            mTarget->installViewEventFilter(this);
        }
    } else {
        KDDW_ERROR("Target widget is null!");
    }
}

void WidgetResizeHandler::updateCursor(CursorPosition m)
{
    if (!m_handlesMouseCursor)
        return;

    if (!mTargetGuard) {
        // Our target was destroyed while we're processing mouse events, it's fine.
        restoreMouseCursor();
        return;
    }

    if (Platform::instance()->isQtWidgets()) {
        // Need for updating cursor when we change child widget
        const auto childViews = mTarget->childViews();
        for (const auto &child : childViews) {
            if (!child->hasAttribute(Qt::WA_SetCursor)) {
                child->setCursor(Qt::ArrowCursor);
            }
        }
    }
    switch (m) {
    case CursorPosition_TopLeft:
    case CursorPosition_BottomRight:
        setMouseCursor(Qt::SizeFDiagCursor);
        break;
    case CursorPosition_BottomLeft:
    case CursorPosition_TopRight:
        setMouseCursor(Qt::SizeBDiagCursor);
        break;
    case CursorPosition_Top:
    case CursorPosition_Bottom:
        setMouseCursor(Qt::SizeVerCursor);
        break;
    case CursorPosition_Left:
    case CursorPosition_Right:
        setMouseCursor(Qt::SizeHorCursor);
        break;
    case CursorPosition_Undefined:
        restoreMouseCursor();
        break;
    case CursorPosition_All:
    case CursorPosition_Horizontal:
    case CursorPosition_Vertical:
        // Doesn't happen
        break;
    }
}

void WidgetResizeHandler::setMouseCursor(Qt::CursorShape cursor)
{
    if (m_usesGlobalEventFilter) {
        Platform::instance()->setMouseCursor(cursor, /*discardLast=*/m_overrideCursorSet);
        m_overrideCursorSet = true;
    } else if (mTargetGuard) {
        mTarget->setCursor(cursor);
    }
}

void WidgetResizeHandler::restoreMouseCursor()
{
    if (!m_handlesMouseCursor)
        return;

    if (m_usesGlobalEventFilter) {
        if (m_overrideCursorSet) {
            Platform::instance()->restoreMouseCursor();
            m_overrideCursorSet = false;
        }
    } else if (mTargetGuard) {
        mTarget->setCursor(Qt::ArrowCursor);
    }
}

CursorPosition WidgetResizeHandler::cursorPosition_(Point globalPos) const
{
#ifdef KDDW_FRONTEND_QTQUICK
    if (Platform::instance()->isQtQuick() && isMDI()) {
        // Special case for QtQuick. The MouseAreas are driving it and know better what's the
        // cursor position
        auto qtview = static_cast<KDDockWidgets::QtCommon::View_qt *>(mTarget);
        const QVariant v = qtview->viewProperty("cursorPosition");
        if (v.isValid()) {
            auto pos = CursorPosition(v.toInt());
            return pos;
        }
    }
#endif

    Point pos = mTarget->mapFromGlobal(globalPos);

    const int x = pos.x();
    const int y = pos.y();
    const int margin = widgetResizeHandlerMargin();

    QFlags<CursorPosition>::Int result = CursorPosition_Undefined;
    if (y >= -margin && y <= mTarget->height() + margin) {
        if (std::abs(x) <= margin)
            result |= CursorPosition_Left;
        else if (std::abs(x - (mTarget->width() - margin)) <= margin)
            result |= CursorPosition_Right;
    }

    if (x >= -margin && x <= mTarget->width() + margin) {
        if (std::abs(y) <= margin)
            result |= CursorPosition_Top;
        else if (std::abs(y - (mTarget->height() - margin)) <= margin)
            result |= CursorPosition_Bottom;
    }

    // Filter out sides we don't allow
    result = result & mAllowedResizeSides;

    return static_cast<CursorPosition>(result);
}

CursorPosition WidgetResizeHandler::cursorPosition(Point globalPos) const
{
    if (!mTargetGuard)
        return CursorPosition_Undefined;

    auto candidatePos = cursorPosition_(globalPos);

    if (isMDI()) {
        int result = int(candidatePos);

        if (auto group = mTarget->asGroupController()) {
            // Honour fixed size. Don't show mouse shape for resizing.
            if (group->isFixedHeight())
                result &= ~CursorPosition_Vertical;

            if (group->isFixedWidth())
                result &= ~CursorPosition_Horizontal;
        } else {
            KDDW_ERROR("WidgetResizeHandler::cursorPosition: Expected group");
        }

        return static_cast<CursorPosition>(result);
    } else {
        // For regular docking there's other mechanisms to enforce fixed size
        return candidatePos;
    }
}

/** static */
void WidgetResizeHandler::setupWindow(Core::Window::Ptr window)
{
    // Does some minor setup on our QWindow.
    // Like adding the drop shadow on Windows and two other workarounds.
#ifdef KDDW_FRONTEND_QT_WINDOWS
    if (KDDockWidgets::usesAeroSnapWithCustomDecos()) {
        const auto wid = HWND(window->handle());
        window->onScreenChanged(nullptr, [](QObject *, Window::Ptr win) {
            // Qt honors our frame hijacking usually... but when screen changes we must give it a
            // nudge. Otherwise what Qt thinks is the client area is not what Windows knows it is.
            // SetWindowPos() will trigger an NCCALCSIZE message, which Qt will intercept and take
            // note of the margins we're using.
            requestNCCALCSIZE(HWND(win->handle()));
        });

        const bool usesTransparentFloatingWindow =
            Config::self().internalFlags() & Config::InternalFlag_UseTransparentFloatingWindow;
        if (!usesTransparentFloatingWindow) {
            // This enables the native drop shadow.
            // Doesn't work well if the floating window has transparent round corners (shows weird
            // white line).

            MARGINS margins = { 0, 0, 0, 1 }; // arbitrary, just needs to be > 0 it seems
            DwmExtendFrameIntoClientArea(wid, &margins);
        }
    }
#else
    KDDW_UNUSED(window);
#endif // Q_OS_WIN
}

#ifdef KDDW_FRONTEND_QT_WINDOWS
bool WidgetResizeHandler::isInterestingNativeEvent(unsigned int nativeEvent)
{
    switch (nativeEvent) {
    case WM_NCHITTEST:
    case WM_NCCALCSIZE:
    case WM_NCLBUTTONDBLCLK:
    case WM_GETMINMAXINFO:
        return true;
    default:
        return false;
    }
}
#endif

#if defined(Q_OS_WIN) && defined(KDDW_FRONTEND_QTWIDGETS)
bool NCHITTESTEventFilter::nativeEventFilter(const QByteArray &eventType, void *message,
                                             Qt5Qt6Compat::qintptr *result)

{
    if (eventType != "windows_generic_MSG" || !m_guard)
        return false;

    auto msg = static_cast<MSG *>(message);
    if (msg->message != WM_NCHITTEST)
        return false;
    const WId wid = WId(msg->hwnd);

    auto child = QtCommon::Platform_qt::instance()->qobjectAsView(QWidget::find(wid));

    if (!child || !m_floatingWindow->equals(child->rootView()))
        return false;
    const bool isThisWindow = m_floatingWindow->equals(child);

    if (!isThisWindow) {
        *result = HTTRANSPARENT;
        return true;
    }

    return false;
}
#endif

void WidgetResizeHandler::setEventFilterStartsManually()
{
    m_eventFilteringStartsManually = true;
    EventFilterInterface::setEnabled(false);
}

void WidgetResizeHandler::setHandlesMouseCursor(bool handles)
{
    m_handlesMouseCursor = handles;
}
