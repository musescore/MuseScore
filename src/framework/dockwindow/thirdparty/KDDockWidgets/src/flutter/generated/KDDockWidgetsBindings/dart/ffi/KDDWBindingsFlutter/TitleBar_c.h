/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <TitleBar.h>
#include "core/TitleBar.h"
#include <core/View.h>
#include <string_p.h>
#include <geometry_helpers_p.h>

namespace KDDockWidgetsBindings_wrappersNS {
namespace KDDWBindingsFlutter {
class TitleBar_wrapper : public ::KDDockWidgets::flutter::TitleBar
{
public:
    ~TitleBar_wrapper();
    TitleBar_wrapper(KDDockWidgets::Core::TitleBar *controller, KDDockWidgets::Core::View *parent = nullptr);
    virtual void activateWindow();
    virtual void activateWindow_nocallback();
    virtual bool close();
    virtual bool close_nocallback();
    virtual void createPlatformWindow();
    virtual void createPlatformWindow_nocallback();
    virtual Qt::WindowFlags flags() const;
    virtual Qt::WindowFlags flags_nocallback() const;
    virtual KDDockWidgets::Rect geometry() const;
    virtual KDDockWidgets::Rect geometry_nocallback() const;
    virtual void grabMouse();
    virtual void grabMouse_nocallback();
    virtual bool hasFocus() const;
    virtual bool hasFocus_nocallback() const;
    virtual void hide();
    virtual void hide_nocallback();
    virtual void init();
    virtual void init_nocallback();
    virtual bool isActiveWindow() const;
    virtual bool isActiveWindow_nocallback() const;
    virtual bool isCloseButtonEnabled() const;
    virtual bool isCloseButtonEnabled_nocallback() const;
    virtual bool isCloseButtonVisible() const;
    virtual bool isCloseButtonVisible_nocallback() const;
    virtual bool isExplicitlyHidden() const;
    virtual bool isExplicitlyHidden_nocallback() const;
    virtual bool isFloatButtonVisible() const;
    virtual bool isFloatButtonVisible_nocallback() const;
    virtual bool isMaximized() const;
    virtual bool isMaximized_nocallback() const;
    virtual bool isMinimized() const;
    virtual bool isMinimized_nocallback() const;
    virtual bool isMounted() const;
    virtual bool isMounted_nocallback() const;
    virtual bool isNull() const;
    virtual bool isNull_nocallback() const;
    virtual bool isRootView() const;
    virtual bool isRootView_nocallback() const;
    virtual bool isVisible() const;
    virtual bool isVisible_nocallback() const;
    virtual KDDockWidgets::Point mapFromGlobal(KDDockWidgets::Point globalPt) const;
    virtual KDDockWidgets::Point mapFromGlobal_nocallback(KDDockWidgets::Point globalPt) const;
    virtual KDDockWidgets::Point mapTo(KDDockWidgets::Core::View *parent, KDDockWidgets::Point pos) const;
    virtual KDDockWidgets::Point mapTo_nocallback(KDDockWidgets::Core::View *parent, KDDockWidgets::Point pos) const;
    virtual KDDockWidgets::Point mapToGlobal(KDDockWidgets::Point localPt) const;
    virtual KDDockWidgets::Point mapToGlobal_nocallback(KDDockWidgets::Point localPt) const;
    virtual KDDockWidgets::Size maxSizeHint() const;
    virtual KDDockWidgets::Size maxSizeHint_nocallback() const;
    virtual KDDockWidgets::Size minSize() const;
    virtual KDDockWidgets::Size minSize_nocallback() const;
    virtual void move(int x, int y);
    virtual void move_nocallback(int x, int y);
    virtual KDDockWidgets::Rect normalGeometry() const;
    virtual KDDockWidgets::Rect normalGeometry_nocallback() const;
    virtual void onChildAdded(KDDockWidgets::Core::View *childView);
    virtual void onChildAdded_nocallback(KDDockWidgets::Core::View *childView);
    virtual void onChildRemoved(KDDockWidgets::Core::View *childView);
    virtual void onChildRemoved_nocallback(KDDockWidgets::Core::View *childView);
    virtual void onChildVisibilityChanged(KDDockWidgets::Core::View *childView);
    virtual void onChildVisibilityChanged_nocallback(KDDockWidgets::Core::View *childView);
    virtual void onGeometryChanged();
    virtual void onGeometryChanged_nocallback();
    virtual void onRebuildRequested();
    virtual void onRebuildRequested_nocallback();
    virtual bool onResize(int h, int w);
    virtual bool onResize_nocallback(int h, int w);
    virtual void onTitleBarChanged(const QString &arg__1);
    virtual void onTitleBarChanged_nocallback(const QString &arg__1);
    virtual void raise();
    virtual void raise_nocallback();
    virtual void raiseAndActivate();
    virtual void raiseAndActivate_nocallback();
    virtual void raiseChild(KDDockWidgets::Core::View *childView);
    virtual void raiseChild_nocallback(KDDockWidgets::Core::View *childView);
    virtual void raiseWindow(KDDockWidgets::Core::View *rootView);
    virtual void raiseWindow_nocallback(KDDockWidgets::Core::View *rootView);
    virtual void releaseKeyboard();
    virtual void releaseKeyboard_nocallback();
    virtual void releaseMouse();
    virtual void releaseMouse_nocallback();
    virtual void setCursor(Qt::CursorShape shape);
    virtual void setCursor_nocallback(Qt::CursorShape shape);
    virtual void setFixedHeight(int h);
    virtual void setFixedHeight_nocallback(int h);
    virtual void setFixedWidth(int w);
    virtual void setFixedWidth_nocallback(int w);
    virtual void setGeometry(KDDockWidgets::Rect geometry);
    virtual void setGeometry_nocallback(KDDockWidgets::Rect geometry);
    virtual void setHeight(int h);
    virtual void setHeight_nocallback(int h);
    virtual void setMaximumSize(KDDockWidgets::Size sz);
    virtual void setMaximumSize_nocallback(KDDockWidgets::Size sz);
    virtual void setMinimumSize(KDDockWidgets::Size sz);
    virtual void setMinimumSize_nocallback(KDDockWidgets::Size sz);
    virtual void setMouseTracking(bool enable);
    virtual void setMouseTracking_nocallback(bool enable);
    virtual void setParent(KDDockWidgets::Core::View *parent);
    virtual void setParent_nocallback(KDDockWidgets::Core::View *parent);
    virtual void setSize(int w, int h);
    virtual void setSize_nocallback(int w, int h);
    virtual void setViewName(const QString &name);
    virtual void setViewName_nocallback(const QString &name);
    virtual void setVisible(bool visible);
    virtual void setVisible_nocallback(bool visible);
    virtual void setWidth(int w);
    virtual void setWidth_nocallback(int w);
    virtual void setWindowOpacity(double v);
    virtual void setWindowOpacity_nocallback(double v);
    virtual void setWindowTitle(const QString &title);
    virtual void setWindowTitle_nocallback(const QString &title);
    virtual void setZOrder(int z);
    virtual void setZOrder_nocallback(int z);
    virtual void show();
    virtual void show_nocallback();
    virtual void showMaximized();
    virtual void showMaximized_nocallback();
    virtual void showMinimized();
    virtual void showMinimized_nocallback();
    virtual void showNormal();
    virtual void showNormal_nocallback();
    virtual void update();
    virtual void update_nocallback();
    virtual QString viewName() const;
    virtual QString viewName_nocallback() const;
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
    typedef bool (*Callback_isCloseButtonEnabled)(void *);
    Callback_isCloseButtonEnabled m_isCloseButtonEnabledCallback = nullptr;
    typedef bool (*Callback_isCloseButtonVisible)(void *);
    Callback_isCloseButtonVisible m_isCloseButtonVisibleCallback = nullptr;
    typedef bool (*Callback_isExplicitlyHidden)(void *);
    Callback_isExplicitlyHidden m_isExplicitlyHiddenCallback = nullptr;
    typedef bool (*Callback_isFloatButtonVisible)(void *);
    Callback_isFloatButtonVisible m_isFloatButtonVisibleCallback = nullptr;
    typedef bool (*Callback_isMaximized)(void *);
    Callback_isMaximized m_isMaximizedCallback = nullptr;
    typedef bool (*Callback_isMinimized)(void *);
    Callback_isMinimized m_isMinimizedCallback = nullptr;
    typedef bool (*Callback_isMounted)(void *);
    Callback_isMounted m_isMountedCallback = nullptr;
    typedef bool (*Callback_isNull)(void *);
    Callback_isNull m_isNullCallback = nullptr;
    typedef bool (*Callback_isRootView)(void *);
    Callback_isRootView m_isRootViewCallback = nullptr;
    typedef bool (*Callback_isVisible)(void *);
    Callback_isVisible m_isVisibleCallback = nullptr;
    typedef KDDockWidgets::Point *(*Callback_mapFromGlobal)(void *, KDDockWidgets::Point *globalPt);
    Callback_mapFromGlobal m_mapFromGlobalCallback = nullptr;
    typedef KDDockWidgets::Point *(*Callback_mapTo)(void *, KDDockWidgets::Core::View *parent, KDDockWidgets::Point *pos);
    Callback_mapTo m_mapToCallback = nullptr;
    typedef KDDockWidgets::Point *(*Callback_mapToGlobal)(void *, KDDockWidgets::Point *localPt);
    Callback_mapToGlobal m_mapToGlobalCallback = nullptr;
    typedef KDDockWidgets::Size *(*Callback_maxSizeHint)(void *);
    Callback_maxSizeHint m_maxSizeHintCallback = nullptr;
    typedef KDDockWidgets::Size *(*Callback_minSize)(void *);
    Callback_minSize m_minSizeCallback = nullptr;
    typedef void (*Callback_move_2)(void *, int x, int y);
    Callback_move_2 m_move_2Callback = nullptr;
    typedef KDDockWidgets::Rect *(*Callback_normalGeometry)(void *);
    Callback_normalGeometry m_normalGeometryCallback = nullptr;
    typedef void (*Callback_onChildAdded)(void *, KDDockWidgets::Core::View *childView);
    Callback_onChildAdded m_onChildAddedCallback = nullptr;
    typedef void (*Callback_onChildRemoved)(void *, KDDockWidgets::Core::View *childView);
    Callback_onChildRemoved m_onChildRemovedCallback = nullptr;
    typedef void (*Callback_onChildVisibilityChanged)(void *, KDDockWidgets::Core::View *childView);
    Callback_onChildVisibilityChanged m_onChildVisibilityChangedCallback = nullptr;
    typedef void (*Callback_onGeometryChanged)(void *);
    Callback_onGeometryChanged m_onGeometryChangedCallback = nullptr;
    typedef void (*Callback_onRebuildRequested)(void *);
    Callback_onRebuildRequested m_onRebuildRequestedCallback = nullptr;
    typedef bool (*Callback_onResize_2)(void *, int h, int w);
    Callback_onResize_2 m_onResize_2Callback = nullptr;
    typedef void (*Callback_onTitleBarChanged)(void *, const QString &arg__1);
    Callback_onTitleBarChanged m_onTitleBarChangedCallback = nullptr;
    typedef void (*Callback_raise)(void *);
    Callback_raise m_raiseCallback = nullptr;
    typedef void (*Callback_raiseAndActivate)(void *);
    Callback_raiseAndActivate m_raiseAndActivateCallback = nullptr;
    typedef void (*Callback_raiseChild)(void *, KDDockWidgets::Core::View *childView);
    Callback_raiseChild m_raiseChildCallback = nullptr;
    typedef void (*Callback_raiseWindow)(void *, KDDockWidgets::Core::View *rootView);
    Callback_raiseWindow m_raiseWindowCallback = nullptr;
    typedef void (*Callback_releaseKeyboard)(void *);
    Callback_releaseKeyboard m_releaseKeyboardCallback = nullptr;
    typedef void (*Callback_releaseMouse)(void *);
    Callback_releaseMouse m_releaseMouseCallback = nullptr;
    typedef void (*Callback_setCursor)(void *, Qt::CursorShape shape);
    Callback_setCursor m_setCursorCallback = nullptr;
    typedef void (*Callback_setFixedHeight)(void *, int h);
    Callback_setFixedHeight m_setFixedHeightCallback = nullptr;
    typedef void (*Callback_setFixedWidth)(void *, int w);
    Callback_setFixedWidth m_setFixedWidthCallback = nullptr;
    typedef void (*Callback_setGeometry)(void *, KDDockWidgets::Rect *geometry);
    Callback_setGeometry m_setGeometryCallback = nullptr;
    typedef void (*Callback_setHeight)(void *, int h);
    Callback_setHeight m_setHeightCallback = nullptr;
    typedef void (*Callback_setMaximumSize)(void *, KDDockWidgets::Size *sz);
    Callback_setMaximumSize m_setMaximumSizeCallback = nullptr;
    typedef void (*Callback_setMinimumSize)(void *, KDDockWidgets::Size *sz);
    Callback_setMinimumSize m_setMinimumSizeCallback = nullptr;
    typedef void (*Callback_setMouseTracking)(void *, bool enable);
    Callback_setMouseTracking m_setMouseTrackingCallback = nullptr;
    typedef void (*Callback_setParent)(void *, KDDockWidgets::Core::View *parent);
    Callback_setParent m_setParentCallback = nullptr;
    typedef void (*Callback_setSize_2)(void *, int w, int h);
    Callback_setSize_2 m_setSize_2Callback = nullptr;
    typedef void (*Callback_setViewName)(void *, const QString &name);
    Callback_setViewName m_setViewNameCallback = nullptr;
    typedef void (*Callback_setVisible)(void *, bool visible);
    Callback_setVisible m_setVisibleCallback = nullptr;
    typedef void (*Callback_setWidth)(void *, int w);
    Callback_setWidth m_setWidthCallback = nullptr;
    typedef void (*Callback_setWindowOpacity)(void *, double v);
    Callback_setWindowOpacity m_setWindowOpacityCallback = nullptr;
    typedef void (*Callback_setWindowTitle)(void *, const QString &title);
    Callback_setWindowTitle m_setWindowTitleCallback = nullptr;
    typedef void (*Callback_setZOrder)(void *, int z);
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
// KDDockWidgets::flutter::TitleBar::TitleBar(KDDockWidgets::Core::TitleBar * controller, KDDockWidgets::Core::View * parent)
DOCKS_EXPORT void *c_KDDockWidgets__flutter__TitleBar__constructor_TitleBar_View(void *controller_, void *parent_);
// KDDockWidgets::flutter::TitleBar::activateWindow()
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__activateWindow(void *thisObj);
// KDDockWidgets::flutter::TitleBar::close()
DOCKS_EXPORT bool c_KDDockWidgets__flutter__TitleBar__close(void *thisObj);
// KDDockWidgets::flutter::TitleBar::createPlatformWindow()
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__createPlatformWindow(void *thisObj);
// KDDockWidgets::flutter::TitleBar::flags() const
DOCKS_EXPORT int c_KDDockWidgets__flutter__TitleBar__flags(void *thisObj);
// KDDockWidgets::flutter::TitleBar::geometry() const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__TitleBar__geometry(void *thisObj);
// KDDockWidgets::flutter::TitleBar::grabMouse()
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__grabMouse(void *thisObj);
// KDDockWidgets::flutter::TitleBar::hasFocus() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__TitleBar__hasFocus(void *thisObj);
// KDDockWidgets::flutter::TitleBar::hide()
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__hide(void *thisObj);
// KDDockWidgets::flutter::TitleBar::init()
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__init(void *thisObj);
// KDDockWidgets::flutter::TitleBar::isActiveWindow() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__TitleBar__isActiveWindow(void *thisObj);
// KDDockWidgets::flutter::TitleBar::isCloseButtonEnabled() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__TitleBar__isCloseButtonEnabled(void *thisObj);
// KDDockWidgets::flutter::TitleBar::isCloseButtonVisible() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__TitleBar__isCloseButtonVisible(void *thisObj);
// KDDockWidgets::flutter::TitleBar::isExplicitlyHidden() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__TitleBar__isExplicitlyHidden(void *thisObj);
// KDDockWidgets::flutter::TitleBar::isFloatButtonVisible() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__TitleBar__isFloatButtonVisible(void *thisObj);
// KDDockWidgets::flutter::TitleBar::isMaximized() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__TitleBar__isMaximized(void *thisObj);
// KDDockWidgets::flutter::TitleBar::isMinimized() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__TitleBar__isMinimized(void *thisObj);
// KDDockWidgets::flutter::TitleBar::isMounted() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__TitleBar__isMounted(void *thisObj);
// KDDockWidgets::flutter::TitleBar::isNull() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__TitleBar__isNull(void *thisObj);
// KDDockWidgets::flutter::TitleBar::isRootView() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__TitleBar__isRootView(void *thisObj);
// KDDockWidgets::flutter::TitleBar::isVisible() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__TitleBar__isVisible(void *thisObj);
// KDDockWidgets::flutter::TitleBar::mapFromGlobal(KDDockWidgets::Point globalPt) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__TitleBar__mapFromGlobal_Point(void *thisObj, void *globalPt_);
// KDDockWidgets::flutter::TitleBar::mapTo(KDDockWidgets::Core::View * parent, KDDockWidgets::Point pos) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__TitleBar__mapTo_View_Point(void *thisObj, void *parent_, void *pos_);
// KDDockWidgets::flutter::TitleBar::mapToGlobal(KDDockWidgets::Point localPt) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__TitleBar__mapToGlobal_Point(void *thisObj, void *localPt_);
// KDDockWidgets::flutter::TitleBar::maxSizeHint() const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__TitleBar__maxSizeHint(void *thisObj);
// KDDockWidgets::flutter::TitleBar::minSize() const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__TitleBar__minSize(void *thisObj);
// KDDockWidgets::flutter::TitleBar::move(int x, int y)
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__move_int_int(void *thisObj, int x, int y);
// KDDockWidgets::flutter::TitleBar::normalGeometry() const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__TitleBar__normalGeometry(void *thisObj);
// KDDockWidgets::flutter::TitleBar::onChildAdded(KDDockWidgets::Core::View * childView)
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__onChildAdded_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::TitleBar::onChildRemoved(KDDockWidgets::Core::View * childView)
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__onChildRemoved_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::TitleBar::onChildVisibilityChanged(KDDockWidgets::Core::View * childView)
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__onChildVisibilityChanged_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::TitleBar::onGeometryChanged()
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__onGeometryChanged(void *thisObj);
// KDDockWidgets::flutter::TitleBar::onRebuildRequested()
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__onRebuildRequested(void *thisObj);
// KDDockWidgets::flutter::TitleBar::onResize(int h, int w)
DOCKS_EXPORT bool c_KDDockWidgets__flutter__TitleBar__onResize_int_int(void *thisObj, int h, int w);
// KDDockWidgets::flutter::TitleBar::onTitleBarChanged(const QString & arg__1)
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__onTitleBarChanged_QString(void *thisObj, const char *arg__1_);
// KDDockWidgets::flutter::TitleBar::raise()
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__raise(void *thisObj);
// KDDockWidgets::flutter::TitleBar::raiseAndActivate()
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__raiseAndActivate(void *thisObj);
// KDDockWidgets::flutter::TitleBar::raiseChild(KDDockWidgets::Core::View * childView)
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__raiseChild_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::TitleBar::raiseWindow(KDDockWidgets::Core::View * rootView)
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__raiseWindow_View(void *thisObj, void *rootView_);
// KDDockWidgets::flutter::TitleBar::releaseKeyboard()
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__releaseKeyboard(void *thisObj);
// KDDockWidgets::flutter::TitleBar::releaseMouse()
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__releaseMouse(void *thisObj);
// KDDockWidgets::flutter::TitleBar::setCursor(Qt::CursorShape shape)
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__setCursor_CursorShape(void *thisObj, int shape);
// KDDockWidgets::flutter::TitleBar::setFixedHeight(int h)
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__setFixedHeight_int(void *thisObj, int h);
// KDDockWidgets::flutter::TitleBar::setFixedWidth(int w)
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__setFixedWidth_int(void *thisObj, int w);
// KDDockWidgets::flutter::TitleBar::setGeometry(KDDockWidgets::Rect geometry)
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__setGeometry_Rect(void *thisObj, void *geometry_);
// KDDockWidgets::flutter::TitleBar::setHeight(int h)
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__setHeight_int(void *thisObj, int h);
// KDDockWidgets::flutter::TitleBar::setMaximumSize(KDDockWidgets::Size sz)
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__setMaximumSize_Size(void *thisObj, void *sz_);
// KDDockWidgets::flutter::TitleBar::setMinimumSize(KDDockWidgets::Size sz)
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__setMinimumSize_Size(void *thisObj, void *sz_);
// KDDockWidgets::flutter::TitleBar::setMouseTracking(bool enable)
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__setMouseTracking_bool(void *thisObj, bool enable);
// KDDockWidgets::flutter::TitleBar::setParent(KDDockWidgets::Core::View * parent)
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__setParent_View(void *thisObj, void *parent_);
// KDDockWidgets::flutter::TitleBar::setSize(int w, int h)
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__setSize_int_int(void *thisObj, int w, int h);
// KDDockWidgets::flutter::TitleBar::setViewName(const QString & name)
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__setViewName_QString(void *thisObj, const char *name_);
// KDDockWidgets::flutter::TitleBar::setVisible(bool visible)
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__setVisible_bool(void *thisObj, bool visible);
// KDDockWidgets::flutter::TitleBar::setWidth(int w)
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__setWidth_int(void *thisObj, int w);
// KDDockWidgets::flutter::TitleBar::setWindowOpacity(double v)
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__setWindowOpacity_double(void *thisObj, double v);
// KDDockWidgets::flutter::TitleBar::setWindowTitle(const QString & title)
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__setWindowTitle_QString(void *thisObj, const char *title_);
// KDDockWidgets::flutter::TitleBar::setZOrder(int z)
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__setZOrder_int(void *thisObj, int z);
// KDDockWidgets::flutter::TitleBar::show()
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__show(void *thisObj);
// KDDockWidgets::flutter::TitleBar::showMaximized()
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__showMaximized(void *thisObj);
// KDDockWidgets::flutter::TitleBar::showMinimized()
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__showMinimized(void *thisObj);
// KDDockWidgets::flutter::TitleBar::showNormal()
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__showNormal(void *thisObj);
// KDDockWidgets::flutter::TitleBar::update()
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__update(void *thisObj);
// KDDockWidgets::flutter::TitleBar::viewName() const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__TitleBar__viewName(void *thisObj);
// KDDockWidgets::flutter::TitleBar::zOrder() const
DOCKS_EXPORT int c_KDDockWidgets__flutter__TitleBar__zOrder(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
DOCKS_EXPORT void c_KDDockWidgets__flutter__TitleBar_Finalizer(void *cppObj);
}
