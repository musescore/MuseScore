/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include "core/Stack.h"
#include <core/DockWidget.h>
#include <TabBar.h>
#include <core/Group.h>
#include <geometry_helpers_p.h>
#include <core/View.h>

namespace KDDockWidgetsBindings_wrappersNS {
namespace KDDWBindingsCore {
class Stack_wrapper : public ::KDDockWidgets::Core::Stack
{
public:
    ~Stack_wrapper();
    void addDockWidget(KDDockWidgets::Core::DockWidget *arg__1);
    bool contains(KDDockWidgets::Core::DockWidget *dw) const;
    KDDockWidgets::Core::Group *group() const;
    bool insertDockWidget(KDDockWidgets::Core::DockWidget *dockwidget, int index);
    virtual bool isMDI() const;
    virtual bool isMDI_nocallback() const;
    virtual bool isPositionDraggable(KDDockWidgets::Point p) const;
    virtual bool isPositionDraggable_nocallback(KDDockWidgets::Point p) const;
    virtual bool isWindow() const;
    virtual bool isWindow_nocallback() const;
    int numDockWidgets() const;
    bool onMouseDoubleClick(KDDockWidgets::Point localPos);
    void setDocumentMode(bool arg__1);
    virtual void setParentView_impl(KDDockWidgets::Core::View *parent);
    virtual void setParentView_impl_nocallback(KDDockWidgets::Core::View *parent);
    void setTabBarAutoHide(bool arg__1);
    KDDockWidgets::Core::TabBar *tabBar() const;
    bool tabBarAutoHide() const;
    typedef bool (*Callback_isMDI)(void *);
    Callback_isMDI m_isMDICallback = nullptr;
    typedef bool (*Callback_isPositionDraggable)(void *, KDDockWidgets::Point *p);
    Callback_isPositionDraggable m_isPositionDraggableCallback = nullptr;
    typedef bool (*Callback_isWindow)(void *);
    Callback_isWindow m_isWindowCallback = nullptr;
    typedef void (*Callback_setParentView_impl)(void *, KDDockWidgets::Core::View *parent);
    Callback_setParentView_impl m_setParentView_implCallback = nullptr;
    typedef KDDockWidgets::Core::DockWidget *(*Callback_singleDockWidget)(void *);
    Callback_singleDockWidget m_singleDockWidgetCallback = nullptr;
};
}
}
extern "C" {
// KDDockWidgets::Core::Stack::addDockWidget(KDDockWidgets::Core::DockWidget * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__Stack__addDockWidget_DockWidget(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Stack::contains(KDDockWidgets::Core::DockWidget * dw) const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Stack__contains_DockWidget(void *thisObj, void *dw_);
// KDDockWidgets::Core::Stack::group() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Stack__group(void *thisObj);
// KDDockWidgets::Core::Stack::insertDockWidget(KDDockWidgets::Core::DockWidget * dockwidget, int index)
DOCKS_EXPORT bool c_KDDockWidgets__Core__Stack__insertDockWidget_DockWidget_int(void *thisObj, void *dockwidget_, int index);
// KDDockWidgets::Core::Stack::isMDI() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Stack__isMDI(void *thisObj);
// KDDockWidgets::Core::Stack::isPositionDraggable(KDDockWidgets::Point p) const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Stack__isPositionDraggable_Point(void *thisObj, void *p_);
// KDDockWidgets::Core::Stack::isWindow() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Stack__isWindow(void *thisObj);
// KDDockWidgets::Core::Stack::numDockWidgets() const
DOCKS_EXPORT int c_KDDockWidgets__Core__Stack__numDockWidgets(void *thisObj);
// KDDockWidgets::Core::Stack::onMouseDoubleClick(KDDockWidgets::Point localPos)
DOCKS_EXPORT bool c_KDDockWidgets__Core__Stack__onMouseDoubleClick_Point(void *thisObj, void *localPos_);
// KDDockWidgets::Core::Stack::setDocumentMode(bool arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__Stack__setDocumentMode_bool(void *thisObj, bool arg__1);
// KDDockWidgets::Core::Stack::setParentView_impl(KDDockWidgets::Core::View * parent)
DOCKS_EXPORT void c_KDDockWidgets__Core__Stack__setParentView_impl_View(void *thisObj, void *parent_);
// KDDockWidgets::Core::Stack::setTabBarAutoHide(bool arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__Stack__setTabBarAutoHide_bool(void *thisObj, bool arg__1);
// KDDockWidgets::Core::Stack::singleDockWidget() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Stack__singleDockWidget(void *thisObj);
// KDDockWidgets::Core::Stack::tabBar() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Stack__tabBar(void *thisObj);
// KDDockWidgets::Core::Stack::tabBarAutoHide() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Stack__tabBarAutoHide(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__Stack__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__Stack__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
DOCKS_EXPORT void c_KDDockWidgets__Core__Stack_Finalizer(void *cppObj);
}
