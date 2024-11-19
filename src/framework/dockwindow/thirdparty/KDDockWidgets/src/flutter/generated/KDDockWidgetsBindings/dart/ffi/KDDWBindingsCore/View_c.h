/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <core/View.h>
#include <core/Controller.h>
#include <geometry_helpers_p.h>
#include <string_p.h>
#include <QtCompat_p.h>
#include <FloatingWindow.h>
#include <core/Group.h>
#include "core/TitleBar.h"
#include <TabBar.h>
#include "core/Stack.h"
#include <core/DockWidget.h>
#include "core/MainWindow.h"
#include "core/DropArea.h"
#include <core/Layout.h>

namespace KDDockWidgetsBindings_wrappersNS {
namespace KDDWBindingsCore {
class View_wrapper : public ::KDDockWidgets::Core::View
{
public:
    ~View_wrapper();
    virtual void activateWindow();
    virtual void activateWindow_nocallback();
    KDDockWidgets::Core::DockWidget *asDockWidgetController() const;
    KDDockWidgets::Core::DropArea *asDropAreaController() const;
    KDDockWidgets::Core::FloatingWindow *asFloatingWindowController() const;
    KDDockWidgets::Core::Group *asGroupController() const;
    KDDockWidgets::Core::Layout *asLayout() const;
    KDDockWidgets::Core::MainWindow *asMainWindowController() const;
    KDDockWidgets::Core::Stack *asStackController() const;
    KDDockWidgets::Core::TabBar *asTabBarController() const;
    KDDockWidgets::Core::TitleBar *asTitleBarController() const;
    virtual bool close();
    virtual bool close_nocallback();
    KDDockWidgets::Core::Controller *controller() const;
    virtual void createPlatformWindow();
    virtual void createPlatformWindow_nocallback();
    bool deliverViewEventToFilters(KDDockWidgets::Event *e);
    void dumpDebug();
    static bool equals(const KDDockWidgets::Core::View *one, const KDDockWidgets::Core::View *two);
    bool equals(const KDDockWidgets::Core::View *other) const;
    static KDDockWidgets::Core::Controller *firstParentOfType(KDDockWidgets::Core::View *view, KDDockWidgets::Core::ViewType arg__2);
    virtual Qt::WindowFlags flags() const;
    virtual Qt::WindowFlags flags_nocallback() const;
    virtual KDDockWidgets::Rect geometry() const;
    virtual KDDockWidgets::Rect geometry_nocallback() const;
    virtual void grabMouse();
    virtual void grabMouse_nocallback();
    static KDDockWidgets::Size hardcodedMinimumSize();
    virtual bool hasFocus() const;
    virtual bool hasFocus_nocallback() const;
    int height() const;
    virtual void hide();
    virtual void hide_nocallback();
    bool inDtor() const;
    virtual void init();
    virtual void init_nocallback();
    virtual bool isActiveWindow() const;
    virtual bool isActiveWindow_nocallback() const;
    virtual bool isExplicitlyHidden() const;
    virtual bool isExplicitlyHidden_nocallback() const;
    bool isFixedHeight() const;
    bool isFixedWidth() const;
    virtual bool isMaximized() const;
    virtual bool isMaximized_nocallback() const;
    virtual bool isMinimized() const;
    virtual bool isMinimized_nocallback() const;
    virtual bool isNull() const;
    virtual bool isNull_nocallback() const;
    virtual bool isRootView() const;
    virtual bool isRootView_nocallback() const;
    virtual bool isVisible() const;
    virtual bool isVisible_nocallback() const;
    virtual KDDockWidgets::Point mapFromGlobal(KDDockWidgets::Point arg__1) const;
    virtual KDDockWidgets::Point mapFromGlobal_nocallback(KDDockWidgets::Point arg__1) const;
    virtual KDDockWidgets::Point mapTo(KDDockWidgets::Core::View *arg__1, KDDockWidgets::Point arg__2) const;
    virtual KDDockWidgets::Point mapTo_nocallback(KDDockWidgets::Core::View *arg__1, KDDockWidgets::Point arg__2) const;
    virtual KDDockWidgets::Point mapToGlobal(KDDockWidgets::Point arg__1) const;
    virtual KDDockWidgets::Point mapToGlobal_nocallback(KDDockWidgets::Point arg__1) const;
    virtual KDDockWidgets::Size maxSizeHint() const;
    virtual KDDockWidgets::Size maxSizeHint_nocallback() const;
    virtual KDDockWidgets::Size minSize() const;
    virtual KDDockWidgets::Size minSize_nocallback() const;
    int minimumHeight() const;
    int minimumWidth() const;
    void move(KDDockWidgets::Point arg__1);
    virtual void move(int x, int y);
    virtual void move_nocallback(int x, int y);
    virtual KDDockWidgets::Rect normalGeometry() const;
    virtual KDDockWidgets::Rect normalGeometry_nocallback() const;
    bool onResize(KDDockWidgets::Size arg__1);
    virtual bool onResize(int h, int w);
    virtual bool onResize_nocallback(int h, int w);
    KDDockWidgets::Point pos() const;
    virtual void raise();
    virtual void raise_nocallback();
    virtual void raiseAndActivate();
    virtual void raiseAndActivate_nocallback();
    KDDockWidgets::Rect rect() const;
    virtual void releaseKeyboard();
    virtual void releaseKeyboard_nocallback();
    virtual void releaseMouse();
    virtual void releaseMouse_nocallback();
    void resize(KDDockWidgets::Size arg__1);
    void resize(int w, int h);
    KDDockWidgets::Size screenSize() const;
    virtual void setCursor(Qt::CursorShape arg__1);
    virtual void setCursor_nocallback(Qt::CursorShape arg__1);
    virtual void setFixedHeight(int arg__1);
    virtual void setFixedHeight_nocallback(int arg__1);
    virtual void setFixedWidth(int arg__1);
    virtual void setFixedWidth_nocallback(int arg__1);
    virtual void setGeometry(KDDockWidgets::Rect arg__1);
    virtual void setGeometry_nocallback(KDDockWidgets::Rect arg__1);
    virtual void setHeight(int height);
    virtual void setHeight_nocallback(int height);
    virtual void setMaximumSize(KDDockWidgets::Size sz);
    virtual void setMaximumSize_nocallback(KDDockWidgets::Size sz);
    virtual void setMinimumSize(KDDockWidgets::Size arg__1);
    virtual void setMinimumSize_nocallback(KDDockWidgets::Size arg__1);
    virtual void setMouseTracking(bool arg__1);
    virtual void setMouseTracking_nocallback(bool arg__1);
    virtual void setParent(KDDockWidgets::Core::View *arg__1);
    virtual void setParent_nocallback(KDDockWidgets::Core::View *arg__1);
    void setSize(KDDockWidgets::Size arg__1);
    virtual void setSize(int width, int height);
    virtual void setSize_nocallback(int width, int height);
    virtual void setViewName(const QString &arg__1);
    virtual void setViewName_nocallback(const QString &arg__1);
    virtual void setVisible(bool arg__1);
    virtual void setVisible_nocallback(bool arg__1);
    virtual void setWidth(int width);
    virtual void setWidth_nocallback(int width);
    virtual void setWindowOpacity(double arg__1);
    virtual void setWindowOpacity_nocallback(double arg__1);
    virtual void setWindowTitle(const QString &title);
    virtual void setWindowTitle_nocallback(const QString &title);
    virtual void setZOrder(int arg__1);
    virtual void setZOrder_nocallback(int arg__1);
    virtual void show();
    virtual void show_nocallback();
    virtual void showMaximized();
    virtual void showMaximized_nocallback();
    virtual void showMinimized();
    virtual void showMinimized_nocallback();
    virtual void showNormal();
    virtual void showNormal_nocallback();
    KDDockWidgets::Size size() const;
    virtual void update();
    virtual void update_nocallback();
    virtual QString viewName() const;
    virtual QString viewName_nocallback() const;
    int width() const;
    int x() const;
    int y() const;
    virtual int zOrder() const;
    virtual int zOrder_nocallback() const;
    typedef void (*Callback_activateWindow)(void *);
    Callback_activateWindow m_activateWindowCallback = nullptr;
    typedef bool (*Callback_close)(void *);
    Callback_close m_closeCallback = nullptr;
    typedef void (*Callback_createPlatformWindow)(void *);
    Callback_createPlatformWindow m_createPlatformWindowCallback = nullptr;
    typedef Qt::WindowFlags (*Callback_flags)(void *);
    Callback_flags m_flagsCallback = nullptr;
    typedef KDDockWidgets::Rect *(*Callback_geometry)(void *);
    Callback_geometry m_geometryCallback = nullptr;
    typedef void (*Callback_grabMouse)(void *);
    Callback_grabMouse m_grabMouseCallback = nullptr;
    typedef bool (*Callback_hasFocus)(void *);
    Callback_hasFocus m_hasFocusCallback = nullptr;
    typedef void (*Callback_hide)(void *);
    Callback_hide m_hideCallback = nullptr;
    typedef void (*Callback_init)(void *);
    Callback_init m_initCallback = nullptr;
    typedef bool (*Callback_isActiveWindow)(void *);
    Callback_isActiveWindow m_isActiveWindowCallback = nullptr;
    typedef bool (*Callback_isExplicitlyHidden)(void *);
    Callback_isExplicitlyHidden m_isExplicitlyHiddenCallback = nullptr;
    typedef bool (*Callback_isMaximized)(void *);
    Callback_isMaximized m_isMaximizedCallback = nullptr;
    typedef bool (*Callback_isMinimized)(void *);
    Callback_isMinimized m_isMinimizedCallback = nullptr;
    typedef bool (*Callback_isNull)(void *);
    Callback_isNull m_isNullCallback = nullptr;
    typedef bool (*Callback_isRootView)(void *);
    Callback_isRootView m_isRootViewCallback = nullptr;
    typedef bool (*Callback_isVisible)(void *);
    Callback_isVisible m_isVisibleCallback = nullptr;
    typedef KDDockWidgets::Point *(*Callback_mapFromGlobal)(void *, KDDockWidgets::Point *arg__1);
    Callback_mapFromGlobal m_mapFromGlobalCallback = nullptr;
    typedef KDDockWidgets::Point *(*Callback_mapTo)(void *, KDDockWidgets::Core::View *arg__1, KDDockWidgets::Point *arg__2);
    Callback_mapTo m_mapToCallback = nullptr;
    typedef KDDockWidgets::Point *(*Callback_mapToGlobal)(void *, KDDockWidgets::Point *arg__1);
    Callback_mapToGlobal m_mapToGlobalCallback = nullptr;
    typedef KDDockWidgets::Size *(*Callback_maxSizeHint)(void *);
    Callback_maxSizeHint m_maxSizeHintCallback = nullptr;
    typedef KDDockWidgets::Size *(*Callback_minSize)(void *);
    Callback_minSize m_minSizeCallback = nullptr;
    typedef void (*Callback_move_2)(void *, int x, int y);
    Callback_move_2 m_move_2Callback = nullptr;
    typedef KDDockWidgets::Rect *(*Callback_normalGeometry)(void *);
    Callback_normalGeometry m_normalGeometryCallback = nullptr;
    typedef bool (*Callback_onResize_2)(void *, int h, int w);
    Callback_onResize_2 m_onResize_2Callback = nullptr;
    typedef void (*Callback_raise)(void *);
    Callback_raise m_raiseCallback = nullptr;
    typedef void (*Callback_raiseAndActivate)(void *);
    Callback_raiseAndActivate m_raiseAndActivateCallback = nullptr;
    typedef void (*Callback_releaseKeyboard)(void *);
    Callback_releaseKeyboard m_releaseKeyboardCallback = nullptr;
    typedef void (*Callback_releaseMouse)(void *);
    Callback_releaseMouse m_releaseMouseCallback = nullptr;
    typedef void (*Callback_setCursor)(void *, Qt::CursorShape arg__1);
    Callback_setCursor m_setCursorCallback = nullptr;
    typedef void (*Callback_setFixedHeight)(void *, int arg__1);
    Callback_setFixedHeight m_setFixedHeightCallback = nullptr;
    typedef void (*Callback_setFixedWidth)(void *, int arg__1);
    Callback_setFixedWidth m_setFixedWidthCallback = nullptr;
    typedef void (*Callback_setGeometry)(void *, KDDockWidgets::Rect *arg__1);
    Callback_setGeometry m_setGeometryCallback = nullptr;
    typedef void (*Callback_setHeight)(void *, int height);
    Callback_setHeight m_setHeightCallback = nullptr;
    typedef void (*Callback_setMaximumSize)(void *, KDDockWidgets::Size *sz);
    Callback_setMaximumSize m_setMaximumSizeCallback = nullptr;
    typedef void (*Callback_setMinimumSize)(void *, KDDockWidgets::Size *arg__1);
    Callback_setMinimumSize m_setMinimumSizeCallback = nullptr;
    typedef void (*Callback_setMouseTracking)(void *, bool arg__1);
    Callback_setMouseTracking m_setMouseTrackingCallback = nullptr;
    typedef void (*Callback_setParent)(void *, KDDockWidgets::Core::View *arg__1);
    Callback_setParent m_setParentCallback = nullptr;
    typedef void (*Callback_setSize_2)(void *, int width, int height);
    Callback_setSize_2 m_setSize_2Callback = nullptr;
    typedef void (*Callback_setViewName)(void *, const QString &arg__1);
    Callback_setViewName m_setViewNameCallback = nullptr;
    typedef void (*Callback_setVisible)(void *, bool arg__1);
    Callback_setVisible m_setVisibleCallback = nullptr;
    typedef void (*Callback_setWidth)(void *, int width);
    Callback_setWidth m_setWidthCallback = nullptr;
    typedef void (*Callback_setWindowOpacity)(void *, double arg__1);
    Callback_setWindowOpacity m_setWindowOpacityCallback = nullptr;
    typedef void (*Callback_setWindowTitle)(void *, const QString &title);
    Callback_setWindowTitle m_setWindowTitleCallback = nullptr;
    typedef void (*Callback_setZOrder)(void *, int arg__1);
    Callback_setZOrder m_setZOrderCallback = nullptr;
    typedef void (*Callback_show)(void *);
    Callback_show m_showCallback = nullptr;
    typedef void (*Callback_showMaximized)(void *);
    Callback_showMaximized m_showMaximizedCallback = nullptr;
    typedef void (*Callback_showMinimized)(void *);
    Callback_showMinimized m_showMinimizedCallback = nullptr;
    typedef void (*Callback_showNormal)(void *);
    Callback_showNormal m_showNormalCallback = nullptr;
    typedef void (*Callback_update)(void *);
    Callback_update m_updateCallback = nullptr;
    typedef QString *(*Callback_viewName)(void *);
    Callback_viewName m_viewNameCallback = nullptr;
    typedef int (*Callback_zOrder)(void *);
    Callback_zOrder m_zOrderCallback = nullptr;
};
}
}
extern "C" {
// KDDockWidgets::Core::View::activateWindow()
DOCKS_EXPORT void c_KDDockWidgets__Core__View__activateWindow(void *thisObj);
// KDDockWidgets::Core::View::asDockWidgetController() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__View__asDockWidgetController(void *thisObj);
// KDDockWidgets::Core::View::asDropAreaController() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__View__asDropAreaController(void *thisObj);
// KDDockWidgets::Core::View::asFloatingWindowController() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__View__asFloatingWindowController(void *thisObj);
// KDDockWidgets::Core::View::asGroupController() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__View__asGroupController(void *thisObj);
// KDDockWidgets::Core::View::asLayout() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__View__asLayout(void *thisObj);
// KDDockWidgets::Core::View::asMainWindowController() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__View__asMainWindowController(void *thisObj);
// KDDockWidgets::Core::View::asStackController() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__View__asStackController(void *thisObj);
// KDDockWidgets::Core::View::asTabBarController() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__View__asTabBarController(void *thisObj);
// KDDockWidgets::Core::View::asTitleBarController() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__View__asTitleBarController(void *thisObj);
// KDDockWidgets::Core::View::close()
DOCKS_EXPORT bool c_KDDockWidgets__Core__View__close(void *thisObj);
// KDDockWidgets::Core::View::controller() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__View__controller(void *thisObj);
// KDDockWidgets::Core::View::createPlatformWindow()
DOCKS_EXPORT void c_KDDockWidgets__Core__View__createPlatformWindow(void *thisObj);
// KDDockWidgets::Core::View::deliverViewEventToFilters(KDDockWidgets::Event * e)
DOCKS_EXPORT bool c_KDDockWidgets__Core__View__deliverViewEventToFilters_Event(void *thisObj, void *e_);
// KDDockWidgets::Core::View::dumpDebug()
DOCKS_EXPORT void c_KDDockWidgets__Core__View__dumpDebug(void *thisObj);
// KDDockWidgets::Core::View::equals(const KDDockWidgets::Core::View * one, const KDDockWidgets::Core::View * two)
DOCKS_EXPORT bool c_static_KDDockWidgets__Core__View__equals_View_View(void *one_, void *two_);
// KDDockWidgets::Core::View::equals(const KDDockWidgets::Core::View * other) const
DOCKS_EXPORT bool c_KDDockWidgets__Core__View__equals_View(void *thisObj, void *other_);
// KDDockWidgets::Core::View::firstParentOfType(KDDockWidgets::Core::View * view, KDDockWidgets::Core::ViewType arg__2)
DOCKS_EXPORT void *c_static_KDDockWidgets__Core__View__firstParentOfType_View_ViewType(void *view_, int arg__2);
// KDDockWidgets::Core::View::flags() const
DOCKS_EXPORT int c_KDDockWidgets__Core__View__flags(void *thisObj);
// KDDockWidgets::Core::View::geometry() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__View__geometry(void *thisObj);
// KDDockWidgets::Core::View::grabMouse()
DOCKS_EXPORT void c_KDDockWidgets__Core__View__grabMouse(void *thisObj);
// KDDockWidgets::Core::View::hardcodedMinimumSize()
DOCKS_EXPORT void *c_static_KDDockWidgets__Core__View__hardcodedMinimumSize();
// KDDockWidgets::Core::View::hasFocus() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__View__hasFocus(void *thisObj);
// KDDockWidgets::Core::View::height() const
DOCKS_EXPORT int c_KDDockWidgets__Core__View__height(void *thisObj);
// KDDockWidgets::Core::View::hide()
DOCKS_EXPORT void c_KDDockWidgets__Core__View__hide(void *thisObj);
// KDDockWidgets::Core::View::inDtor() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__View__inDtor(void *thisObj);
// KDDockWidgets::Core::View::init()
DOCKS_EXPORT void c_KDDockWidgets__Core__View__init(void *thisObj);
// KDDockWidgets::Core::View::isActiveWindow() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__View__isActiveWindow(void *thisObj);
// KDDockWidgets::Core::View::isExplicitlyHidden() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__View__isExplicitlyHidden(void *thisObj);
// KDDockWidgets::Core::View::isFixedHeight() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__View__isFixedHeight(void *thisObj);
// KDDockWidgets::Core::View::isFixedWidth() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__View__isFixedWidth(void *thisObj);
// KDDockWidgets::Core::View::isMaximized() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__View__isMaximized(void *thisObj);
// KDDockWidgets::Core::View::isMinimized() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__View__isMinimized(void *thisObj);
// KDDockWidgets::Core::View::isNull() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__View__isNull(void *thisObj);
// KDDockWidgets::Core::View::isRootView() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__View__isRootView(void *thisObj);
// KDDockWidgets::Core::View::isVisible() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__View__isVisible(void *thisObj);
// KDDockWidgets::Core::View::mapFromGlobal(KDDockWidgets::Point arg__1) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__View__mapFromGlobal_Point(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::View::mapTo(KDDockWidgets::Core::View * arg__1, KDDockWidgets::Point arg__2) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__View__mapTo_View_Point(void *thisObj, void *arg__1_, void *arg__2_);
// KDDockWidgets::Core::View::mapToGlobal(KDDockWidgets::Point arg__1) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__View__mapToGlobal_Point(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::View::maxSizeHint() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__View__maxSizeHint(void *thisObj);
// KDDockWidgets::Core::View::minSize() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__View__minSize(void *thisObj);
// KDDockWidgets::Core::View::minimumHeight() const
DOCKS_EXPORT int c_KDDockWidgets__Core__View__minimumHeight(void *thisObj);
// KDDockWidgets::Core::View::minimumWidth() const
DOCKS_EXPORT int c_KDDockWidgets__Core__View__minimumWidth(void *thisObj);
// KDDockWidgets::Core::View::move(KDDockWidgets::Point arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__View__move_Point(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::View::move(int x, int y)
DOCKS_EXPORT void c_KDDockWidgets__Core__View__move_int_int(void *thisObj, int x, int y);
// KDDockWidgets::Core::View::normalGeometry() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__View__normalGeometry(void *thisObj);
// KDDockWidgets::Core::View::onResize(KDDockWidgets::Size arg__1)
DOCKS_EXPORT bool c_KDDockWidgets__Core__View__onResize_Size(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::View::onResize(int h, int w)
DOCKS_EXPORT bool c_KDDockWidgets__Core__View__onResize_int_int(void *thisObj, int h, int w);
// KDDockWidgets::Core::View::pos() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__View__pos(void *thisObj);
// KDDockWidgets::Core::View::raise()
DOCKS_EXPORT void c_KDDockWidgets__Core__View__raise(void *thisObj);
// KDDockWidgets::Core::View::raiseAndActivate()
DOCKS_EXPORT void c_KDDockWidgets__Core__View__raiseAndActivate(void *thisObj);
// KDDockWidgets::Core::View::rect() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__View__rect(void *thisObj);
// KDDockWidgets::Core::View::releaseKeyboard()
DOCKS_EXPORT void c_KDDockWidgets__Core__View__releaseKeyboard(void *thisObj);
// KDDockWidgets::Core::View::releaseMouse()
DOCKS_EXPORT void c_KDDockWidgets__Core__View__releaseMouse(void *thisObj);
// KDDockWidgets::Core::View::resize(KDDockWidgets::Size arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__View__resize_Size(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::View::resize(int w, int h)
DOCKS_EXPORT void c_KDDockWidgets__Core__View__resize_int_int(void *thisObj, int w, int h);
// KDDockWidgets::Core::View::screenSize() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__View__screenSize(void *thisObj);
// KDDockWidgets::Core::View::setCursor(Qt::CursorShape arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__View__setCursor_CursorShape(void *thisObj, int arg__1);
// KDDockWidgets::Core::View::setFixedHeight(int arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__View__setFixedHeight_int(void *thisObj, int arg__1);
// KDDockWidgets::Core::View::setFixedWidth(int arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__View__setFixedWidth_int(void *thisObj, int arg__1);
// KDDockWidgets::Core::View::setGeometry(KDDockWidgets::Rect arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__View__setGeometry_Rect(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::View::setHeight(int height)
DOCKS_EXPORT void c_KDDockWidgets__Core__View__setHeight_int(void *thisObj, int height);
// KDDockWidgets::Core::View::setMaximumSize(KDDockWidgets::Size sz)
DOCKS_EXPORT void c_KDDockWidgets__Core__View__setMaximumSize_Size(void *thisObj, void *sz_);
// KDDockWidgets::Core::View::setMinimumSize(KDDockWidgets::Size arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__View__setMinimumSize_Size(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::View::setMouseTracking(bool arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__View__setMouseTracking_bool(void *thisObj, bool arg__1);
// KDDockWidgets::Core::View::setParent(KDDockWidgets::Core::View * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__View__setParent_View(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::View::setSize(KDDockWidgets::Size arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__View__setSize_Size(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::View::setSize(int width, int height)
DOCKS_EXPORT void c_KDDockWidgets__Core__View__setSize_int_int(void *thisObj, int width, int height);
// KDDockWidgets::Core::View::setViewName(const QString & arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__View__setViewName_QString(void *thisObj, const char *arg__1_);
// KDDockWidgets::Core::View::setVisible(bool arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__View__setVisible_bool(void *thisObj, bool arg__1);
// KDDockWidgets::Core::View::setWidth(int width)
DOCKS_EXPORT void c_KDDockWidgets__Core__View__setWidth_int(void *thisObj, int width);
// KDDockWidgets::Core::View::setWindowOpacity(double arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__View__setWindowOpacity_double(void *thisObj, double arg__1);
// KDDockWidgets::Core::View::setWindowTitle(const QString & title)
DOCKS_EXPORT void c_KDDockWidgets__Core__View__setWindowTitle_QString(void *thisObj, const char *title_);
// KDDockWidgets::Core::View::setZOrder(int arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__View__setZOrder_int(void *thisObj, int arg__1);
// KDDockWidgets::Core::View::show()
DOCKS_EXPORT void c_KDDockWidgets__Core__View__show(void *thisObj);
// KDDockWidgets::Core::View::showMaximized()
DOCKS_EXPORT void c_KDDockWidgets__Core__View__showMaximized(void *thisObj);
// KDDockWidgets::Core::View::showMinimized()
DOCKS_EXPORT void c_KDDockWidgets__Core__View__showMinimized(void *thisObj);
// KDDockWidgets::Core::View::showNormal()
DOCKS_EXPORT void c_KDDockWidgets__Core__View__showNormal(void *thisObj);
// KDDockWidgets::Core::View::size() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__View__size(void *thisObj);
// KDDockWidgets::Core::View::update()
DOCKS_EXPORT void c_KDDockWidgets__Core__View__update(void *thisObj);
// KDDockWidgets::Core::View::viewName() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__View__viewName(void *thisObj);
// KDDockWidgets::Core::View::width() const
DOCKS_EXPORT int c_KDDockWidgets__Core__View__width(void *thisObj);
// KDDockWidgets::Core::View::x() const
DOCKS_EXPORT int c_KDDockWidgets__Core__View__x(void *thisObj);
// KDDockWidgets::Core::View::y() const
DOCKS_EXPORT int c_KDDockWidgets__Core__View__y(void *thisObj);
// KDDockWidgets::Core::View::zOrder() const
DOCKS_EXPORT int c_KDDockWidgets__Core__View__zOrder(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__View__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__View__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
DOCKS_EXPORT void c_KDDockWidgets__Core__View_Finalizer(void *cppObj);
}
