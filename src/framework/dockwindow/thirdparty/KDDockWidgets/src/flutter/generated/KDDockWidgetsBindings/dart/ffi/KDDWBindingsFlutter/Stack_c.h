/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <Stack.h>
#include "core/Stack.h"
#include <core/View.h>
#include <geometry_helpers_p.h>
#include <string_p.h>

namespace KDDockWidgetsBindings_wrappersNS {
namespace KDDWBindingsFlutter {
class Stack_wrapper : public ::KDDockWidgets::flutter::Stack
{
public:
    ~Stack_wrapper();
    Stack_wrapper(KDDockWidgets::Core::Stack *controller, KDDockWidgets::Core::View *parent = nullptr);
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
    virtual bool isExplicitlyHidden() const;
    virtual bool isExplicitlyHidden_nocallback() const;
    virtual bool isMaximized() const;
    virtual bool isMaximized_nocallback() const;
    virtual bool isMinimized() const;
    virtual bool isMinimized_nocallback() const;
    virtual bool isMounted() const;
    virtual bool isMounted_nocallback() const;
    virtual bool isNull() const;
    virtual bool isNull_nocallback() const;
    virtual bool isPositionDraggable(KDDockWidgets::Point p) const;
    virtual bool isPositionDraggable_nocallback(KDDockWidgets::Point p) const;
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
    virtual void setDocumentMode(bool arg__1);
    virtual void setDocumentMode_nocallback(bool arg__1);
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
    typedef bool (*Callback_isExplicitlyHidden)(void *);
    Callback_isExplicitlyHidden m_isExplicitlyHiddenCallback = nullptr;
    typedef bool (*Callback_isMaximized)(void *);
    Callback_isMaximized m_isMaximizedCallback = nullptr;
    typedef bool (*Callback_isMinimized)(void *);
    Callback_isMinimized m_isMinimizedCallback = nullptr;
    typedef bool (*Callback_isMounted)(void *);
    Callback_isMounted m_isMountedCallback = nullptr;
    typedef bool (*Callback_isNull)(void *);
    Callback_isNull m_isNullCallback = nullptr;
    typedef bool (*Callback_isPositionDraggable)(void *, KDDockWidgets::Point *p);
    Callback_isPositionDraggable m_isPositionDraggableCallback = nullptr;
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
    typedef void (*Callback_setDocumentMode)(void *, bool arg__1);
    Callback_setDocumentMode m_setDocumentModeCallback = nullptr;
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
// KDDockWidgets::flutter::Stack::Stack(KDDockWidgets::Core::Stack * controller, KDDockWidgets::Core::View * parent)
DOCKS_EXPORT void *c_KDDockWidgets__flutter__Stack__constructor_Stack_View(void *controller_, void *parent_);
// KDDockWidgets::flutter::Stack::activateWindow()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__activateWindow(void *thisObj);
// KDDockWidgets::flutter::Stack::close()
DOCKS_EXPORT bool c_KDDockWidgets__flutter__Stack__close(void *thisObj);
// KDDockWidgets::flutter::Stack::createPlatformWindow()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__createPlatformWindow(void *thisObj);
// KDDockWidgets::flutter::Stack::flags() const
DOCKS_EXPORT int c_KDDockWidgets__flutter__Stack__flags(void *thisObj);
// KDDockWidgets::flutter::Stack::geometry() const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__Stack__geometry(void *thisObj);
// KDDockWidgets::flutter::Stack::grabMouse()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__grabMouse(void *thisObj);
// KDDockWidgets::flutter::Stack::hasFocus() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__Stack__hasFocus(void *thisObj);
// KDDockWidgets::flutter::Stack::hide()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__hide(void *thisObj);
// KDDockWidgets::flutter::Stack::init()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__init(void *thisObj);
// KDDockWidgets::flutter::Stack::isActiveWindow() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__Stack__isActiveWindow(void *thisObj);
// KDDockWidgets::flutter::Stack::isExplicitlyHidden() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__Stack__isExplicitlyHidden(void *thisObj);
// KDDockWidgets::flutter::Stack::isMaximized() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__Stack__isMaximized(void *thisObj);
// KDDockWidgets::flutter::Stack::isMinimized() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__Stack__isMinimized(void *thisObj);
// KDDockWidgets::flutter::Stack::isMounted() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__Stack__isMounted(void *thisObj);
// KDDockWidgets::flutter::Stack::isNull() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__Stack__isNull(void *thisObj);
// KDDockWidgets::flutter::Stack::isPositionDraggable(KDDockWidgets::Point p) const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__Stack__isPositionDraggable_Point(void *thisObj, void *p_);
// KDDockWidgets::flutter::Stack::isRootView() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__Stack__isRootView(void *thisObj);
// KDDockWidgets::flutter::Stack::isVisible() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__Stack__isVisible(void *thisObj);
// KDDockWidgets::flutter::Stack::mapFromGlobal(KDDockWidgets::Point globalPt) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__Stack__mapFromGlobal_Point(void *thisObj, void *globalPt_);
// KDDockWidgets::flutter::Stack::mapTo(KDDockWidgets::Core::View * parent, KDDockWidgets::Point pos) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__Stack__mapTo_View_Point(void *thisObj, void *parent_, void *pos_);
// KDDockWidgets::flutter::Stack::mapToGlobal(KDDockWidgets::Point localPt) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__Stack__mapToGlobal_Point(void *thisObj, void *localPt_);
// KDDockWidgets::flutter::Stack::maxSizeHint() const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__Stack__maxSizeHint(void *thisObj);
// KDDockWidgets::flutter::Stack::minSize() const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__Stack__minSize(void *thisObj);
// KDDockWidgets::flutter::Stack::move(int x, int y)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__move_int_int(void *thisObj, int x, int y);
// KDDockWidgets::flutter::Stack::normalGeometry() const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__Stack__normalGeometry(void *thisObj);
// KDDockWidgets::flutter::Stack::onChildAdded(KDDockWidgets::Core::View * childView)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__onChildAdded_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::Stack::onChildRemoved(KDDockWidgets::Core::View * childView)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__onChildRemoved_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::Stack::onChildVisibilityChanged(KDDockWidgets::Core::View * childView)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__onChildVisibilityChanged_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::Stack::onGeometryChanged()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__onGeometryChanged(void *thisObj);
// KDDockWidgets::flutter::Stack::onRebuildRequested()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__onRebuildRequested(void *thisObj);
// KDDockWidgets::flutter::Stack::onResize(int h, int w)
DOCKS_EXPORT bool c_KDDockWidgets__flutter__Stack__onResize_int_int(void *thisObj, int h, int w);
// KDDockWidgets::flutter::Stack::raise()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__raise(void *thisObj);
// KDDockWidgets::flutter::Stack::raiseAndActivate()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__raiseAndActivate(void *thisObj);
// KDDockWidgets::flutter::Stack::raiseChild(KDDockWidgets::Core::View * childView)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__raiseChild_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::Stack::raiseWindow(KDDockWidgets::Core::View * rootView)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__raiseWindow_View(void *thisObj, void *rootView_);
// KDDockWidgets::flutter::Stack::releaseKeyboard()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__releaseKeyboard(void *thisObj);
// KDDockWidgets::flutter::Stack::releaseMouse()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__releaseMouse(void *thisObj);
// KDDockWidgets::flutter::Stack::setCursor(Qt::CursorShape shape)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__setCursor_CursorShape(void *thisObj, int shape);
// KDDockWidgets::flutter::Stack::setDocumentMode(bool arg__1)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__setDocumentMode_bool(void *thisObj, bool arg__1);
// KDDockWidgets::flutter::Stack::setFixedHeight(int h)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__setFixedHeight_int(void *thisObj, int h);
// KDDockWidgets::flutter::Stack::setFixedWidth(int w)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__setFixedWidth_int(void *thisObj, int w);
// KDDockWidgets::flutter::Stack::setGeometry(KDDockWidgets::Rect geometry)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__setGeometry_Rect(void *thisObj, void *geometry_);
// KDDockWidgets::flutter::Stack::setHeight(int h)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__setHeight_int(void *thisObj, int h);
// KDDockWidgets::flutter::Stack::setMaximumSize(KDDockWidgets::Size sz)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__setMaximumSize_Size(void *thisObj, void *sz_);
// KDDockWidgets::flutter::Stack::setMinimumSize(KDDockWidgets::Size sz)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__setMinimumSize_Size(void *thisObj, void *sz_);
// KDDockWidgets::flutter::Stack::setMouseTracking(bool enable)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__setMouseTracking_bool(void *thisObj, bool enable);
// KDDockWidgets::flutter::Stack::setParent(KDDockWidgets::Core::View * parent)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__setParent_View(void *thisObj, void *parent_);
// KDDockWidgets::flutter::Stack::setSize(int w, int h)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__setSize_int_int(void *thisObj, int w, int h);
// KDDockWidgets::flutter::Stack::setViewName(const QString & name)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__setViewName_QString(void *thisObj, const char *name_);
// KDDockWidgets::flutter::Stack::setVisible(bool visible)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__setVisible_bool(void *thisObj, bool visible);
// KDDockWidgets::flutter::Stack::setWidth(int w)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__setWidth_int(void *thisObj, int w);
// KDDockWidgets::flutter::Stack::setWindowOpacity(double v)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__setWindowOpacity_double(void *thisObj, double v);
// KDDockWidgets::flutter::Stack::setWindowTitle(const QString & title)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__setWindowTitle_QString(void *thisObj, const char *title_);
// KDDockWidgets::flutter::Stack::setZOrder(int z)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__setZOrder_int(void *thisObj, int z);
// KDDockWidgets::flutter::Stack::show()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__show(void *thisObj);
// KDDockWidgets::flutter::Stack::showMaximized()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__showMaximized(void *thisObj);
// KDDockWidgets::flutter::Stack::showMinimized()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__showMinimized(void *thisObj);
// KDDockWidgets::flutter::Stack::showNormal()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__showNormal(void *thisObj);
// KDDockWidgets::flutter::Stack::update()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__update(void *thisObj);
// KDDockWidgets::flutter::Stack::viewName() const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__Stack__viewName(void *thisObj);
// KDDockWidgets::flutter::Stack::zOrder() const
DOCKS_EXPORT int c_KDDockWidgets__flutter__Stack__zOrder(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
DOCKS_EXPORT void c_KDDockWidgets__flutter__Stack_Finalizer(void *cppObj);
}
