/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KD_WIDGET_RESIZE_HANDLER_P_H
#define KD_WIDGET_RESIZE_HANDLER_P_H

#include "kddockwidgets/KDDockWidgets.h"
#include "kddockwidgets/docks_export.h"
#include "kddockwidgets/core/EventFilterInterface.h"
#include "core/Window_p.h"
#include "core/ViewGuard.h"

#if defined(Q_OS_WIN)
#include <windows.h>

#ifdef KDDW_FRONTEND_QTWIDGETS
#include <QAbstractNativeEventFilter>
#endif
#endif

namespace KDDockWidgets {

namespace Core {
class FloatingWindow;
}

class DOCKS_EXPORT WidgetResizeHandler : public Core::Object, public Core::EventFilterInterface
{
    Q_OBJECT
public:
    enum Feature {
        Feature_None = 0,
        Feature_NativeShadow = 1,
        Feature_NativeResize = 2,
        Feature_NativeDrag = 4,
        Feature_NativeMaximize = 8,
        Feature_All = Feature_NativeShadow | Feature_NativeResize | Feature_NativeDrag
            | Feature_NativeMaximize
    };
    Q_DECLARE_FLAGS(Features, Feature)

    struct NativeFeatures
    {
        NativeFeatures() = default;

        NativeFeatures(Rect r)
            : htCaptionRect(r)
        {
        }

        NativeFeatures(Feature f)
            : features(f)
        {
        }

        NativeFeatures(Features f)
            : features(f)
        {
        }

        Rect htCaptionRect; // in global coordinates
        Features features = Feature_All;
        bool hasFeatures() const
        {
            return features != Feature_None;
        }

        bool hasShadow() const
        {
            return features & Feature_NativeShadow;
        }

        bool hasMaximize() const
        {
            return features & Feature_NativeMaximize;
        }

        bool hasResize() const
        {
            return features & Feature_NativeResize;
        }

        bool hasDrag() const
        {
            return (features & Feature_NativeDrag) && !htCaptionRect.isNull();
        }
    };

    enum class EventFilterMode {
        Local = 1, ///< The event filter is set only on the widget being resized, this is the
                   ///< default for floating windows
        Global = 2 ///< The event filter is app-wide, used for embedded MDI windows, for example
    };

    enum class WindowMode {
        TopLevel = 1, ///< For resizing Floating Windows
        MDI = 2, ///< For resizing MDI "Windows"
    };

    /**
     * @brief CTOR
     * @param isTopLevelResizer If true, then this resize handler is for top-level widgets (aka
     * windows) if false, they are docked (like for example resizing docked MDI widgets, or the
     * sidebar overlay)
     * @param target The target widget that will be resized. Also acts as parent Object.
     */
    explicit WidgetResizeHandler(EventFilterMode, WindowMode, Core::View *target);
    ~WidgetResizeHandler() override;

    /**
     * @brief Sets the sides the user is allowed to resize with mouse.
     *
     * By default the user can resize all 4 sides.
     * However, when a dock widget is overlayed (popuped), only one side can be resized.
     */
    void setAllowedResizeSides(CursorPositions);

    /**
     * Sets the resize gap. By default 10.
     *
     * This is only used for non-top-level (child) widgets.
     * When resizing a child widget, it will be clipped by its parent, but we leave a little space
     * so we can resize it again.
     *
     * Meaning, if you're resizing 'bottom' of the child widget, it can never be bigger than
     * parent.geometry().bottom() - gap. The gap allows you to put your mouse there and resize
     * again.
     */
    void setResizeGap(int);

    bool isMDI() const;

    bool isResizing() const;

    /// Disable our global event filter and require a caller to explicitly ask for event filtering to start
    /// Used only by QtQuick MDI, which uses a MDIResizeHandlerHelper to detect 1st press. When there's multiple
    /// MDI windows, we only want the one being resized to filter for global mouse events
    void setEventFilterStartsManually();

    /// Default true. Sets whether WidgetResizeHandler will change mouse cursor depending on where it is.
    /// This is false for QtQuick as there we use a MouseArea to change cursor.
    void setHandlesMouseCursor(bool);

    static int widgetResizeHandlerMargin();

    static void setupWindow(Core::Window::Ptr);
#ifdef KDDW_FRONTEND_QT_WINDOWS
    static bool isInterestingNativeEvent(unsigned int);
    static bool handleWindowsNativeEvent(Core::Window::Ptr, MSG *msg, Qt5Qt6Compat::qintptr *result,
                                         const NativeFeatures &);
    static bool handleWindowsNativeEvent(Core::FloatingWindow *, const QByteArray &eventType,
                                         void *message, Qt5Qt6Compat::qintptr *result);

    /// Tells Qt to query window margins
    static void requestNCCALCSIZE(HWND);
#endif
    static bool s_disableAllHandlers;

private:
    // EventFilterInterface:
    bool onMouseEvent(Core::View *, MouseEvent *) override;
    void setTarget(Core::View *w);
    bool mouseMoveEvent(MouseEvent *);
    void updateCursor(CursorPosition);
    void setMouseCursor(Qt::CursorShape);
    void restoreMouseCursor();

    /// Returns which widget side the cursor is at for resize purposes
    /// Honours fixed size widgets. If fixed size, no position will be reported.
    CursorPosition cursorPosition(Point) const;

    /// Lower level overload of cursorPosition, which does not honour fixed size
    CursorPosition cursorPosition_(Point) const;

    Core::View *mTarget = nullptr;
    Core::ViewGuard mTargetGuard = nullptr;
    CursorPosition mCursorPos = CursorPosition_Undefined;
    Point mNewPosition;
    bool m_resizingInProgress = false;
    const bool m_usesGlobalEventFilter;
    const bool m_isTopLevelWindowResizer;
    int m_resizeGap = 10;
    CursorPositions mAllowedResizeSides = CursorPosition_All;
    bool m_overrideCursorSet = false;
    bool m_handlesMouseCursor = true;

    bool m_eventFilteringStartsManually = false;
};

#if defined(Q_OS_WIN) && defined(KDDW_FRONTEND_QTWIDGETS)

/**
 * @brief Helper to rediriect WM_NCHITTEST from child widgets to the top-level widget
 *
 * To implement aero-snap the top-level window must respond to WM_NCHITTEST, we do that
 * in FloatingWindow::nativeEvent(). But if the child widgets have a native handle, then
 * the WM_NCHITTEST will go to them. They have to respond HTTRANSPARENT so the event
 * is redirected.
 *
 * This only affects QtWidgets, since QQuickItems never have native WId.
 */
class NCHITTESTEventFilter : public QAbstractNativeEventFilter
{
public:
    explicit NCHITTESTEventFilter(Core::View *fw)
        : m_floatingWindow(fw)
        , m_guard(fw)
    {
    }
    bool nativeEventFilter(const QByteArray &eventType, void *message,
                           Qt5Qt6Compat::qintptr *result) override;

    Core::View *m_floatingWindow;
    Core::ViewGuard m_guard = nullptr;
};

#endif // Q_OS_WIN

}

#endif
