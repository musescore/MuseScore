/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <ClassicIndicatorsWindow.h>
#include <ClassicDropIndicatorOverlay.h>
#include <core/View.h>
#include <geometry_helpers_p.h>
#include <string_p.h>
#include <core/Group.h>

namespace KDDockWidgetsBindings_wrappersNS {
namespace KDDWBindingsFlutter {
class IndicatorWindow_wrapper : public ::KDDockWidgets::flutter::IndicatorWindow
{
public:
    ~IndicatorWindow_wrapper();
    IndicatorWindow_wrapper(KDDockWidgets::Core::ClassicDropIndicatorOverlay *arg__1, KDDockWidgets::Core::View *parent);
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
    virtual KDDockWidgets::DropLocation hover(KDDockWidgets::Point globalPos);
    virtual KDDockWidgets::DropLocation hover_nocallback(KDDockWidgets::Point globalPos);
    virtual KDDockWidgets::DropLocation hover_flutter(KDDockWidgets::Point globalPos);
    virtual KDDockWidgets::DropLocation hover_flutter_nocallback(KDDockWidgets::Point globalPos);
    KDDockWidgets::Core::Group *hoveredGroup() const;
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
    virtual bool isRootView() const;
    virtual bool isRootView_nocallback() const;
    virtual bool isVisible() const;
    virtual bool isVisible_nocallback() const;
    virtual bool isWindow() const;
    virtual bool isWindow_nocallback() const;
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
    virtual KDDockWidgets::Point posForIndicator(KDDockWidgets::DropLocation arg__1) const;
    virtual KDDockWidgets::Point posForIndicator_nocallback(KDDockWidgets::DropLocation arg__1) const;
    virtual KDDockWidgets::Point posForIndicator_flutter(KDDockWidgets::DropLocation arg__1) const;
    virtual KDDockWidgets::Point posForIndicator_flutter_nocallback(KDDockWidgets::DropLocation arg__1) const;
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
    virtual void resize(KDDockWidgets::Size arg__1);
    virtual void resize_nocallback(KDDockWidgets::Size arg__1);
    KDDockWidgets::Core::View *rubberBand() const;
    virtual void setCursor(Qt::CursorShape shape);
    virtual void setCursor_nocallback(Qt::CursorShape shape);
    virtual void setFixedHeight(int h);
    virtual void setFixedHeight_nocallback(int h);
    virtual void setFixedWidth(int w);
    virtual void setFixedWidth_nocallback(int w);
    virtual void setGeometry(KDDockWidgets::Rect arg__1);
    virtual void setGeometry_nocallback(KDDockWidgets::Rect arg__1);
    virtual void setHeight(int h);
    virtual void setHeight_nocallback(int h);
    virtual void setMaximumSize(KDDockWidgets::Size sz);
    virtual void setMaximumSize_nocallback(KDDockWidgets::Size sz);
    virtual void setMinimumSize(KDDockWidgets::Size sz);
    virtual void setMinimumSize_nocallback(KDDockWidgets::Size sz);
    virtual void setMouseTracking(bool enable);
    virtual void setMouseTracking_nocallback(bool enable);
    virtual void setObjectName(const QString &arg__1);
    virtual void setObjectName_nocallback(const QString &arg__1);
    virtual void setParent(KDDockWidgets::Core::View *parent);
    virtual void setParent_nocallback(KDDockWidgets::Core::View *parent);
    virtual void setSize(int w, int h);
    virtual void setSize_nocallback(int w, int h);
    virtual void setViewName(const QString &name);
    virtual void setViewName_nocallback(const QString &name);
    virtual void setVisible(bool arg__1);
    virtual void setVisible_nocallback(bool arg__1);
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
    virtual void updateIndicatorVisibility();
    virtual void updateIndicatorVisibility_nocallback();
    virtual void updatePositions();
    virtual void updatePositions_nocallback();
    virtual bool updatePositions_flutter(int overlayWidth, int overlayHeight, KDDockWidgets::Core::Group *hoveredGroup, int visibleLocations);
    virtual bool updatePositions_flutter_nocallback(int overlayWidth, int overlayHeight, KDDockWidgets::Core::Group *hoveredGroup, int visibleLocations);
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
    typedef KDDockWidgets::DropLocation (*Callback_hover)(void *, KDDockWidgets::Point *globalPos);
    Callback_hover m_hoverCallback = nullptr;
    typedef KDDockWidgets::DropLocation (*Callback_hover_flutter)(void *, KDDockWidgets::Point *globalPos);
    Callback_hover_flutter m_hover_flutterCallback = nullptr;
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
    typedef bool (*Callback_isRootView)(void *);
    Callback_isRootView m_isRootViewCallback = nullptr;
    typedef bool (*Callback_isVisible)(void *);
    Callback_isVisible m_isVisibleCallback = nullptr;
    typedef bool (*Callback_isWindow)(void *);
    Callback_isWindow m_isWindowCallback = nullptr;
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
    typedef KDDockWidgets::Point *(*Callback_posForIndicator)(void *, KDDockWidgets::DropLocation arg__1);
    Callback_posForIndicator m_posForIndicatorCallback = nullptr;
    typedef KDDockWidgets::Point *(*Callback_posForIndicator_flutter)(void *, KDDockWidgets::DropLocation arg__1);
    Callback_posForIndicator_flutter m_posForIndicator_flutterCallback = nullptr;
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
    typedef void (*Callback_resize)(void *, KDDockWidgets::Size *arg__1);
    Callback_resize m_resizeCallback = nullptr;
    typedef void (*Callback_setCursor)(void *, Qt::CursorShape shape);
    Callback_setCursor m_setCursorCallback = nullptr;
    typedef void (*Callback_setFixedHeight)(void *, int h);
    Callback_setFixedHeight m_setFixedHeightCallback = nullptr;
    typedef void (*Callback_setFixedWidth)(void *, int w);
    Callback_setFixedWidth m_setFixedWidthCallback = nullptr;
    typedef void (*Callback_setGeometry)(void *, KDDockWidgets::Rect *arg__1);
    Callback_setGeometry m_setGeometryCallback = nullptr;
    typedef void (*Callback_setHeight)(void *, int h);
    Callback_setHeight m_setHeightCallback = nullptr;
    typedef void (*Callback_setMaximumSize)(void *, KDDockWidgets::Size *sz);
    Callback_setMaximumSize m_setMaximumSizeCallback = nullptr;
    typedef void (*Callback_setMinimumSize)(void *, KDDockWidgets::Size *sz);
    Callback_setMinimumSize m_setMinimumSizeCallback = nullptr;
    typedef void (*Callback_setMouseTracking)(void *, bool enable);
    Callback_setMouseTracking m_setMouseTrackingCallback = nullptr;
    typedef void (*Callback_setObjectName)(void *, const QString &arg__1);
    Callback_setObjectName m_setObjectNameCallback = nullptr;
    typedef void (*Callback_setParent)(void *, KDDockWidgets::Core::View *parent);
    Callback_setParent m_setParentCallback = nullptr;
    typedef void (*Callback_setSize_2)(void *, int w, int h);
    Callback_setSize_2 m_setSize_2Callback = nullptr;
    typedef void (*Callback_setViewName)(void *, const QString &name);
    Callback_setViewName m_setViewNameCallback = nullptr;
    typedef void (*Callback_setVisible)(void *, bool arg__1);
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
    typedef void (*Callback_updateIndicatorVisibility)(void *);
    Callback_updateIndicatorVisibility m_updateIndicatorVisibilityCallback = nullptr;
    typedef void (*Callback_updatePositions)(void *);
    Callback_updatePositions m_updatePositionsCallback = nullptr;
    typedef bool (*Callback_updatePositions_flutter)(void *, int overlayWidth, int overlayHeight, KDDockWidgets::Core::Group *hoveredGroup, int visibleLocations);
    Callback_updatePositions_flutter m_updatePositions_flutterCallback = nullptr;
    typedef QString *(*Callback_viewName)(void *);
    Callback_viewName m_viewNameCallback = nullptr;
    typedef int (*Callback_zOrder)(void *);
    Callback_zOrder m_zOrderCallback = nullptr;
};
}
}
extern "C" {
// KDDockWidgets::flutter::IndicatorWindow::IndicatorWindow(KDDockWidgets::Core::ClassicDropIndicatorOverlay * arg__1, KDDockWidgets::Core::View * parent)
DOCKS_EXPORT void *c_KDDockWidgets__flutter__IndicatorWindow__constructor_ClassicDropIndicatorOverlay_View(void *arg__1_, void *parent_);
// KDDockWidgets::flutter::IndicatorWindow::activateWindow()
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__activateWindow(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::close()
DOCKS_EXPORT bool c_KDDockWidgets__flutter__IndicatorWindow__close(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::createPlatformWindow()
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__createPlatformWindow(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::flags() const
DOCKS_EXPORT int c_KDDockWidgets__flutter__IndicatorWindow__flags(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::geometry() const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__IndicatorWindow__geometry(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::grabMouse()
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__grabMouse(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::hasFocus() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__IndicatorWindow__hasFocus(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::hide()
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__hide(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::hover(KDDockWidgets::Point globalPos)
DOCKS_EXPORT int c_KDDockWidgets__flutter__IndicatorWindow__hover_Point(void *thisObj, void *globalPos_);
// KDDockWidgets::flutter::IndicatorWindow::hover_flutter(KDDockWidgets::Point globalPos)
DOCKS_EXPORT int c_KDDockWidgets__flutter__IndicatorWindow__hover_flutter_Point(void *thisObj, void *globalPos_);
// KDDockWidgets::flutter::IndicatorWindow::hoveredGroup() const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__IndicatorWindow__hoveredGroup(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::init()
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__init(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::isActiveWindow() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__IndicatorWindow__isActiveWindow(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::isExplicitlyHidden() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__IndicatorWindow__isExplicitlyHidden(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::isMaximized() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__IndicatorWindow__isMaximized(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::isMinimized() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__IndicatorWindow__isMinimized(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::isMounted() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__IndicatorWindow__isMounted(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::isNull() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__IndicatorWindow__isNull(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::isRootView() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__IndicatorWindow__isRootView(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::isVisible() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__IndicatorWindow__isVisible(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::isWindow() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__IndicatorWindow__isWindow(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::mapFromGlobal(KDDockWidgets::Point globalPt) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__IndicatorWindow__mapFromGlobal_Point(void *thisObj, void *globalPt_);
// KDDockWidgets::flutter::IndicatorWindow::mapTo(KDDockWidgets::Core::View * parent, KDDockWidgets::Point pos) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__IndicatorWindow__mapTo_View_Point(void *thisObj, void *parent_, void *pos_);
// KDDockWidgets::flutter::IndicatorWindow::mapToGlobal(KDDockWidgets::Point localPt) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__IndicatorWindow__mapToGlobal_Point(void *thisObj, void *localPt_);
// KDDockWidgets::flutter::IndicatorWindow::maxSizeHint() const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__IndicatorWindow__maxSizeHint(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::minSize() const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__IndicatorWindow__minSize(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::move(int x, int y)
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__move_int_int(void *thisObj, int x, int y);
// KDDockWidgets::flutter::IndicatorWindow::normalGeometry() const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__IndicatorWindow__normalGeometry(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::onChildAdded(KDDockWidgets::Core::View * childView)
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__onChildAdded_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::IndicatorWindow::onChildRemoved(KDDockWidgets::Core::View * childView)
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__onChildRemoved_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::IndicatorWindow::onChildVisibilityChanged(KDDockWidgets::Core::View * childView)
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__onChildVisibilityChanged_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::IndicatorWindow::onGeometryChanged()
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__onGeometryChanged(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::onRebuildRequested()
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__onRebuildRequested(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::onResize(int h, int w)
DOCKS_EXPORT bool c_KDDockWidgets__flutter__IndicatorWindow__onResize_int_int(void *thisObj, int h, int w);
// KDDockWidgets::flutter::IndicatorWindow::posForIndicator(KDDockWidgets::DropLocation arg__1) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__IndicatorWindow__posForIndicator_DropLocation(void *thisObj, int arg__1);
// KDDockWidgets::flutter::IndicatorWindow::posForIndicator_flutter(KDDockWidgets::DropLocation arg__1) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__IndicatorWindow__posForIndicator_flutter_DropLocation(void *thisObj, int arg__1);
// KDDockWidgets::flutter::IndicatorWindow::raise()
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__raise(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::raiseAndActivate()
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__raiseAndActivate(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::raiseChild(KDDockWidgets::Core::View * childView)
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__raiseChild_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::IndicatorWindow::raiseWindow(KDDockWidgets::Core::View * rootView)
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__raiseWindow_View(void *thisObj, void *rootView_);
// KDDockWidgets::flutter::IndicatorWindow::releaseKeyboard()
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__releaseKeyboard(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::releaseMouse()
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__releaseMouse(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::resize(KDDockWidgets::Size arg__1)
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__resize_Size(void *thisObj, void *arg__1_);
// KDDockWidgets::flutter::IndicatorWindow::rubberBand() const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__IndicatorWindow__rubberBand(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::setCursor(Qt::CursorShape shape)
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__setCursor_CursorShape(void *thisObj, int shape);
// KDDockWidgets::flutter::IndicatorWindow::setFixedHeight(int h)
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__setFixedHeight_int(void *thisObj, int h);
// KDDockWidgets::flutter::IndicatorWindow::setFixedWidth(int w)
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__setFixedWidth_int(void *thisObj, int w);
// KDDockWidgets::flutter::IndicatorWindow::setGeometry(KDDockWidgets::Rect arg__1)
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__setGeometry_Rect(void *thisObj, void *arg__1_);
// KDDockWidgets::flutter::IndicatorWindow::setHeight(int h)
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__setHeight_int(void *thisObj, int h);
// KDDockWidgets::flutter::IndicatorWindow::setMaximumSize(KDDockWidgets::Size sz)
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__setMaximumSize_Size(void *thisObj, void *sz_);
// KDDockWidgets::flutter::IndicatorWindow::setMinimumSize(KDDockWidgets::Size sz)
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__setMinimumSize_Size(void *thisObj, void *sz_);
// KDDockWidgets::flutter::IndicatorWindow::setMouseTracking(bool enable)
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__setMouseTracking_bool(void *thisObj, bool enable);
// KDDockWidgets::flutter::IndicatorWindow::setObjectName(const QString & arg__1)
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__setObjectName_QString(void *thisObj, const char *arg__1_);
// KDDockWidgets::flutter::IndicatorWindow::setParent(KDDockWidgets::Core::View * parent)
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__setParent_View(void *thisObj, void *parent_);
// KDDockWidgets::flutter::IndicatorWindow::setSize(int w, int h)
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__setSize_int_int(void *thisObj, int w, int h);
// KDDockWidgets::flutter::IndicatorWindow::setViewName(const QString & name)
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__setViewName_QString(void *thisObj, const char *name_);
// KDDockWidgets::flutter::IndicatorWindow::setVisible(bool arg__1)
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__setVisible_bool(void *thisObj, bool arg__1);
// KDDockWidgets::flutter::IndicatorWindow::setWidth(int w)
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__setWidth_int(void *thisObj, int w);
// KDDockWidgets::flutter::IndicatorWindow::setWindowOpacity(double v)
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__setWindowOpacity_double(void *thisObj, double v);
// KDDockWidgets::flutter::IndicatorWindow::setWindowTitle(const QString & title)
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__setWindowTitle_QString(void *thisObj, const char *title_);
// KDDockWidgets::flutter::IndicatorWindow::setZOrder(int z)
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__setZOrder_int(void *thisObj, int z);
// KDDockWidgets::flutter::IndicatorWindow::show()
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__show(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::showMaximized()
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__showMaximized(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::showMinimized()
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__showMinimized(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::showNormal()
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__showNormal(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::update()
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__update(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::updateIndicatorVisibility()
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__updateIndicatorVisibility(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::updatePositions()
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__updatePositions(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::updatePositions_flutter(int overlayWidth, int overlayHeight, KDDockWidgets::Core::Group * hoveredGroup, int visibleLocations)
DOCKS_EXPORT bool c_KDDockWidgets__flutter__IndicatorWindow__updatePositions_flutter_int_int_Group_int(void *thisObj, int overlayWidth, int overlayHeight, void *hoveredGroup_, int visibleLocations);
// KDDockWidgets::flutter::IndicatorWindow::viewName() const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__IndicatorWindow__viewName(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::zOrder() const
DOCKS_EXPORT int c_KDDockWidgets__flutter__IndicatorWindow__zOrder(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
DOCKS_EXPORT void c_KDDockWidgets__flutter__IndicatorWindow_Finalizer(void *cppObj);
}
