/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "DockWidget_c.h"


#include <iostream>

#include <cassert>


namespace Dartagnan {

typedef int (*CleanupCallback)(void *thisPtr);
static CleanupCallback s_cleanupCallback = nullptr;

template<typename T>
struct ValueWrapper
{
    T value;
};

}
namespace KDDockWidgetsBindings_wrappersNS {
namespace KDDWBindingsFlutter {
DockWidget_wrapper::DockWidget_wrapper(const QString &uniqueName, QFlags<KDDockWidgets::DockWidgetOption> options, QFlags<KDDockWidgets::LayoutSaverOption> layoutSaverOptions)
    : ::KDDockWidgets::flutter::DockWidget(uniqueName, options, layoutSaverOptions)
{
}
void DockWidget_wrapper::activateWindow()
{
    if (m_activateWindowCallback) {
        const void *thisPtr = this;
        m_activateWindowCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::flutter::DockWidget::activateWindow();
    }
}
void DockWidget_wrapper::activateWindow_nocallback()
{
    ::KDDockWidgets::flutter::DockWidget::activateWindow();
}
bool DockWidget_wrapper::close()
{
    if (m_closeCallback) {
        const void *thisPtr = this;
        return m_closeCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::flutter::DockWidget::close();
    }
}
bool DockWidget_wrapper::close_nocallback()
{
    return ::KDDockWidgets::flutter::DockWidget::close();
}
void DockWidget_wrapper::createPlatformWindow()
{
    if (m_createPlatformWindowCallback) {
        const void *thisPtr = this;
        m_createPlatformWindowCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::flutter::DockWidget::createPlatformWindow();
    }
}
void DockWidget_wrapper::createPlatformWindow_nocallback()
{
    ::KDDockWidgets::flutter::DockWidget::createPlatformWindow();
}
KDDockWidgets::Core::DockWidget *DockWidget_wrapper::dockWidget() const
{
    return ::KDDockWidgets::flutter::DockWidget::dockWidget();
}
Qt::WindowFlags DockWidget_wrapper::flags() const
{
    if (m_flagsCallback) {
        const void *thisPtr = this;
        return m_flagsCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::flutter::DockWidget::flags();
    }
}
Qt::WindowFlags DockWidget_wrapper::flags_nocallback() const
{
    return ::KDDockWidgets::flutter::DockWidget::flags();
}
KDDockWidgets::Rect DockWidget_wrapper::geometry() const
{
    if (m_geometryCallback) {
        const void *thisPtr = this;
        return *m_geometryCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::flutter::DockWidget::geometry();
    }
}
KDDockWidgets::Rect DockWidget_wrapper::geometry_nocallback() const
{
    return ::KDDockWidgets::flutter::DockWidget::geometry();
}
void DockWidget_wrapper::grabMouse()
{
    if (m_grabMouseCallback) {
        const void *thisPtr = this;
        m_grabMouseCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::flutter::DockWidget::grabMouse();
    }
}
void DockWidget_wrapper::grabMouse_nocallback()
{
    ::KDDockWidgets::flutter::DockWidget::grabMouse();
}
bool DockWidget_wrapper::hasFocus() const
{
    if (m_hasFocusCallback) {
        const void *thisPtr = this;
        return m_hasFocusCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::flutter::DockWidget::hasFocus();
    }
}
bool DockWidget_wrapper::hasFocus_nocallback() const
{
    return ::KDDockWidgets::flutter::DockWidget::hasFocus();
}
void DockWidget_wrapper::hide()
{
    if (m_hideCallback) {
        const void *thisPtr = this;
        m_hideCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::flutter::DockWidget::hide();
    }
}
void DockWidget_wrapper::hide_nocallback()
{
    ::KDDockWidgets::flutter::DockWidget::hide();
}
void DockWidget_wrapper::init()
{
    if (m_initCallback) {
        const void *thisPtr = this;
        m_initCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::flutter::DockWidget::init();
    }
}
void DockWidget_wrapper::init_nocallback()
{
    ::KDDockWidgets::flutter::DockWidget::init();
}
bool DockWidget_wrapper::isActiveWindow() const
{
    if (m_isActiveWindowCallback) {
        const void *thisPtr = this;
        return m_isActiveWindowCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::flutter::DockWidget::isActiveWindow();
    }
}
bool DockWidget_wrapper::isActiveWindow_nocallback() const
{
    return ::KDDockWidgets::flutter::DockWidget::isActiveWindow();
}
bool DockWidget_wrapper::isExplicitlyHidden() const
{
    if (m_isExplicitlyHiddenCallback) {
        const void *thisPtr = this;
        return m_isExplicitlyHiddenCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::flutter::DockWidget::isExplicitlyHidden();
    }
}
bool DockWidget_wrapper::isExplicitlyHidden_nocallback() const
{
    return ::KDDockWidgets::flutter::DockWidget::isExplicitlyHidden();
}
bool DockWidget_wrapper::isMaximized() const
{
    if (m_isMaximizedCallback) {
        const void *thisPtr = this;
        return m_isMaximizedCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::flutter::DockWidget::isMaximized();
    }
}
bool DockWidget_wrapper::isMaximized_nocallback() const
{
    return ::KDDockWidgets::flutter::DockWidget::isMaximized();
}
bool DockWidget_wrapper::isMinimized() const
{
    if (m_isMinimizedCallback) {
        const void *thisPtr = this;
        return m_isMinimizedCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::flutter::DockWidget::isMinimized();
    }
}
bool DockWidget_wrapper::isMinimized_nocallback() const
{
    return ::KDDockWidgets::flutter::DockWidget::isMinimized();
}
bool DockWidget_wrapper::isMounted() const
{
    if (m_isMountedCallback) {
        const void *thisPtr = this;
        return m_isMountedCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::flutter::DockWidget::isMounted();
    }
}
bool DockWidget_wrapper::isMounted_nocallback() const
{
    return ::KDDockWidgets::flutter::DockWidget::isMounted();
}
bool DockWidget_wrapper::isNull() const
{
    if (m_isNullCallback) {
        const void *thisPtr = this;
        return m_isNullCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::flutter::DockWidget::isNull();
    }
}
bool DockWidget_wrapper::isNull_nocallback() const
{
    return ::KDDockWidgets::flutter::DockWidget::isNull();
}
bool DockWidget_wrapper::isRootView() const
{
    if (m_isRootViewCallback) {
        const void *thisPtr = this;
        return m_isRootViewCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::flutter::DockWidget::isRootView();
    }
}
bool DockWidget_wrapper::isRootView_nocallback() const
{
    return ::KDDockWidgets::flutter::DockWidget::isRootView();
}
bool DockWidget_wrapper::isVisible() const
{
    if (m_isVisibleCallback) {
        const void *thisPtr = this;
        return m_isVisibleCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::flutter::DockWidget::isVisible();
    }
}
bool DockWidget_wrapper::isVisible_nocallback() const
{
    return ::KDDockWidgets::flutter::DockWidget::isVisible();
}
KDDockWidgets::Point DockWidget_wrapper::mapFromGlobal(KDDockWidgets::Point globalPt) const
{
    if (m_mapFromGlobalCallback) {
        const void *thisPtr = this;
        return *m_mapFromGlobalCallback(const_cast<void *>(thisPtr), &globalPt);
    } else {
        return ::KDDockWidgets::flutter::DockWidget::mapFromGlobal(globalPt);
    }
}
KDDockWidgets::Point DockWidget_wrapper::mapFromGlobal_nocallback(KDDockWidgets::Point globalPt) const
{
    return ::KDDockWidgets::flutter::DockWidget::mapFromGlobal(globalPt);
}
KDDockWidgets::Point DockWidget_wrapper::mapTo(KDDockWidgets::Core::View *parent, KDDockWidgets::Point pos) const
{
    if (m_mapToCallback) {
        const void *thisPtr = this;
        return *m_mapToCallback(const_cast<void *>(thisPtr), parent, &pos);
    } else {
        return ::KDDockWidgets::flutter::DockWidget::mapTo(parent, pos);
    }
}
KDDockWidgets::Point DockWidget_wrapper::mapTo_nocallback(KDDockWidgets::Core::View *parent, KDDockWidgets::Point pos) const
{
    return ::KDDockWidgets::flutter::DockWidget::mapTo(parent, pos);
}
KDDockWidgets::Point DockWidget_wrapper::mapToGlobal(KDDockWidgets::Point localPt) const
{
    if (m_mapToGlobalCallback) {
        const void *thisPtr = this;
        return *m_mapToGlobalCallback(const_cast<void *>(thisPtr), &localPt);
    } else {
        return ::KDDockWidgets::flutter::DockWidget::mapToGlobal(localPt);
    }
}
KDDockWidgets::Point DockWidget_wrapper::mapToGlobal_nocallback(KDDockWidgets::Point localPt) const
{
    return ::KDDockWidgets::flutter::DockWidget::mapToGlobal(localPt);
}
KDDockWidgets::Size DockWidget_wrapper::maxSizeHint() const
{
    if (m_maxSizeHintCallback) {
        const void *thisPtr = this;
        return *m_maxSizeHintCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::flutter::DockWidget::maxSizeHint();
    }
}
KDDockWidgets::Size DockWidget_wrapper::maxSizeHint_nocallback() const
{
    return ::KDDockWidgets::flutter::DockWidget::maxSizeHint();
}
KDDockWidgets::Size DockWidget_wrapper::minSize() const
{
    if (m_minSizeCallback) {
        const void *thisPtr = this;
        return *m_minSizeCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::flutter::DockWidget::minSize();
    }
}
KDDockWidgets::Size DockWidget_wrapper::minSize_nocallback() const
{
    return ::KDDockWidgets::flutter::DockWidget::minSize();
}
void DockWidget_wrapper::move(int x, int y)
{
    if (m_move_2Callback) {
        const void *thisPtr = this;
        m_move_2Callback(const_cast<void *>(thisPtr), x, y);
    } else {
        ::KDDockWidgets::flutter::DockWidget::move(x, y);
    }
}
void DockWidget_wrapper::move_nocallback(int x, int y)
{
    ::KDDockWidgets::flutter::DockWidget::move(x, y);
}
KDDockWidgets::Rect DockWidget_wrapper::normalGeometry() const
{
    if (m_normalGeometryCallback) {
        const void *thisPtr = this;
        return *m_normalGeometryCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::flutter::DockWidget::normalGeometry();
    }
}
KDDockWidgets::Rect DockWidget_wrapper::normalGeometry_nocallback() const
{
    return ::KDDockWidgets::flutter::DockWidget::normalGeometry();
}
void DockWidget_wrapper::onChildAdded(KDDockWidgets::Core::View *childView)
{
    if (m_onChildAddedCallback) {
        const void *thisPtr = this;
        m_onChildAddedCallback(const_cast<void *>(thisPtr), childView);
    } else {
        ::KDDockWidgets::flutter::DockWidget::onChildAdded(childView);
    }
}
void DockWidget_wrapper::onChildAdded_nocallback(KDDockWidgets::Core::View *childView)
{
    ::KDDockWidgets::flutter::DockWidget::onChildAdded(childView);
}
void DockWidget_wrapper::onChildRemoved(KDDockWidgets::Core::View *childView)
{
    if (m_onChildRemovedCallback) {
        const void *thisPtr = this;
        m_onChildRemovedCallback(const_cast<void *>(thisPtr), childView);
    } else {
        ::KDDockWidgets::flutter::DockWidget::onChildRemoved(childView);
    }
}
void DockWidget_wrapper::onChildRemoved_nocallback(KDDockWidgets::Core::View *childView)
{
    ::KDDockWidgets::flutter::DockWidget::onChildRemoved(childView);
}
void DockWidget_wrapper::onChildVisibilityChanged(KDDockWidgets::Core::View *childView)
{
    if (m_onChildVisibilityChangedCallback) {
        const void *thisPtr = this;
        m_onChildVisibilityChangedCallback(const_cast<void *>(thisPtr), childView);
    } else {
        ::KDDockWidgets::flutter::DockWidget::onChildVisibilityChanged(childView);
    }
}
void DockWidget_wrapper::onChildVisibilityChanged_nocallback(KDDockWidgets::Core::View *childView)
{
    ::KDDockWidgets::flutter::DockWidget::onChildVisibilityChanged(childView);
}
void DockWidget_wrapper::onGeometryChanged()
{
    if (m_onGeometryChangedCallback) {
        const void *thisPtr = this;
        m_onGeometryChangedCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::flutter::DockWidget::onGeometryChanged();
    }
}
void DockWidget_wrapper::onGeometryChanged_nocallback()
{
    ::KDDockWidgets::flutter::DockWidget::onGeometryChanged();
}
void DockWidget_wrapper::onRebuildRequested()
{
    if (m_onRebuildRequestedCallback) {
        const void *thisPtr = this;
        m_onRebuildRequestedCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::flutter::DockWidget::onRebuildRequested();
    }
}
void DockWidget_wrapper::onRebuildRequested_nocallback()
{
    ::KDDockWidgets::flutter::DockWidget::onRebuildRequested();
}
bool DockWidget_wrapper::onResize(int h, int w)
{
    if (m_onResize_2Callback) {
        const void *thisPtr = this;
        return m_onResize_2Callback(const_cast<void *>(thisPtr), h, w);
    } else {
        return ::KDDockWidgets::flutter::DockWidget::onResize(h, w);
    }
}
bool DockWidget_wrapper::onResize_nocallback(int h, int w)
{
    return ::KDDockWidgets::flutter::DockWidget::onResize(h, w);
}
void DockWidget_wrapper::raise()
{
    if (m_raiseCallback) {
        const void *thisPtr = this;
        m_raiseCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::flutter::DockWidget::raise();
    }
}
void DockWidget_wrapper::raise_nocallback()
{
    ::KDDockWidgets::flutter::DockWidget::raise();
}
void DockWidget_wrapper::raiseAndActivate()
{
    if (m_raiseAndActivateCallback) {
        const void *thisPtr = this;
        m_raiseAndActivateCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::flutter::DockWidget::raiseAndActivate();
    }
}
void DockWidget_wrapper::raiseAndActivate_nocallback()
{
    ::KDDockWidgets::flutter::DockWidget::raiseAndActivate();
}
void DockWidget_wrapper::raiseChild(KDDockWidgets::Core::View *childView)
{
    if (m_raiseChildCallback) {
        const void *thisPtr = this;
        m_raiseChildCallback(const_cast<void *>(thisPtr), childView);
    } else {
        ::KDDockWidgets::flutter::DockWidget::raiseChild(childView);
    }
}
void DockWidget_wrapper::raiseChild_nocallback(KDDockWidgets::Core::View *childView)
{
    ::KDDockWidgets::flutter::DockWidget::raiseChild(childView);
}
void DockWidget_wrapper::raiseWindow(KDDockWidgets::Core::View *rootView)
{
    if (m_raiseWindowCallback) {
        const void *thisPtr = this;
        m_raiseWindowCallback(const_cast<void *>(thisPtr), rootView);
    } else {
        ::KDDockWidgets::flutter::DockWidget::raiseWindow(rootView);
    }
}
void DockWidget_wrapper::raiseWindow_nocallback(KDDockWidgets::Core::View *rootView)
{
    ::KDDockWidgets::flutter::DockWidget::raiseWindow(rootView);
}
void DockWidget_wrapper::releaseKeyboard()
{
    if (m_releaseKeyboardCallback) {
        const void *thisPtr = this;
        m_releaseKeyboardCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::flutter::DockWidget::releaseKeyboard();
    }
}
void DockWidget_wrapper::releaseKeyboard_nocallback()
{
    ::KDDockWidgets::flutter::DockWidget::releaseKeyboard();
}
void DockWidget_wrapper::releaseMouse()
{
    if (m_releaseMouseCallback) {
        const void *thisPtr = this;
        m_releaseMouseCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::flutter::DockWidget::releaseMouse();
    }
}
void DockWidget_wrapper::releaseMouse_nocallback()
{
    ::KDDockWidgets::flutter::DockWidget::releaseMouse();
}
void DockWidget_wrapper::setCursor(Qt::CursorShape shape)
{
    if (m_setCursorCallback) {
        const void *thisPtr = this;
        m_setCursorCallback(const_cast<void *>(thisPtr), shape);
    } else {
        ::KDDockWidgets::flutter::DockWidget::setCursor(shape);
    }
}
void DockWidget_wrapper::setCursor_nocallback(Qt::CursorShape shape)
{
    ::KDDockWidgets::flutter::DockWidget::setCursor(shape);
}
void DockWidget_wrapper::setFixedHeight(int h)
{
    if (m_setFixedHeightCallback) {
        const void *thisPtr = this;
        m_setFixedHeightCallback(const_cast<void *>(thisPtr), h);
    } else {
        ::KDDockWidgets::flutter::DockWidget::setFixedHeight(h);
    }
}
void DockWidget_wrapper::setFixedHeight_nocallback(int h)
{
    ::KDDockWidgets::flutter::DockWidget::setFixedHeight(h);
}
void DockWidget_wrapper::setFixedWidth(int w)
{
    if (m_setFixedWidthCallback) {
        const void *thisPtr = this;
        m_setFixedWidthCallback(const_cast<void *>(thisPtr), w);
    } else {
        ::KDDockWidgets::flutter::DockWidget::setFixedWidth(w);
    }
}
void DockWidget_wrapper::setFixedWidth_nocallback(int w)
{
    ::KDDockWidgets::flutter::DockWidget::setFixedWidth(w);
}
void DockWidget_wrapper::setGeometry(KDDockWidgets::Rect geometry)
{
    if (m_setGeometryCallback) {
        const void *thisPtr = this;
        m_setGeometryCallback(const_cast<void *>(thisPtr), &geometry);
    } else {
        ::KDDockWidgets::flutter::DockWidget::setGeometry(geometry);
    }
}
void DockWidget_wrapper::setGeometry_nocallback(KDDockWidgets::Rect geometry)
{
    ::KDDockWidgets::flutter::DockWidget::setGeometry(geometry);
}
void DockWidget_wrapper::setHeight(int h)
{
    if (m_setHeightCallback) {
        const void *thisPtr = this;
        m_setHeightCallback(const_cast<void *>(thisPtr), h);
    } else {
        ::KDDockWidgets::flutter::DockWidget::setHeight(h);
    }
}
void DockWidget_wrapper::setHeight_nocallback(int h)
{
    ::KDDockWidgets::flutter::DockWidget::setHeight(h);
}
void DockWidget_wrapper::setMaximumSize(KDDockWidgets::Size sz)
{
    if (m_setMaximumSizeCallback) {
        const void *thisPtr = this;
        m_setMaximumSizeCallback(const_cast<void *>(thisPtr), &sz);
    } else {
        ::KDDockWidgets::flutter::DockWidget::setMaximumSize(sz);
    }
}
void DockWidget_wrapper::setMaximumSize_nocallback(KDDockWidgets::Size sz)
{
    ::KDDockWidgets::flutter::DockWidget::setMaximumSize(sz);
}
void DockWidget_wrapper::setMinimumSize(KDDockWidgets::Size sz)
{
    if (m_setMinimumSizeCallback) {
        const void *thisPtr = this;
        m_setMinimumSizeCallback(const_cast<void *>(thisPtr), &sz);
    } else {
        ::KDDockWidgets::flutter::DockWidget::setMinimumSize(sz);
    }
}
void DockWidget_wrapper::setMinimumSize_nocallback(KDDockWidgets::Size sz)
{
    ::KDDockWidgets::flutter::DockWidget::setMinimumSize(sz);
}
void DockWidget_wrapper::setMouseTracking(bool enable)
{
    if (m_setMouseTrackingCallback) {
        const void *thisPtr = this;
        m_setMouseTrackingCallback(const_cast<void *>(thisPtr), enable);
    } else {
        ::KDDockWidgets::flutter::DockWidget::setMouseTracking(enable);
    }
}
void DockWidget_wrapper::setMouseTracking_nocallback(bool enable)
{
    ::KDDockWidgets::flutter::DockWidget::setMouseTracking(enable);
}
void DockWidget_wrapper::setParent(KDDockWidgets::Core::View *parent)
{
    if (m_setParentCallback) {
        const void *thisPtr = this;
        m_setParentCallback(const_cast<void *>(thisPtr), parent);
    } else {
        ::KDDockWidgets::flutter::DockWidget::setParent(parent);
    }
}
void DockWidget_wrapper::setParent_nocallback(KDDockWidgets::Core::View *parent)
{
    ::KDDockWidgets::flutter::DockWidget::setParent(parent);
}
void DockWidget_wrapper::setSize(int w, int h)
{
    if (m_setSize_2Callback) {
        const void *thisPtr = this;
        m_setSize_2Callback(const_cast<void *>(thisPtr), w, h);
    } else {
        ::KDDockWidgets::flutter::DockWidget::setSize(w, h);
    }
}
void DockWidget_wrapper::setSize_nocallback(int w, int h)
{
    ::KDDockWidgets::flutter::DockWidget::setSize(w, h);
}
void DockWidget_wrapper::setViewName(const QString &name)
{
    if (m_setViewNameCallback) {
        const void *thisPtr = this;
        m_setViewNameCallback(const_cast<void *>(thisPtr), name);
    } else {
        ::KDDockWidgets::flutter::DockWidget::setViewName(name);
    }
}
void DockWidget_wrapper::setViewName_nocallback(const QString &name)
{
    ::KDDockWidgets::flutter::DockWidget::setViewName(name);
}
void DockWidget_wrapper::setVisible(bool visible)
{
    if (m_setVisibleCallback) {
        const void *thisPtr = this;
        m_setVisibleCallback(const_cast<void *>(thisPtr), visible);
    } else {
        ::KDDockWidgets::flutter::DockWidget::setVisible(visible);
    }
}
void DockWidget_wrapper::setVisible_nocallback(bool visible)
{
    ::KDDockWidgets::flutter::DockWidget::setVisible(visible);
}
void DockWidget_wrapper::setWidth(int w)
{
    if (m_setWidthCallback) {
        const void *thisPtr = this;
        m_setWidthCallback(const_cast<void *>(thisPtr), w);
    } else {
        ::KDDockWidgets::flutter::DockWidget::setWidth(w);
    }
}
void DockWidget_wrapper::setWidth_nocallback(int w)
{
    ::KDDockWidgets::flutter::DockWidget::setWidth(w);
}
void DockWidget_wrapper::setWindowOpacity(double v)
{
    if (m_setWindowOpacityCallback) {
        const void *thisPtr = this;
        m_setWindowOpacityCallback(const_cast<void *>(thisPtr), v);
    } else {
        ::KDDockWidgets::flutter::DockWidget::setWindowOpacity(v);
    }
}
void DockWidget_wrapper::setWindowOpacity_nocallback(double v)
{
    ::KDDockWidgets::flutter::DockWidget::setWindowOpacity(v);
}
void DockWidget_wrapper::setWindowTitle(const QString &title)
{
    if (m_setWindowTitleCallback) {
        const void *thisPtr = this;
        m_setWindowTitleCallback(const_cast<void *>(thisPtr), title);
    } else {
        ::KDDockWidgets::flutter::DockWidget::setWindowTitle(title);
    }
}
void DockWidget_wrapper::setWindowTitle_nocallback(const QString &title)
{
    ::KDDockWidgets::flutter::DockWidget::setWindowTitle(title);
}
void DockWidget_wrapper::setZOrder(int z)
{
    if (m_setZOrderCallback) {
        const void *thisPtr = this;
        m_setZOrderCallback(const_cast<void *>(thisPtr), z);
    } else {
        ::KDDockWidgets::flutter::DockWidget::setZOrder(z);
    }
}
void DockWidget_wrapper::setZOrder_nocallback(int z)
{
    ::KDDockWidgets::flutter::DockWidget::setZOrder(z);
}
void DockWidget_wrapper::show()
{
    if (m_showCallback) {
        const void *thisPtr = this;
        m_showCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::flutter::DockWidget::show();
    }
}
void DockWidget_wrapper::show_nocallback()
{
    ::KDDockWidgets::flutter::DockWidget::show();
}
void DockWidget_wrapper::showMaximized()
{
    if (m_showMaximizedCallback) {
        const void *thisPtr = this;
        m_showMaximizedCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::flutter::DockWidget::showMaximized();
    }
}
void DockWidget_wrapper::showMaximized_nocallback()
{
    ::KDDockWidgets::flutter::DockWidget::showMaximized();
}
void DockWidget_wrapper::showMinimized()
{
    if (m_showMinimizedCallback) {
        const void *thisPtr = this;
        m_showMinimizedCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::flutter::DockWidget::showMinimized();
    }
}
void DockWidget_wrapper::showMinimized_nocallback()
{
    ::KDDockWidgets::flutter::DockWidget::showMinimized();
}
void DockWidget_wrapper::showNormal()
{
    if (m_showNormalCallback) {
        const void *thisPtr = this;
        m_showNormalCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::flutter::DockWidget::showNormal();
    }
}
void DockWidget_wrapper::showNormal_nocallback()
{
    ::KDDockWidgets::flutter::DockWidget::showNormal();
}
void DockWidget_wrapper::update()
{
    if (m_updateCallback) {
        const void *thisPtr = this;
        m_updateCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::flutter::DockWidget::update();
    }
}
void DockWidget_wrapper::update_nocallback()
{
    ::KDDockWidgets::flutter::DockWidget::update();
}
QString DockWidget_wrapper::viewName() const
{
    if (m_viewNameCallback) {
        const void *thisPtr = this;
        return *m_viewNameCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::flutter::DockWidget::viewName();
    }
}
QString DockWidget_wrapper::viewName_nocallback() const
{
    return ::KDDockWidgets::flutter::DockWidget::viewName();
}
int DockWidget_wrapper::zOrder() const
{
    if (m_zOrderCallback) {
        const void *thisPtr = this;
        return m_zOrderCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::flutter::DockWidget::zOrder();
    }
}
int DockWidget_wrapper::zOrder_nocallback() const
{
    return ::KDDockWidgets::flutter::DockWidget::zOrder();
}
DockWidget_wrapper::~DockWidget_wrapper()
{
}

}
}
static KDDockWidgets::flutter::DockWidget *fromPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgets::flutter::DockWidget *>(ptr);
}
static KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper *fromWrapperPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper *>(ptr);
}
extern "C" {
void c_KDDockWidgets__flutter__DockWidget_Finalizer(void *cppObj)
{
    delete reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper *>(cppObj);
}
void *c_KDDockWidgets__flutter__DockWidget__constructor_QString_DockWidgetOptions_LayoutSaverOptions(const char *uniqueName_, int options_, int layoutSaverOptions_)
{
    const auto uniqueName = QString::fromUtf8(uniqueName_);
    auto options = static_cast<QFlags<KDDockWidgets::DockWidgetOption>>(options_);
    auto layoutSaverOptions = static_cast<QFlags<KDDockWidgets::LayoutSaverOption>>(layoutSaverOptions_);
    auto ptr = new KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper(uniqueName, options, layoutSaverOptions);
    return reinterpret_cast<void *>(ptr);
}
// activateWindow()
void c_KDDockWidgets__flutter__DockWidget__activateWindow(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->activateWindow_nocallback();} else {    return targetPtr->activateWindow();} }();
}
// close()
bool c_KDDockWidgets__flutter__DockWidget__close(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->close_nocallback();} else {    return targetPtr->close();} }();
    return result;
}
// createPlatformWindow()
void c_KDDockWidgets__flutter__DockWidget__createPlatformWindow(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->createPlatformWindow_nocallback();} else {    return targetPtr->createPlatformWindow();} }();
}
// dockWidget() const
void *c_KDDockWidgets__flutter__DockWidget__dockWidget(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->dockWidget();
    return result;
}
// flags() const
int c_KDDockWidgets__flutter__DockWidget__flags(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->flags_nocallback();} else {    return targetPtr->flags();} }();
    return result;
}
// geometry() const
void *c_KDDockWidgets__flutter__DockWidget__geometry(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Rect> { [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->geometry_nocallback();} else {    return targetPtr->geometry();} }() };
    return result;
}
// grabMouse()
void c_KDDockWidgets__flutter__DockWidget__grabMouse(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->grabMouse_nocallback();} else {    return targetPtr->grabMouse();} }();
}
// hasFocus() const
bool c_KDDockWidgets__flutter__DockWidget__hasFocus(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->hasFocus_nocallback();} else {    return targetPtr->hasFocus();} }();
    return result;
}
// hide()
void c_KDDockWidgets__flutter__DockWidget__hide(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->hide_nocallback();} else {    return targetPtr->hide();} }();
}
// init()
void c_KDDockWidgets__flutter__DockWidget__init(void *thisObj)
{
    fromWrapperPtr(thisObj)->init_nocallback();
}
// isActiveWindow() const
bool c_KDDockWidgets__flutter__DockWidget__isActiveWindow(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->isActiveWindow_nocallback();} else {    return targetPtr->isActiveWindow();} }();
    return result;
}
// isExplicitlyHidden() const
bool c_KDDockWidgets__flutter__DockWidget__isExplicitlyHidden(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->isExplicitlyHidden_nocallback();} else {    return targetPtr->isExplicitlyHidden();} }();
    return result;
}
// isMaximized() const
bool c_KDDockWidgets__flutter__DockWidget__isMaximized(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->isMaximized_nocallback();} else {    return targetPtr->isMaximized();} }();
    return result;
}
// isMinimized() const
bool c_KDDockWidgets__flutter__DockWidget__isMinimized(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->isMinimized_nocallback();} else {    return targetPtr->isMinimized();} }();
    return result;
}
// isMounted() const
bool c_KDDockWidgets__flutter__DockWidget__isMounted(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->isMounted_nocallback();} else {    return targetPtr->isMounted();} }();
    return result;
}
// isNull() const
bool c_KDDockWidgets__flutter__DockWidget__isNull(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->isNull_nocallback();} else {    return targetPtr->isNull();} }();
    return result;
}
// isRootView() const
bool c_KDDockWidgets__flutter__DockWidget__isRootView(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->isRootView_nocallback();} else {    return targetPtr->isRootView();} }();
    return result;
}
// isVisible() const
bool c_KDDockWidgets__flutter__DockWidget__isVisible(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->isVisible_nocallback();} else {    return targetPtr->isVisible();} }();
    return result;
}
// mapFromGlobal(KDDockWidgets::Point globalPt) const
void *c_KDDockWidgets__flutter__DockWidget__mapFromGlobal_Point(void *thisObj, void *globalPt_)
{
    assert(globalPt_);
    auto &globalPt = *reinterpret_cast<KDDockWidgets::Point *>(globalPt_);
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Point> { [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->mapFromGlobal_nocallback(globalPt);} else {    return targetPtr->mapFromGlobal(globalPt);} }() };
    return result;
}
// mapTo(KDDockWidgets::Core::View * parent, KDDockWidgets::Point pos) const
void *c_KDDockWidgets__flutter__DockWidget__mapTo_View_Point(void *thisObj, void *parent_, void *pos_)
{
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    assert(pos_);
    auto &pos = *reinterpret_cast<KDDockWidgets::Point *>(pos_);
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Point> { [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->mapTo_nocallback(parent,pos);} else {    return targetPtr->mapTo(parent,pos);} }() };
    return result;
}
// mapToGlobal(KDDockWidgets::Point localPt) const
void *c_KDDockWidgets__flutter__DockWidget__mapToGlobal_Point(void *thisObj, void *localPt_)
{
    assert(localPt_);
    auto &localPt = *reinterpret_cast<KDDockWidgets::Point *>(localPt_);
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Point> { [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->mapToGlobal_nocallback(localPt);} else {    return targetPtr->mapToGlobal(localPt);} }() };
    return result;
}
// maxSizeHint() const
void *c_KDDockWidgets__flutter__DockWidget__maxSizeHint(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Size> { [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->maxSizeHint_nocallback();} else {    return targetPtr->maxSizeHint();} }() };
    return result;
}
// minSize() const
void *c_KDDockWidgets__flutter__DockWidget__minSize(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Size> { [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->minSize_nocallback();} else {    return targetPtr->minSize();} }() };
    return result;
}
// move(int x, int y)
void c_KDDockWidgets__flutter__DockWidget__move_int_int(void *thisObj, int x, int y)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->move_nocallback(x,y);} else {    return targetPtr->move(x,y);} }();
}
// normalGeometry() const
void *c_KDDockWidgets__flutter__DockWidget__normalGeometry(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Rect> { [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->normalGeometry_nocallback();} else {    return targetPtr->normalGeometry();} }() };
    return result;
}
// onChildAdded(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__DockWidget__onChildAdded_View(void *thisObj, void *childView_)
{
    auto childView = reinterpret_cast<KDDockWidgets::Core::View *>(childView_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->onChildAdded_nocallback(childView);} else {    return targetPtr->onChildAdded(childView);} }();
}
// onChildRemoved(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__DockWidget__onChildRemoved_View(void *thisObj, void *childView_)
{
    auto childView = reinterpret_cast<KDDockWidgets::Core::View *>(childView_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->onChildRemoved_nocallback(childView);} else {    return targetPtr->onChildRemoved(childView);} }();
}
// onChildVisibilityChanged(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__DockWidget__onChildVisibilityChanged_View(void *thisObj, void *childView_)
{
    auto childView = reinterpret_cast<KDDockWidgets::Core::View *>(childView_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->onChildVisibilityChanged_nocallback(childView);} else {    return targetPtr->onChildVisibilityChanged(childView);} }();
}
// onGeometryChanged()
void c_KDDockWidgets__flutter__DockWidget__onGeometryChanged(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->onGeometryChanged_nocallback();} else {    return targetPtr->onGeometryChanged();} }();
}
// onRebuildRequested()
void c_KDDockWidgets__flutter__DockWidget__onRebuildRequested(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->onRebuildRequested_nocallback();} else {    return targetPtr->onRebuildRequested();} }();
}
// onResize(int h, int w)
bool c_KDDockWidgets__flutter__DockWidget__onResize_int_int(void *thisObj, int h, int w)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->onResize_nocallback(h,w);} else {    return targetPtr->onResize(h,w);} }();
    return result;
}
// raise()
void c_KDDockWidgets__flutter__DockWidget__raise(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->raise_nocallback();} else {    return targetPtr->raise();} }();
}
// raiseAndActivate()
void c_KDDockWidgets__flutter__DockWidget__raiseAndActivate(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->raiseAndActivate_nocallback();} else {    return targetPtr->raiseAndActivate();} }();
}
// raiseChild(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__DockWidget__raiseChild_View(void *thisObj, void *childView_)
{
    auto childView = reinterpret_cast<KDDockWidgets::Core::View *>(childView_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->raiseChild_nocallback(childView);} else {    return targetPtr->raiseChild(childView);} }();
}
// raiseWindow(KDDockWidgets::Core::View * rootView)
void c_KDDockWidgets__flutter__DockWidget__raiseWindow_View(void *thisObj, void *rootView_)
{
    auto rootView = reinterpret_cast<KDDockWidgets::Core::View *>(rootView_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->raiseWindow_nocallback(rootView);} else {    return targetPtr->raiseWindow(rootView);} }();
}
// releaseKeyboard()
void c_KDDockWidgets__flutter__DockWidget__releaseKeyboard(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->releaseKeyboard_nocallback();} else {    return targetPtr->releaseKeyboard();} }();
}
// releaseMouse()
void c_KDDockWidgets__flutter__DockWidget__releaseMouse(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->releaseMouse_nocallback();} else {    return targetPtr->releaseMouse();} }();
}
// setCursor(Qt::CursorShape shape)
void c_KDDockWidgets__flutter__DockWidget__setCursor_CursorShape(void *thisObj, int shape)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setCursor_nocallback(static_cast<Qt::CursorShape>(shape));} else {    return targetPtr->setCursor(static_cast<Qt::CursorShape>(shape));} }();
}
// setFixedHeight(int h)
void c_KDDockWidgets__flutter__DockWidget__setFixedHeight_int(void *thisObj, int h)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setFixedHeight_nocallback(h);} else {    return targetPtr->setFixedHeight(h);} }();
}
// setFixedWidth(int w)
void c_KDDockWidgets__flutter__DockWidget__setFixedWidth_int(void *thisObj, int w)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setFixedWidth_nocallback(w);} else {    return targetPtr->setFixedWidth(w);} }();
}
// setGeometry(KDDockWidgets::Rect geometry)
void c_KDDockWidgets__flutter__DockWidget__setGeometry_Rect(void *thisObj, void *geometry_)
{
    assert(geometry_);
    auto &geometry = *reinterpret_cast<KDDockWidgets::Rect *>(geometry_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setGeometry_nocallback(geometry);} else {    return targetPtr->setGeometry(geometry);} }();
}
// setHeight(int h)
void c_KDDockWidgets__flutter__DockWidget__setHeight_int(void *thisObj, int h)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setHeight_nocallback(h);} else {    return targetPtr->setHeight(h);} }();
}
// setMaximumSize(KDDockWidgets::Size sz)
void c_KDDockWidgets__flutter__DockWidget__setMaximumSize_Size(void *thisObj, void *sz_)
{
    assert(sz_);
    auto &sz = *reinterpret_cast<KDDockWidgets::Size *>(sz_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setMaximumSize_nocallback(sz);} else {    return targetPtr->setMaximumSize(sz);} }();
}
// setMinimumSize(KDDockWidgets::Size sz)
void c_KDDockWidgets__flutter__DockWidget__setMinimumSize_Size(void *thisObj, void *sz_)
{
    assert(sz_);
    auto &sz = *reinterpret_cast<KDDockWidgets::Size *>(sz_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setMinimumSize_nocallback(sz);} else {    return targetPtr->setMinimumSize(sz);} }();
}
// setMouseTracking(bool enable)
void c_KDDockWidgets__flutter__DockWidget__setMouseTracking_bool(void *thisObj, bool enable)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setMouseTracking_nocallback(enable);} else {    return targetPtr->setMouseTracking(enable);} }();
}
// setParent(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__flutter__DockWidget__setParent_View(void *thisObj, void *parent_)
{
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setParent_nocallback(parent);} else {    return targetPtr->setParent(parent);} }();
}
// setSize(int w, int h)
void c_KDDockWidgets__flutter__DockWidget__setSize_int_int(void *thisObj, int w, int h)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setSize_nocallback(w,h);} else {    return targetPtr->setSize(w,h);} }();
}
// setViewName(const QString & name)
void c_KDDockWidgets__flutter__DockWidget__setViewName_QString(void *thisObj, const char *name_)
{
    const auto name = QString::fromUtf8(name_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setViewName_nocallback(name);} else {    return targetPtr->setViewName(name);} }();
    free(( char * )name_);
}
// setVisible(bool visible)
void c_KDDockWidgets__flutter__DockWidget__setVisible_bool(void *thisObj, bool visible)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setVisible_nocallback(visible);} else {    return targetPtr->setVisible(visible);} }();
}
// setWidth(int w)
void c_KDDockWidgets__flutter__DockWidget__setWidth_int(void *thisObj, int w)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setWidth_nocallback(w);} else {    return targetPtr->setWidth(w);} }();
}
// setWindowOpacity(double v)
void c_KDDockWidgets__flutter__DockWidget__setWindowOpacity_double(void *thisObj, double v)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setWindowOpacity_nocallback(v);} else {    return targetPtr->setWindowOpacity(v);} }();
}
// setWindowTitle(const QString & title)
void c_KDDockWidgets__flutter__DockWidget__setWindowTitle_QString(void *thisObj, const char *title_)
{
    const auto title = QString::fromUtf8(title_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setWindowTitle_nocallback(title);} else {    return targetPtr->setWindowTitle(title);} }();
    free(( char * )title_);
}
// setZOrder(int z)
void c_KDDockWidgets__flutter__DockWidget__setZOrder_int(void *thisObj, int z)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setZOrder_nocallback(z);} else {    return targetPtr->setZOrder(z);} }();
}
// show()
void c_KDDockWidgets__flutter__DockWidget__show(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->show_nocallback();} else {    return targetPtr->show();} }();
}
// showMaximized()
void c_KDDockWidgets__flutter__DockWidget__showMaximized(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->showMaximized_nocallback();} else {    return targetPtr->showMaximized();} }();
}
// showMinimized()
void c_KDDockWidgets__flutter__DockWidget__showMinimized(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->showMinimized_nocallback();} else {    return targetPtr->showMinimized();} }();
}
// showNormal()
void c_KDDockWidgets__flutter__DockWidget__showNormal(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->showNormal_nocallback();} else {    return targetPtr->showNormal();} }();
}
// update()
void c_KDDockWidgets__flutter__DockWidget__update(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->update_nocallback();} else {    return targetPtr->update();} }();
}
// viewName() const
void *c_KDDockWidgets__flutter__DockWidget__viewName(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<QString> { [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->viewName_nocallback();} else {    return targetPtr->viewName();} }() };
    return result;
}
// zOrder() const
int c_KDDockWidgets__flutter__DockWidget__zOrder(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->zOrder_nocallback();} else {    return targetPtr->zOrder();} }();
    return result;
}
void c_KDDockWidgets__flutter__DockWidget__destructor(void *thisObj)
{
    delete fromPtr(thisObj);
}
void c_KDDockWidgets__flutter__DockWidget__registerVirtualMethodCallback(void *ptr, void *callback, int methodId)
{
    auto wrapper = fromWrapperPtr(ptr);
    switch (methodId) {
    case 342:
        wrapper->m_activateWindowCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_activateWindow>(callback);
        break;
    case 352:
        wrapper->m_closeCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_close>(callback);
        break;
    case 354:
        wrapper->m_createPlatformWindowCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_createPlatformWindow>(callback);
        break;
    case 360:
        wrapper->m_flagsCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_flags>(callback);
        break;
    case 361:
        wrapper->m_geometryCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_geometry>(callback);
        break;
    case 362:
        wrapper->m_grabMouseCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_grabMouse>(callback);
        break;
    case 365:
        wrapper->m_hasFocusCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_hasFocus>(callback);
        break;
    case 367:
        wrapper->m_hideCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_hide>(callback);
        break;
    case 369:
        wrapper->m_initCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_init>(callback);
        break;
    case 371:
        wrapper->m_isActiveWindowCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_isActiveWindow>(callback);
        break;
    case 372:
        wrapper->m_isExplicitlyHiddenCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_isExplicitlyHidden>(callback);
        break;
    case 375:
        wrapper->m_isMaximizedCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_isMaximized>(callback);
        break;
    case 376:
        wrapper->m_isMinimizedCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_isMinimized>(callback);
        break;
    case 450:
        wrapper->m_isMountedCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_isMounted>(callback);
        break;
    case 377:
        wrapper->m_isNullCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_isNull>(callback);
        break;
    case 378:
        wrapper->m_isRootViewCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_isRootView>(callback);
        break;
    case 379:
        wrapper->m_isVisibleCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_isVisible>(callback);
        break;
    case 380:
        wrapper->m_mapFromGlobalCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_mapFromGlobal>(callback);
        break;
    case 381:
        wrapper->m_mapToCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_mapTo>(callback);
        break;
    case 382:
        wrapper->m_mapToGlobalCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_mapToGlobal>(callback);
        break;
    case 383:
        wrapper->m_maxSizeHintCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_maxSizeHint>(callback);
        break;
    case 384:
        wrapper->m_minSizeCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_minSize>(callback);
        break;
    case 388:
        wrapper->m_move_2Callback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_move_2>(callback);
        break;
    case 389:
        wrapper->m_normalGeometryCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_normalGeometry>(callback);
        break;
    case 453:
        wrapper->m_onChildAddedCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_onChildAdded>(callback);
        break;
    case 454:
        wrapper->m_onChildRemovedCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_onChildRemoved>(callback);
        break;
    case 455:
        wrapper->m_onChildVisibilityChangedCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_onChildVisibilityChanged>(callback);
        break;
    case 457:
        wrapper->m_onGeometryChangedCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_onGeometryChanged>(callback);
        break;
    case 459:
        wrapper->m_onRebuildRequestedCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_onRebuildRequested>(callback);
        break;
    case 391:
        wrapper->m_onResize_2Callback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_onResize_2>(callback);
        break;
    case 393:
        wrapper->m_raiseCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_raise>(callback);
        break;
    case 394:
        wrapper->m_raiseAndActivateCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_raiseAndActivate>(callback);
        break;
    case 462:
        wrapper->m_raiseChildCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_raiseChild>(callback);
        break;
    case 463:
        wrapper->m_raiseWindowCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_raiseWindow>(callback);
        break;
    case 396:
        wrapper->m_releaseKeyboardCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_releaseKeyboard>(callback);
        break;
    case 397:
        wrapper->m_releaseMouseCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_releaseMouse>(callback);
        break;
    case 401:
        wrapper->m_setCursorCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_setCursor>(callback);
        break;
    case 402:
        wrapper->m_setFixedHeightCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_setFixedHeight>(callback);
        break;
    case 403:
        wrapper->m_setFixedWidthCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_setFixedWidth>(callback);
        break;
    case 404:
        wrapper->m_setGeometryCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_setGeometry>(callback);
        break;
    case 405:
        wrapper->m_setHeightCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_setHeight>(callback);
        break;
    case 406:
        wrapper->m_setMaximumSizeCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_setMaximumSize>(callback);
        break;
    case 407:
        wrapper->m_setMinimumSizeCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_setMinimumSize>(callback);
        break;
    case 408:
        wrapper->m_setMouseTrackingCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_setMouseTracking>(callback);
        break;
    case 409:
        wrapper->m_setParentCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_setParent>(callback);
        break;
    case 411:
        wrapper->m_setSize_2Callback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_setSize_2>(callback);
        break;
    case 412:
        wrapper->m_setViewNameCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_setViewName>(callback);
        break;
    case 413:
        wrapper->m_setVisibleCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_setVisible>(callback);
        break;
    case 414:
        wrapper->m_setWidthCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_setWidth>(callback);
        break;
    case 415:
        wrapper->m_setWindowOpacityCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_setWindowOpacity>(callback);
        break;
    case 416:
        wrapper->m_setWindowTitleCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_setWindowTitle>(callback);
        break;
    case 417:
        wrapper->m_setZOrderCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_setZOrder>(callback);
        break;
    case 418:
        wrapper->m_showCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_show>(callback);
        break;
    case 419:
        wrapper->m_showMaximizedCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_showMaximized>(callback);
        break;
    case 420:
        wrapper->m_showMinimizedCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_showMinimized>(callback);
        break;
    case 421:
        wrapper->m_showNormalCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_showNormal>(callback);
        break;
    case 423:
        wrapper->m_updateCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_update>(callback);
        break;
    case 424:
        wrapper->m_viewNameCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_viewName>(callback);
        break;
    case 428:
        wrapper->m_zOrderCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::DockWidget_wrapper::Callback_zOrder>(callback);
        break;
    }
}
}
