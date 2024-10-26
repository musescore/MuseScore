/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "View_c.h"


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
namespace KDDWBindingsCore {
void View_wrapper::activateWindow()
{
    if (m_activateWindowCallback) {
        const void *thisPtr = this;
        m_activateWindowCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "activateWindow: Warning: Calling pure-virtual\n";
        return;
    }
}
void View_wrapper::activateWindow_nocallback()
{
    std::cerr << "activateWindow: Warning: Calling pure-virtual\n";
    return;
}
KDDockWidgets::Core::DockWidget *View_wrapper::asDockWidgetController() const
{
    return ::KDDockWidgets::Core::View::asDockWidgetController();
}
KDDockWidgets::Core::DropArea *View_wrapper::asDropAreaController() const
{
    return ::KDDockWidgets::Core::View::asDropAreaController();
}
KDDockWidgets::Core::FloatingWindow *View_wrapper::asFloatingWindowController() const
{
    return ::KDDockWidgets::Core::View::asFloatingWindowController();
}
KDDockWidgets::Core::Group *View_wrapper::asGroupController() const
{
    return ::KDDockWidgets::Core::View::asGroupController();
}
KDDockWidgets::Core::Layout *View_wrapper::asLayout() const
{
    return ::KDDockWidgets::Core::View::asLayout();
}
KDDockWidgets::Core::MainWindow *View_wrapper::asMainWindowController() const
{
    return ::KDDockWidgets::Core::View::asMainWindowController();
}
KDDockWidgets::Core::Stack *View_wrapper::asStackController() const
{
    return ::KDDockWidgets::Core::View::asStackController();
}
KDDockWidgets::Core::TabBar *View_wrapper::asTabBarController() const
{
    return ::KDDockWidgets::Core::View::asTabBarController();
}
KDDockWidgets::Core::TitleBar *View_wrapper::asTitleBarController() const
{
    return ::KDDockWidgets::Core::View::asTitleBarController();
}
bool View_wrapper::close()
{
    if (m_closeCallback) {
        const void *thisPtr = this;
        return m_closeCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "close: Warning: Calling pure-virtual\n";
        return {};
    }
}
bool View_wrapper::close_nocallback()
{
    std::cerr << "close: Warning: Calling pure-virtual\n";
    return {};
}
KDDockWidgets::Core::Controller *View_wrapper::controller() const
{
    return ::KDDockWidgets::Core::View::controller();
}
void View_wrapper::createPlatformWindow()
{
    if (m_createPlatformWindowCallback) {
        const void *thisPtr = this;
        m_createPlatformWindowCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::Core::View::createPlatformWindow();
    }
}
void View_wrapper::createPlatformWindow_nocallback()
{
    ::KDDockWidgets::Core::View::createPlatformWindow();
}
bool View_wrapper::deliverViewEventToFilters(KDDockWidgets::Event *e)
{
    return ::KDDockWidgets::Core::View::deliverViewEventToFilters(e);
}
void View_wrapper::dumpDebug()
{
    ::KDDockWidgets::Core::View::dumpDebug();
}
bool View_wrapper::equals(const KDDockWidgets::Core::View *one, const KDDockWidgets::Core::View *two)
{
    return ::KDDockWidgets::Core::View::equals(one, two);
}
bool View_wrapper::equals(const KDDockWidgets::Core::View *other) const
{
    return ::KDDockWidgets::Core::View::equals(other);
}
KDDockWidgets::Core::Controller *View_wrapper::firstParentOfType(KDDockWidgets::Core::View *view, KDDockWidgets::Core::ViewType arg__2)
{
    return ::KDDockWidgets::Core::View::firstParentOfType(view, arg__2);
}
Qt::WindowFlags View_wrapper::flags() const
{
    if (m_flagsCallback) {
        const void *thisPtr = this;
        return m_flagsCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "flags: Warning: Calling pure-virtual\n";
        return {};
    }
}
Qt::WindowFlags View_wrapper::flags_nocallback() const
{
    std::cerr << "flags: Warning: Calling pure-virtual\n";
    return {};
}
KDDockWidgets::Rect View_wrapper::geometry() const
{
    if (m_geometryCallback) {
        const void *thisPtr = this;
        return *m_geometryCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "geometry: Warning: Calling pure-virtual\n";
        return {};
    }
}
KDDockWidgets::Rect View_wrapper::geometry_nocallback() const
{
    std::cerr << "geometry: Warning: Calling pure-virtual\n";
    return {};
}
void View_wrapper::grabMouse()
{
    if (m_grabMouseCallback) {
        const void *thisPtr = this;
        m_grabMouseCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "grabMouse: Warning: Calling pure-virtual\n";
        return;
    }
}
void View_wrapper::grabMouse_nocallback()
{
    std::cerr << "grabMouse: Warning: Calling pure-virtual\n";
    return;
}
KDDockWidgets::Size View_wrapper::hardcodedMinimumSize()
{
    return ::KDDockWidgets::Core::View::hardcodedMinimumSize();
}
bool View_wrapper::hasFocus() const
{
    if (m_hasFocusCallback) {
        const void *thisPtr = this;
        return m_hasFocusCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "hasFocus: Warning: Calling pure-virtual\n";
        return {};
    }
}
bool View_wrapper::hasFocus_nocallback() const
{
    std::cerr << "hasFocus: Warning: Calling pure-virtual\n";
    return {};
}
int View_wrapper::height() const
{
    return ::KDDockWidgets::Core::View::height();
}
void View_wrapper::hide()
{
    if (m_hideCallback) {
        const void *thisPtr = this;
        m_hideCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "hide: Warning: Calling pure-virtual\n";
        return;
    }
}
void View_wrapper::hide_nocallback()
{
    std::cerr << "hide: Warning: Calling pure-virtual\n";
    return;
}
bool View_wrapper::inDtor() const
{
    return ::KDDockWidgets::Core::View::inDtor();
}
void View_wrapper::init()
{
    if (m_initCallback) {
        const void *thisPtr = this;
        m_initCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::Core::View::init();
    }
}
void View_wrapper::init_nocallback()
{
    ::KDDockWidgets::Core::View::init();
}
bool View_wrapper::isActiveWindow() const
{
    if (m_isActiveWindowCallback) {
        const void *thisPtr = this;
        return m_isActiveWindowCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "isActiveWindow: Warning: Calling pure-virtual\n";
        return {};
    }
}
bool View_wrapper::isActiveWindow_nocallback() const
{
    std::cerr << "isActiveWindow: Warning: Calling pure-virtual\n";
    return {};
}
bool View_wrapper::isExplicitlyHidden() const
{
    if (m_isExplicitlyHiddenCallback) {
        const void *thisPtr = this;
        return m_isExplicitlyHiddenCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "isExplicitlyHidden: Warning: Calling pure-virtual\n";
        return {};
    }
}
bool View_wrapper::isExplicitlyHidden_nocallback() const
{
    std::cerr << "isExplicitlyHidden: Warning: Calling pure-virtual\n";
    return {};
}
bool View_wrapper::isFixedHeight() const
{
    return ::KDDockWidgets::Core::View::isFixedHeight();
}
bool View_wrapper::isFixedWidth() const
{
    return ::KDDockWidgets::Core::View::isFixedWidth();
}
bool View_wrapper::isMaximized() const
{
    if (m_isMaximizedCallback) {
        const void *thisPtr = this;
        return m_isMaximizedCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "isMaximized: Warning: Calling pure-virtual\n";
        return {};
    }
}
bool View_wrapper::isMaximized_nocallback() const
{
    std::cerr << "isMaximized: Warning: Calling pure-virtual\n";
    return {};
}
bool View_wrapper::isMinimized() const
{
    if (m_isMinimizedCallback) {
        const void *thisPtr = this;
        return m_isMinimizedCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "isMinimized: Warning: Calling pure-virtual\n";
        return {};
    }
}
bool View_wrapper::isMinimized_nocallback() const
{
    std::cerr << "isMinimized: Warning: Calling pure-virtual\n";
    return {};
}
bool View_wrapper::isNull() const
{
    if (m_isNullCallback) {
        const void *thisPtr = this;
        return m_isNullCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::Core::View::isNull();
    }
}
bool View_wrapper::isNull_nocallback() const
{
    return ::KDDockWidgets::Core::View::isNull();
}
bool View_wrapper::isRootView() const
{
    if (m_isRootViewCallback) {
        const void *thisPtr = this;
        return m_isRootViewCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "isRootView: Warning: Calling pure-virtual\n";
        return {};
    }
}
bool View_wrapper::isRootView_nocallback() const
{
    std::cerr << "isRootView: Warning: Calling pure-virtual\n";
    return {};
}
bool View_wrapper::isVisible() const
{
    if (m_isVisibleCallback) {
        const void *thisPtr = this;
        return m_isVisibleCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "isVisible: Warning: Calling pure-virtual\n";
        return {};
    }
}
bool View_wrapper::isVisible_nocallback() const
{
    std::cerr << "isVisible: Warning: Calling pure-virtual\n";
    return {};
}
KDDockWidgets::Point View_wrapper::mapFromGlobal(KDDockWidgets::Point arg__1) const
{
    if (m_mapFromGlobalCallback) {
        const void *thisPtr = this;
        return *m_mapFromGlobalCallback(const_cast<void *>(thisPtr), &arg__1);
    } else {
        std::cerr << "mapFromGlobal: Warning: Calling pure-virtual\n";
        return {};
    }
}
KDDockWidgets::Point View_wrapper::mapFromGlobal_nocallback(KDDockWidgets::Point arg__1) const
{
    std::cerr << "mapFromGlobal: Warning: Calling pure-virtual\n";
    return {};
}
KDDockWidgets::Point View_wrapper::mapTo(KDDockWidgets::Core::View *arg__1, KDDockWidgets::Point arg__2) const
{
    if (m_mapToCallback) {
        const void *thisPtr = this;
        return *m_mapToCallback(const_cast<void *>(thisPtr), arg__1, &arg__2);
    } else {
        std::cerr << "mapTo: Warning: Calling pure-virtual\n";
        return {};
    }
}
KDDockWidgets::Point View_wrapper::mapTo_nocallback(KDDockWidgets::Core::View *arg__1, KDDockWidgets::Point arg__2) const
{
    std::cerr << "mapTo: Warning: Calling pure-virtual\n";
    return {};
}
KDDockWidgets::Point View_wrapper::mapToGlobal(KDDockWidgets::Point arg__1) const
{
    if (m_mapToGlobalCallback) {
        const void *thisPtr = this;
        return *m_mapToGlobalCallback(const_cast<void *>(thisPtr), &arg__1);
    } else {
        std::cerr << "mapToGlobal: Warning: Calling pure-virtual\n";
        return {};
    }
}
KDDockWidgets::Point View_wrapper::mapToGlobal_nocallback(KDDockWidgets::Point arg__1) const
{
    std::cerr << "mapToGlobal: Warning: Calling pure-virtual\n";
    return {};
}
KDDockWidgets::Size View_wrapper::maxSizeHint() const
{
    if (m_maxSizeHintCallback) {
        const void *thisPtr = this;
        return *m_maxSizeHintCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "maxSizeHint: Warning: Calling pure-virtual\n";
        return {};
    }
}
KDDockWidgets::Size View_wrapper::maxSizeHint_nocallback() const
{
    std::cerr << "maxSizeHint: Warning: Calling pure-virtual\n";
    return {};
}
KDDockWidgets::Size View_wrapper::minSize() const
{
    if (m_minSizeCallback) {
        const void *thisPtr = this;
        return *m_minSizeCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "minSize: Warning: Calling pure-virtual\n";
        return {};
    }
}
KDDockWidgets::Size View_wrapper::minSize_nocallback() const
{
    std::cerr << "minSize: Warning: Calling pure-virtual\n";
    return {};
}
int View_wrapper::minimumHeight() const
{
    return ::KDDockWidgets::Core::View::minimumHeight();
}
int View_wrapper::minimumWidth() const
{
    return ::KDDockWidgets::Core::View::minimumWidth();
}
void View_wrapper::move(KDDockWidgets::Point arg__1)
{
    ::KDDockWidgets::Core::View::move(arg__1);
}
void View_wrapper::move(int x, int y)
{
    if (m_move_2Callback) {
        const void *thisPtr = this;
        m_move_2Callback(const_cast<void *>(thisPtr), x, y);
    } else {
        std::cerr << "move: Warning: Calling pure-virtual\n";
        return;
    }
}
void View_wrapper::move_nocallback(int x, int y)
{
    std::cerr << "move: Warning: Calling pure-virtual\n";
    return;
}
KDDockWidgets::Rect View_wrapper::normalGeometry() const
{
    if (m_normalGeometryCallback) {
        const void *thisPtr = this;
        return *m_normalGeometryCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "normalGeometry: Warning: Calling pure-virtual\n";
        return {};
    }
}
KDDockWidgets::Rect View_wrapper::normalGeometry_nocallback() const
{
    std::cerr << "normalGeometry: Warning: Calling pure-virtual\n";
    return {};
}
bool View_wrapper::onResize(KDDockWidgets::Size arg__1)
{
    return ::KDDockWidgets::Core::View::onResize(arg__1);
}
bool View_wrapper::onResize(int h, int w)
{
    if (m_onResize_2Callback) {
        const void *thisPtr = this;
        return m_onResize_2Callback(const_cast<void *>(thisPtr), h, w);
    } else {
        return ::KDDockWidgets::Core::View::onResize(h, w);
    }
}
bool View_wrapper::onResize_nocallback(int h, int w)
{
    return ::KDDockWidgets::Core::View::onResize(h, w);
}
KDDockWidgets::Point View_wrapper::pos() const
{
    return ::KDDockWidgets::Core::View::pos();
}
void View_wrapper::raise()
{
    if (m_raiseCallback) {
        const void *thisPtr = this;
        m_raiseCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "raise: Warning: Calling pure-virtual\n";
        return;
    }
}
void View_wrapper::raise_nocallback()
{
    std::cerr << "raise: Warning: Calling pure-virtual\n";
    return;
}
void View_wrapper::raiseAndActivate()
{
    if (m_raiseAndActivateCallback) {
        const void *thisPtr = this;
        m_raiseAndActivateCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "raiseAndActivate: Warning: Calling pure-virtual\n";
        return;
    }
}
void View_wrapper::raiseAndActivate_nocallback()
{
    std::cerr << "raiseAndActivate: Warning: Calling pure-virtual\n";
    return;
}
KDDockWidgets::Rect View_wrapper::rect() const
{
    return ::KDDockWidgets::Core::View::rect();
}
void View_wrapper::releaseKeyboard()
{
    if (m_releaseKeyboardCallback) {
        const void *thisPtr = this;
        m_releaseKeyboardCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "releaseKeyboard: Warning: Calling pure-virtual\n";
        return;
    }
}
void View_wrapper::releaseKeyboard_nocallback()
{
    std::cerr << "releaseKeyboard: Warning: Calling pure-virtual\n";
    return;
}
void View_wrapper::releaseMouse()
{
    if (m_releaseMouseCallback) {
        const void *thisPtr = this;
        m_releaseMouseCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "releaseMouse: Warning: Calling pure-virtual\n";
        return;
    }
}
void View_wrapper::releaseMouse_nocallback()
{
    std::cerr << "releaseMouse: Warning: Calling pure-virtual\n";
    return;
}
void View_wrapper::resize(KDDockWidgets::Size arg__1)
{
    ::KDDockWidgets::Core::View::resize(arg__1);
}
void View_wrapper::resize(int w, int h)
{
    ::KDDockWidgets::Core::View::resize(w, h);
}
KDDockWidgets::Size View_wrapper::screenSize() const
{
    return ::KDDockWidgets::Core::View::screenSize();
}
void View_wrapper::setCursor(Qt::CursorShape arg__1)
{
    if (m_setCursorCallback) {
        const void *thisPtr = this;
        m_setCursorCallback(const_cast<void *>(thisPtr), arg__1);
    } else {
        std::cerr << "setCursor: Warning: Calling pure-virtual\n";
        return;
    }
}
void View_wrapper::setCursor_nocallback(Qt::CursorShape arg__1)
{
    std::cerr << "setCursor: Warning: Calling pure-virtual\n";
    return;
}
void View_wrapper::setFixedHeight(int arg__1)
{
    if (m_setFixedHeightCallback) {
        const void *thisPtr = this;
        m_setFixedHeightCallback(const_cast<void *>(thisPtr), arg__1);
    } else {
        std::cerr << "setFixedHeight: Warning: Calling pure-virtual\n";
        return;
    }
}
void View_wrapper::setFixedHeight_nocallback(int arg__1)
{
    std::cerr << "setFixedHeight: Warning: Calling pure-virtual\n";
    return;
}
void View_wrapper::setFixedWidth(int arg__1)
{
    if (m_setFixedWidthCallback) {
        const void *thisPtr = this;
        m_setFixedWidthCallback(const_cast<void *>(thisPtr), arg__1);
    } else {
        std::cerr << "setFixedWidth: Warning: Calling pure-virtual\n";
        return;
    }
}
void View_wrapper::setFixedWidth_nocallback(int arg__1)
{
    std::cerr << "setFixedWidth: Warning: Calling pure-virtual\n";
    return;
}
void View_wrapper::setGeometry(KDDockWidgets::Rect arg__1)
{
    if (m_setGeometryCallback) {
        const void *thisPtr = this;
        m_setGeometryCallback(const_cast<void *>(thisPtr), &arg__1);
    } else {
        std::cerr << "setGeometry: Warning: Calling pure-virtual\n";
        return;
    }
}
void View_wrapper::setGeometry_nocallback(KDDockWidgets::Rect arg__1)
{
    std::cerr << "setGeometry: Warning: Calling pure-virtual\n";
    return;
}
void View_wrapper::setHeight(int height)
{
    if (m_setHeightCallback) {
        const void *thisPtr = this;
        m_setHeightCallback(const_cast<void *>(thisPtr), height);
    } else {
        std::cerr << "setHeight: Warning: Calling pure-virtual\n";
        return;
    }
}
void View_wrapper::setHeight_nocallback(int height)
{
    std::cerr << "setHeight: Warning: Calling pure-virtual\n";
    return;
}
void View_wrapper::setMaximumSize(KDDockWidgets::Size sz)
{
    if (m_setMaximumSizeCallback) {
        const void *thisPtr = this;
        m_setMaximumSizeCallback(const_cast<void *>(thisPtr), &sz);
    } else {
        std::cerr << "setMaximumSize: Warning: Calling pure-virtual\n";
        return;
    }
}
void View_wrapper::setMaximumSize_nocallback(KDDockWidgets::Size sz)
{
    std::cerr << "setMaximumSize: Warning: Calling pure-virtual\n";
    return;
}
void View_wrapper::setMinimumSize(KDDockWidgets::Size arg__1)
{
    if (m_setMinimumSizeCallback) {
        const void *thisPtr = this;
        m_setMinimumSizeCallback(const_cast<void *>(thisPtr), &arg__1);
    } else {
        std::cerr << "setMinimumSize: Warning: Calling pure-virtual\n";
        return;
    }
}
void View_wrapper::setMinimumSize_nocallback(KDDockWidgets::Size arg__1)
{
    std::cerr << "setMinimumSize: Warning: Calling pure-virtual\n";
    return;
}
void View_wrapper::setMouseTracking(bool arg__1)
{
    if (m_setMouseTrackingCallback) {
        const void *thisPtr = this;
        m_setMouseTrackingCallback(const_cast<void *>(thisPtr), arg__1);
    } else {
        std::cerr << "setMouseTracking: Warning: Calling pure-virtual\n";
        return;
    }
}
void View_wrapper::setMouseTracking_nocallback(bool arg__1)
{
    std::cerr << "setMouseTracking: Warning: Calling pure-virtual\n";
    return;
}
void View_wrapper::setParent(KDDockWidgets::Core::View *arg__1)
{
    if (m_setParentCallback) {
        const void *thisPtr = this;
        m_setParentCallback(const_cast<void *>(thisPtr), arg__1);
    } else {
        std::cerr << "setParent: Warning: Calling pure-virtual\n";
        return;
    }
}
void View_wrapper::setParent_nocallback(KDDockWidgets::Core::View *arg__1)
{
    std::cerr << "setParent: Warning: Calling pure-virtual\n";
    return;
}
void View_wrapper::setSize(KDDockWidgets::Size arg__1)
{
    ::KDDockWidgets::Core::View::setSize(arg__1);
}
void View_wrapper::setSize(int width, int height)
{
    if (m_setSize_2Callback) {
        const void *thisPtr = this;
        m_setSize_2Callback(const_cast<void *>(thisPtr), width, height);
    } else {
        std::cerr << "setSize: Warning: Calling pure-virtual\n";
        return;
    }
}
void View_wrapper::setSize_nocallback(int width, int height)
{
    std::cerr << "setSize: Warning: Calling pure-virtual\n";
    return;
}
void View_wrapper::setViewName(const QString &arg__1)
{
    if (m_setViewNameCallback) {
        const void *thisPtr = this;
        m_setViewNameCallback(const_cast<void *>(thisPtr), arg__1);
    } else {
        std::cerr << "setViewName: Warning: Calling pure-virtual\n";
        return;
    }
}
void View_wrapper::setViewName_nocallback(const QString &arg__1)
{
    std::cerr << "setViewName: Warning: Calling pure-virtual\n";
    return;
}
void View_wrapper::setVisible(bool arg__1)
{
    if (m_setVisibleCallback) {
        const void *thisPtr = this;
        m_setVisibleCallback(const_cast<void *>(thisPtr), arg__1);
    } else {
        std::cerr << "setVisible: Warning: Calling pure-virtual\n";
        return;
    }
}
void View_wrapper::setVisible_nocallback(bool arg__1)
{
    std::cerr << "setVisible: Warning: Calling pure-virtual\n";
    return;
}
void View_wrapper::setWidth(int width)
{
    if (m_setWidthCallback) {
        const void *thisPtr = this;
        m_setWidthCallback(const_cast<void *>(thisPtr), width);
    } else {
        std::cerr << "setWidth: Warning: Calling pure-virtual\n";
        return;
    }
}
void View_wrapper::setWidth_nocallback(int width)
{
    std::cerr << "setWidth: Warning: Calling pure-virtual\n";
    return;
}
void View_wrapper::setWindowOpacity(double arg__1)
{
    if (m_setWindowOpacityCallback) {
        const void *thisPtr = this;
        m_setWindowOpacityCallback(const_cast<void *>(thisPtr), arg__1);
    } else {
        std::cerr << "setWindowOpacity: Warning: Calling pure-virtual\n";
        return;
    }
}
void View_wrapper::setWindowOpacity_nocallback(double arg__1)
{
    std::cerr << "setWindowOpacity: Warning: Calling pure-virtual\n";
    return;
}
void View_wrapper::setWindowTitle(const QString &title)
{
    if (m_setWindowTitleCallback) {
        const void *thisPtr = this;
        m_setWindowTitleCallback(const_cast<void *>(thisPtr), title);
    } else {
        std::cerr << "setWindowTitle: Warning: Calling pure-virtual\n";
        return;
    }
}
void View_wrapper::setWindowTitle_nocallback(const QString &title)
{
    std::cerr << "setWindowTitle: Warning: Calling pure-virtual\n";
    return;
}
void View_wrapper::setZOrder(int arg__1)
{
    if (m_setZOrderCallback) {
        const void *thisPtr = this;
        m_setZOrderCallback(const_cast<void *>(thisPtr), arg__1);
    } else {
        ::KDDockWidgets::Core::View::setZOrder(arg__1);
    }
}
void View_wrapper::setZOrder_nocallback(int arg__1)
{
    ::KDDockWidgets::Core::View::setZOrder(arg__1);
}
void View_wrapper::show()
{
    if (m_showCallback) {
        const void *thisPtr = this;
        m_showCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "show: Warning: Calling pure-virtual\n";
        return;
    }
}
void View_wrapper::show_nocallback()
{
    std::cerr << "show: Warning: Calling pure-virtual\n";
    return;
}
void View_wrapper::showMaximized()
{
    if (m_showMaximizedCallback) {
        const void *thisPtr = this;
        m_showMaximizedCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "showMaximized: Warning: Calling pure-virtual\n";
        return;
    }
}
void View_wrapper::showMaximized_nocallback()
{
    std::cerr << "showMaximized: Warning: Calling pure-virtual\n";
    return;
}
void View_wrapper::showMinimized()
{
    if (m_showMinimizedCallback) {
        const void *thisPtr = this;
        m_showMinimizedCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "showMinimized: Warning: Calling pure-virtual\n";
        return;
    }
}
void View_wrapper::showMinimized_nocallback()
{
    std::cerr << "showMinimized: Warning: Calling pure-virtual\n";
    return;
}
void View_wrapper::showNormal()
{
    if (m_showNormalCallback) {
        const void *thisPtr = this;
        m_showNormalCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "showNormal: Warning: Calling pure-virtual\n";
        return;
    }
}
void View_wrapper::showNormal_nocallback()
{
    std::cerr << "showNormal: Warning: Calling pure-virtual\n";
    return;
}
KDDockWidgets::Size View_wrapper::size() const
{
    return ::KDDockWidgets::Core::View::size();
}
void View_wrapper::update()
{
    if (m_updateCallback) {
        const void *thisPtr = this;
        m_updateCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "update: Warning: Calling pure-virtual\n";
        return;
    }
}
void View_wrapper::update_nocallback()
{
    std::cerr << "update: Warning: Calling pure-virtual\n";
    return;
}
QString View_wrapper::viewName() const
{
    if (m_viewNameCallback) {
        const void *thisPtr = this;
        return *m_viewNameCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "viewName: Warning: Calling pure-virtual\n";
        return {};
    }
}
QString View_wrapper::viewName_nocallback() const
{
    std::cerr << "viewName: Warning: Calling pure-virtual\n";
    return {};
}
int View_wrapper::width() const
{
    return ::KDDockWidgets::Core::View::width();
}
int View_wrapper::x() const
{
    return ::KDDockWidgets::Core::View::x();
}
int View_wrapper::y() const
{
    return ::KDDockWidgets::Core::View::y();
}
int View_wrapper::zOrder() const
{
    if (m_zOrderCallback) {
        const void *thisPtr = this;
        return m_zOrderCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::Core::View::zOrder();
    }
}
int View_wrapper::zOrder_nocallback() const
{
    return ::KDDockWidgets::Core::View::zOrder();
}
View_wrapper::~View_wrapper()
{
}

}
}
static KDDockWidgets::Core::View *fromPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgets::Core::View *>(ptr);
}
static KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper *fromWrapperPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper *>(ptr);
}
extern "C" {
void c_KDDockWidgets__Core__View_Finalizer(void *cppObj)
{
    delete reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper *>(cppObj);
} // activateWindow()
void c_KDDockWidgets__Core__View__activateWindow(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->activateWindow_nocallback();} else {    return targetPtr->activateWindow();} }();
}
// asDockWidgetController() const
void *c_KDDockWidgets__Core__View__asDockWidgetController(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->asDockWidgetController();
    return result;
}
// asDropAreaController() const
void *c_KDDockWidgets__Core__View__asDropAreaController(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->asDropAreaController();
    return result;
}
// asFloatingWindowController() const
void *c_KDDockWidgets__Core__View__asFloatingWindowController(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->asFloatingWindowController();
    return result;
}
// asGroupController() const
void *c_KDDockWidgets__Core__View__asGroupController(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->asGroupController();
    return result;
}
// asLayout() const
void *c_KDDockWidgets__Core__View__asLayout(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->asLayout();
    return result;
}
// asMainWindowController() const
void *c_KDDockWidgets__Core__View__asMainWindowController(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->asMainWindowController();
    return result;
}
// asStackController() const
void *c_KDDockWidgets__Core__View__asStackController(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->asStackController();
    return result;
}
// asTabBarController() const
void *c_KDDockWidgets__Core__View__asTabBarController(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->asTabBarController();
    return result;
}
// asTitleBarController() const
void *c_KDDockWidgets__Core__View__asTitleBarController(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->asTitleBarController();
    return result;
}
// close()
bool c_KDDockWidgets__Core__View__close(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->close_nocallback();} else {    return targetPtr->close();} }();
    return result;
}
// controller() const
void *c_KDDockWidgets__Core__View__controller(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->controller();
    return result;
}
// createPlatformWindow()
void c_KDDockWidgets__Core__View__createPlatformWindow(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->createPlatformWindow_nocallback();} else {    return targetPtr->createPlatformWindow();} }();
}
// deliverViewEventToFilters(KDDockWidgets::Event * e)
bool c_KDDockWidgets__Core__View__deliverViewEventToFilters_Event(void *thisObj, void *e_)
{
    auto e = reinterpret_cast<KDDockWidgets::Event *>(e_);
    const auto &result = fromPtr(thisObj)->deliverViewEventToFilters(e);
    return result;
}
// dumpDebug()
void c_KDDockWidgets__Core__View__dumpDebug(void *thisObj)
{
    fromPtr(thisObj)->dumpDebug();
}
// equals(const KDDockWidgets::Core::View * one, const KDDockWidgets::Core::View * two)
bool c_static_KDDockWidgets__Core__View__equals_View_View(void *one_, void *two_)
{
    auto one = reinterpret_cast<KDDockWidgets::Core::View *>(one_);
    auto two = reinterpret_cast<KDDockWidgets::Core::View *>(two_);
    const auto &result = KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::equals(one, two);
    return result;
}
// equals(const KDDockWidgets::Core::View * other) const
bool c_KDDockWidgets__Core__View__equals_View(void *thisObj, void *other_)
{
    auto other = reinterpret_cast<KDDockWidgets::Core::View *>(other_);
    const auto &result = fromPtr(thisObj)->equals(other);
    return result;
}
// firstParentOfType(KDDockWidgets::Core::View * view, KDDockWidgets::Core::ViewType arg__2)
void *c_static_KDDockWidgets__Core__View__firstParentOfType_View_ViewType(void *view_, int arg__2)
{
    auto view = reinterpret_cast<KDDockWidgets::Core::View *>(view_);
    const auto &result = KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::firstParentOfType(view, static_cast<KDDockWidgets::Core::ViewType>(arg__2));
    return result;
}
// flags() const
int c_KDDockWidgets__Core__View__flags(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->flags_nocallback();} else {    return targetPtr->flags();} }();
    return result;
}
// geometry() const
void *c_KDDockWidgets__Core__View__geometry(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Rect> { [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->geometry_nocallback();} else {    return targetPtr->geometry();} }() };
    return result;
}
// grabMouse()
void c_KDDockWidgets__Core__View__grabMouse(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->grabMouse_nocallback();} else {    return targetPtr->grabMouse();} }();
}
// hardcodedMinimumSize()
void *c_static_KDDockWidgets__Core__View__hardcodedMinimumSize()
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Size> { KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::hardcodedMinimumSize() };
    return result;
}
// hasFocus() const
bool c_KDDockWidgets__Core__View__hasFocus(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->hasFocus_nocallback();} else {    return targetPtr->hasFocus();} }();
    return result;
}
// height() const
int c_KDDockWidgets__Core__View__height(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->height();
    return result;
}
// hide()
void c_KDDockWidgets__Core__View__hide(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->hide_nocallback();} else {    return targetPtr->hide();} }();
}
// inDtor() const
bool c_KDDockWidgets__Core__View__inDtor(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->inDtor();
    return result;
}
// init()
void c_KDDockWidgets__Core__View__init(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->init_nocallback();} else {    return targetPtr->init();} }();
}
// isActiveWindow() const
bool c_KDDockWidgets__Core__View__isActiveWindow(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->isActiveWindow_nocallback();} else {    return targetPtr->isActiveWindow();} }();
    return result;
}
// isExplicitlyHidden() const
bool c_KDDockWidgets__Core__View__isExplicitlyHidden(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->isExplicitlyHidden_nocallback();} else {    return targetPtr->isExplicitlyHidden();} }();
    return result;
}
// isFixedHeight() const
bool c_KDDockWidgets__Core__View__isFixedHeight(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->isFixedHeight();
    return result;
}
// isFixedWidth() const
bool c_KDDockWidgets__Core__View__isFixedWidth(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->isFixedWidth();
    return result;
}
// isMaximized() const
bool c_KDDockWidgets__Core__View__isMaximized(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->isMaximized_nocallback();} else {    return targetPtr->isMaximized();} }();
    return result;
}
// isMinimized() const
bool c_KDDockWidgets__Core__View__isMinimized(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->isMinimized_nocallback();} else {    return targetPtr->isMinimized();} }();
    return result;
}
// isNull() const
bool c_KDDockWidgets__Core__View__isNull(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->isNull_nocallback();} else {    return targetPtr->isNull();} }();
    return result;
}
// isRootView() const
bool c_KDDockWidgets__Core__View__isRootView(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->isRootView_nocallback();} else {    return targetPtr->isRootView();} }();
    return result;
}
// isVisible() const
bool c_KDDockWidgets__Core__View__isVisible(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->isVisible_nocallback();} else {    return targetPtr->isVisible();} }();
    return result;
}
// mapFromGlobal(KDDockWidgets::Point arg__1) const
void *c_KDDockWidgets__Core__View__mapFromGlobal_Point(void *thisObj, void *arg__1_)
{
    assert(arg__1_);
    auto &arg__1 = *reinterpret_cast<KDDockWidgets::Point *>(arg__1_);
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Point> { [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->mapFromGlobal_nocallback(arg__1);} else {    return targetPtr->mapFromGlobal(arg__1);} }() };
    return result;
}
// mapTo(KDDockWidgets::Core::View * arg__1, KDDockWidgets::Point arg__2) const
void *c_KDDockWidgets__Core__View__mapTo_View_Point(void *thisObj, void *arg__1_, void *arg__2_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::View *>(arg__1_);
    assert(arg__2_);
    auto &arg__2 = *reinterpret_cast<KDDockWidgets::Point *>(arg__2_);
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Point> { [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->mapTo_nocallback(arg__1,arg__2);} else {    return targetPtr->mapTo(arg__1,arg__2);} }() };
    return result;
}
// mapToGlobal(KDDockWidgets::Point arg__1) const
void *c_KDDockWidgets__Core__View__mapToGlobal_Point(void *thisObj, void *arg__1_)
{
    assert(arg__1_);
    auto &arg__1 = *reinterpret_cast<KDDockWidgets::Point *>(arg__1_);
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Point> { [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->mapToGlobal_nocallback(arg__1);} else {    return targetPtr->mapToGlobal(arg__1);} }() };
    return result;
}
// maxSizeHint() const
void *c_KDDockWidgets__Core__View__maxSizeHint(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Size> { [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->maxSizeHint_nocallback();} else {    return targetPtr->maxSizeHint();} }() };
    return result;
}
// minSize() const
void *c_KDDockWidgets__Core__View__minSize(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Size> { [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->minSize_nocallback();} else {    return targetPtr->minSize();} }() };
    return result;
}
// minimumHeight() const
int c_KDDockWidgets__Core__View__minimumHeight(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->minimumHeight();
    return result;
}
// minimumWidth() const
int c_KDDockWidgets__Core__View__minimumWidth(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->minimumWidth();
    return result;
}
// move(KDDockWidgets::Point arg__1)
void c_KDDockWidgets__Core__View__move_Point(void *thisObj, void *arg__1_)
{
    assert(arg__1_);
    auto &arg__1 = *reinterpret_cast<KDDockWidgets::Point *>(arg__1_);
    fromPtr(thisObj)->move(arg__1);
}
// move(int x, int y)
void c_KDDockWidgets__Core__View__move_int_int(void *thisObj, int x, int y)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->move_nocallback(x,y);} else {    return targetPtr->move(x,y);} }();
}
// normalGeometry() const
void *c_KDDockWidgets__Core__View__normalGeometry(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Rect> { [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->normalGeometry_nocallback();} else {    return targetPtr->normalGeometry();} }() };
    return result;
}
// onResize(KDDockWidgets::Size arg__1)
bool c_KDDockWidgets__Core__View__onResize_Size(void *thisObj, void *arg__1_)
{
    assert(arg__1_);
    auto &arg__1 = *reinterpret_cast<KDDockWidgets::Size *>(arg__1_);
    const auto &result = fromPtr(thisObj)->onResize(arg__1);
    return result;
}
// onResize(int h, int w)
bool c_KDDockWidgets__Core__View__onResize_int_int(void *thisObj, int h, int w)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->onResize_nocallback(h,w);} else {    return targetPtr->onResize(h,w);} }();
    return result;
}
// pos() const
void *c_KDDockWidgets__Core__View__pos(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Point> { fromPtr(thisObj)->pos() };
    return result;
}
// raise()
void c_KDDockWidgets__Core__View__raise(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->raise_nocallback();} else {    return targetPtr->raise();} }();
}
// raiseAndActivate()
void c_KDDockWidgets__Core__View__raiseAndActivate(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->raiseAndActivate_nocallback();} else {    return targetPtr->raiseAndActivate();} }();
}
// rect() const
void *c_KDDockWidgets__Core__View__rect(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Rect> { fromPtr(thisObj)->rect() };
    return result;
}
// releaseKeyboard()
void c_KDDockWidgets__Core__View__releaseKeyboard(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->releaseKeyboard_nocallback();} else {    return targetPtr->releaseKeyboard();} }();
}
// releaseMouse()
void c_KDDockWidgets__Core__View__releaseMouse(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->releaseMouse_nocallback();} else {    return targetPtr->releaseMouse();} }();
}
// resize(KDDockWidgets::Size arg__1)
void c_KDDockWidgets__Core__View__resize_Size(void *thisObj, void *arg__1_)
{
    assert(arg__1_);
    auto &arg__1 = *reinterpret_cast<KDDockWidgets::Size *>(arg__1_);
    fromPtr(thisObj)->resize(arg__1);
}
// resize(int w, int h)
void c_KDDockWidgets__Core__View__resize_int_int(void *thisObj, int w, int h)
{
    fromPtr(thisObj)->resize(w, h);
}
// screenSize() const
void *c_KDDockWidgets__Core__View__screenSize(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Size> { fromPtr(thisObj)->screenSize() };
    return result;
}
// setCursor(Qt::CursorShape arg__1)
void c_KDDockWidgets__Core__View__setCursor_CursorShape(void *thisObj, int arg__1)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setCursor_nocallback(static_cast<Qt::CursorShape>(arg__1));} else {    return targetPtr->setCursor(static_cast<Qt::CursorShape>(arg__1));} }();
}
// setFixedHeight(int arg__1)
void c_KDDockWidgets__Core__View__setFixedHeight_int(void *thisObj, int arg__1)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setFixedHeight_nocallback(arg__1);} else {    return targetPtr->setFixedHeight(arg__1);} }();
}
// setFixedWidth(int arg__1)
void c_KDDockWidgets__Core__View__setFixedWidth_int(void *thisObj, int arg__1)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setFixedWidth_nocallback(arg__1);} else {    return targetPtr->setFixedWidth(arg__1);} }();
}
// setGeometry(KDDockWidgets::Rect arg__1)
void c_KDDockWidgets__Core__View__setGeometry_Rect(void *thisObj, void *arg__1_)
{
    assert(arg__1_);
    auto &arg__1 = *reinterpret_cast<KDDockWidgets::Rect *>(arg__1_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setGeometry_nocallback(arg__1);} else {    return targetPtr->setGeometry(arg__1);} }();
}
// setHeight(int height)
void c_KDDockWidgets__Core__View__setHeight_int(void *thisObj, int height)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setHeight_nocallback(height);} else {    return targetPtr->setHeight(height);} }();
}
// setMaximumSize(KDDockWidgets::Size sz)
void c_KDDockWidgets__Core__View__setMaximumSize_Size(void *thisObj, void *sz_)
{
    assert(sz_);
    auto &sz = *reinterpret_cast<KDDockWidgets::Size *>(sz_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setMaximumSize_nocallback(sz);} else {    return targetPtr->setMaximumSize(sz);} }();
}
// setMinimumSize(KDDockWidgets::Size arg__1)
void c_KDDockWidgets__Core__View__setMinimumSize_Size(void *thisObj, void *arg__1_)
{
    assert(arg__1_);
    auto &arg__1 = *reinterpret_cast<KDDockWidgets::Size *>(arg__1_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setMinimumSize_nocallback(arg__1);} else {    return targetPtr->setMinimumSize(arg__1);} }();
}
// setMouseTracking(bool arg__1)
void c_KDDockWidgets__Core__View__setMouseTracking_bool(void *thisObj, bool arg__1)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setMouseTracking_nocallback(arg__1);} else {    return targetPtr->setMouseTracking(arg__1);} }();
}
// setParent(KDDockWidgets::Core::View * arg__1)
void c_KDDockWidgets__Core__View__setParent_View(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::View *>(arg__1_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setParent_nocallback(arg__1);} else {    return targetPtr->setParent(arg__1);} }();
}
// setSize(KDDockWidgets::Size arg__1)
void c_KDDockWidgets__Core__View__setSize_Size(void *thisObj, void *arg__1_)
{
    assert(arg__1_);
    auto &arg__1 = *reinterpret_cast<KDDockWidgets::Size *>(arg__1_);
    fromPtr(thisObj)->setSize(arg__1);
}
// setSize(int width, int height)
void c_KDDockWidgets__Core__View__setSize_int_int(void *thisObj, int width, int height)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setSize_nocallback(width,height);} else {    return targetPtr->setSize(width,height);} }();
}
// setViewName(const QString & arg__1)
void c_KDDockWidgets__Core__View__setViewName_QString(void *thisObj, const char *arg__1_)
{
    const auto arg__1 = QString::fromUtf8(arg__1_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setViewName_nocallback(arg__1);} else {    return targetPtr->setViewName(arg__1);} }();
    free(( char * )arg__1_);
}
// setVisible(bool arg__1)
void c_KDDockWidgets__Core__View__setVisible_bool(void *thisObj, bool arg__1)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setVisible_nocallback(arg__1);} else {    return targetPtr->setVisible(arg__1);} }();
}
// setWidth(int width)
void c_KDDockWidgets__Core__View__setWidth_int(void *thisObj, int width)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setWidth_nocallback(width);} else {    return targetPtr->setWidth(width);} }();
}
// setWindowOpacity(double arg__1)
void c_KDDockWidgets__Core__View__setWindowOpacity_double(void *thisObj, double arg__1)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setWindowOpacity_nocallback(arg__1);} else {    return targetPtr->setWindowOpacity(arg__1);} }();
}
// setWindowTitle(const QString & title)
void c_KDDockWidgets__Core__View__setWindowTitle_QString(void *thisObj, const char *title_)
{
    const auto title = QString::fromUtf8(title_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setWindowTitle_nocallback(title);} else {    return targetPtr->setWindowTitle(title);} }();
    free(( char * )title_);
}
// setZOrder(int arg__1)
void c_KDDockWidgets__Core__View__setZOrder_int(void *thisObj, int arg__1)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setZOrder_nocallback(arg__1);} else {    return targetPtr->setZOrder(arg__1);} }();
}
// show()
void c_KDDockWidgets__Core__View__show(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->show_nocallback();} else {    return targetPtr->show();} }();
}
// showMaximized()
void c_KDDockWidgets__Core__View__showMaximized(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->showMaximized_nocallback();} else {    return targetPtr->showMaximized();} }();
}
// showMinimized()
void c_KDDockWidgets__Core__View__showMinimized(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->showMinimized_nocallback();} else {    return targetPtr->showMinimized();} }();
}
// showNormal()
void c_KDDockWidgets__Core__View__showNormal(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->showNormal_nocallback();} else {    return targetPtr->showNormal();} }();
}
// size() const
void *c_KDDockWidgets__Core__View__size(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Size> { fromPtr(thisObj)->size() };
    return result;
}
// update()
void c_KDDockWidgets__Core__View__update(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->update_nocallback();} else {    return targetPtr->update();} }();
}
// viewName() const
void *c_KDDockWidgets__Core__View__viewName(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<QString> { [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->viewName_nocallback();} else {    return targetPtr->viewName();} }() };
    return result;
}
// width() const
int c_KDDockWidgets__Core__View__width(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->width();
    return result;
}
// x() const
int c_KDDockWidgets__Core__View__x(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->x();
    return result;
}
// y() const
int c_KDDockWidgets__Core__View__y(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->y();
    return result;
}
// zOrder() const
int c_KDDockWidgets__Core__View__zOrder(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->zOrder_nocallback();} else {    return targetPtr->zOrder();} }();
    return result;
}
void c_KDDockWidgets__Core__View__destructor(void *thisObj)
{
    delete fromPtr(thisObj);
}
void c_KDDockWidgets__Core__View__registerVirtualMethodCallback(void *ptr, void *callback, int methodId)
{
    auto wrapper = fromWrapperPtr(ptr);
    switch (methodId) {
    case 342:
        wrapper->m_activateWindowCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_activateWindow>(callback);
        break;
    case 352:
        wrapper->m_closeCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_close>(callback);
        break;
    case 354:
        wrapper->m_createPlatformWindowCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_createPlatformWindow>(callback);
        break;
    case 360:
        wrapper->m_flagsCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_flags>(callback);
        break;
    case 361:
        wrapper->m_geometryCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_geometry>(callback);
        break;
    case 362:
        wrapper->m_grabMouseCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_grabMouse>(callback);
        break;
    case 365:
        wrapper->m_hasFocusCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_hasFocus>(callback);
        break;
    case 367:
        wrapper->m_hideCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_hide>(callback);
        break;
    case 369:
        wrapper->m_initCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_init>(callback);
        break;
    case 371:
        wrapper->m_isActiveWindowCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_isActiveWindow>(callback);
        break;
    case 372:
        wrapper->m_isExplicitlyHiddenCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_isExplicitlyHidden>(callback);
        break;
    case 375:
        wrapper->m_isMaximizedCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_isMaximized>(callback);
        break;
    case 376:
        wrapper->m_isMinimizedCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_isMinimized>(callback);
        break;
    case 377:
        wrapper->m_isNullCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_isNull>(callback);
        break;
    case 378:
        wrapper->m_isRootViewCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_isRootView>(callback);
        break;
    case 379:
        wrapper->m_isVisibleCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_isVisible>(callback);
        break;
    case 380:
        wrapper->m_mapFromGlobalCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_mapFromGlobal>(callback);
        break;
    case 381:
        wrapper->m_mapToCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_mapTo>(callback);
        break;
    case 382:
        wrapper->m_mapToGlobalCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_mapToGlobal>(callback);
        break;
    case 383:
        wrapper->m_maxSizeHintCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_maxSizeHint>(callback);
        break;
    case 384:
        wrapper->m_minSizeCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_minSize>(callback);
        break;
    case 388:
        wrapper->m_move_2Callback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_move_2>(callback);
        break;
    case 389:
        wrapper->m_normalGeometryCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_normalGeometry>(callback);
        break;
    case 391:
        wrapper->m_onResize_2Callback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_onResize_2>(callback);
        break;
    case 393:
        wrapper->m_raiseCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_raise>(callback);
        break;
    case 394:
        wrapper->m_raiseAndActivateCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_raiseAndActivate>(callback);
        break;
    case 396:
        wrapper->m_releaseKeyboardCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_releaseKeyboard>(callback);
        break;
    case 397:
        wrapper->m_releaseMouseCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_releaseMouse>(callback);
        break;
    case 401:
        wrapper->m_setCursorCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_setCursor>(callback);
        break;
    case 402:
        wrapper->m_setFixedHeightCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_setFixedHeight>(callback);
        break;
    case 403:
        wrapper->m_setFixedWidthCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_setFixedWidth>(callback);
        break;
    case 404:
        wrapper->m_setGeometryCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_setGeometry>(callback);
        break;
    case 405:
        wrapper->m_setHeightCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_setHeight>(callback);
        break;
    case 406:
        wrapper->m_setMaximumSizeCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_setMaximumSize>(callback);
        break;
    case 407:
        wrapper->m_setMinimumSizeCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_setMinimumSize>(callback);
        break;
    case 408:
        wrapper->m_setMouseTrackingCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_setMouseTracking>(callback);
        break;
    case 409:
        wrapper->m_setParentCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_setParent>(callback);
        break;
    case 411:
        wrapper->m_setSize_2Callback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_setSize_2>(callback);
        break;
    case 412:
        wrapper->m_setViewNameCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_setViewName>(callback);
        break;
    case 413:
        wrapper->m_setVisibleCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_setVisible>(callback);
        break;
    case 414:
        wrapper->m_setWidthCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_setWidth>(callback);
        break;
    case 415:
        wrapper->m_setWindowOpacityCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_setWindowOpacity>(callback);
        break;
    case 416:
        wrapper->m_setWindowTitleCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_setWindowTitle>(callback);
        break;
    case 417:
        wrapper->m_setZOrderCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_setZOrder>(callback);
        break;
    case 418:
        wrapper->m_showCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_show>(callback);
        break;
    case 419:
        wrapper->m_showMaximizedCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_showMaximized>(callback);
        break;
    case 420:
        wrapper->m_showMinimizedCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_showMinimized>(callback);
        break;
    case 421:
        wrapper->m_showNormalCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_showNormal>(callback);
        break;
    case 423:
        wrapper->m_updateCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_update>(callback);
        break;
    case 424:
        wrapper->m_viewNameCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_viewName>(callback);
        break;
    case 428:
        wrapper->m_zOrderCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::View_wrapper::Callback_zOrder>(callback);
        break;
    }
}
}
