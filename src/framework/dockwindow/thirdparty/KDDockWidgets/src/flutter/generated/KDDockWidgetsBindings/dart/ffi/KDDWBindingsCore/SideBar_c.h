/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <SideBar.h>
#include <core/DockWidget.h>
#include "core/MainWindow.h"
#include <core/View.h>
#include <geometry_helpers_p.h>

namespace KDDockWidgetsBindings_wrappersNS {
namespace KDDWBindingsCore {
class SideBar_wrapper : public ::KDDockWidgets::Core::SideBar
{
public:
    ~SideBar_wrapper();
    void addDockWidget(KDDockWidgets::Core::DockWidget *dw);
    void clear();
    bool containsDockWidget(KDDockWidgets::Core::DockWidget *arg__1) const;
    bool isEmpty() const;
    bool isVertical() const;
    KDDockWidgets::Core::MainWindow *mainWindow() const;
    void onButtonClicked(KDDockWidgets::Core::DockWidget *dw);
    void removeDockWidget(KDDockWidgets::Core::DockWidget *dw);
    virtual void setParentView_impl(KDDockWidgets::Core::View *parent);
    virtual void setParentView_impl_nocallback(KDDockWidgets::Core::View *parent);
    void toggleOverlay(KDDockWidgets::Core::DockWidget *arg__1);
    typedef void (*Callback_setParentView_impl)(void *, KDDockWidgets::Core::View *parent);
    Callback_setParentView_impl m_setParentView_implCallback = nullptr;
};
}
}
extern "C" {
// KDDockWidgets::Core::SideBar::addDockWidget(KDDockWidgets::Core::DockWidget * dw)
DOCKS_EXPORT void c_KDDockWidgets__Core__SideBar__addDockWidget_DockWidget(void *thisObj, void *dw_);
// KDDockWidgets::Core::SideBar::clear()
DOCKS_EXPORT void c_KDDockWidgets__Core__SideBar__clear(void *thisObj);
// KDDockWidgets::Core::SideBar::containsDockWidget(KDDockWidgets::Core::DockWidget * arg__1) const
DOCKS_EXPORT bool c_KDDockWidgets__Core__SideBar__containsDockWidget_DockWidget(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::SideBar::isEmpty() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__SideBar__isEmpty(void *thisObj);
// KDDockWidgets::Core::SideBar::isVertical() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__SideBar__isVertical(void *thisObj);
// KDDockWidgets::Core::SideBar::mainWindow() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__SideBar__mainWindow(void *thisObj);
// KDDockWidgets::Core::SideBar::onButtonClicked(KDDockWidgets::Core::DockWidget * dw)
DOCKS_EXPORT void c_KDDockWidgets__Core__SideBar__onButtonClicked_DockWidget(void *thisObj, void *dw_);
// KDDockWidgets::Core::SideBar::removeDockWidget(KDDockWidgets::Core::DockWidget * dw)
DOCKS_EXPORT void c_KDDockWidgets__Core__SideBar__removeDockWidget_DockWidget(void *thisObj, void *dw_);
// KDDockWidgets::Core::SideBar::setParentView_impl(KDDockWidgets::Core::View * parent)
DOCKS_EXPORT void c_KDDockWidgets__Core__SideBar__setParentView_impl_View(void *thisObj, void *parent_);
// KDDockWidgets::Core::SideBar::toggleOverlay(KDDockWidgets::Core::DockWidget * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__SideBar__toggleOverlay_DockWidget(void *thisObj, void *arg__1_);
DOCKS_EXPORT void c_KDDockWidgets__Core__SideBar__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__SideBar__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
DOCKS_EXPORT void c_KDDockWidgets__Core__SideBar_Finalizer(void *cppObj);
}
