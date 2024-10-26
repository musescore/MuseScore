/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "MainWindow_c.h"


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
MainWindow_wrapper::MainWindow_wrapper(KDDockWidgets::Core::View *view, const QString &uniqueName, QFlags<KDDockWidgets::MainWindowOption> options)
    : ::KDDockWidgets::Core::MainWindow(view, uniqueName, options)
{
}
void MainWindow_wrapper::addDockWidget(KDDockWidgets::Core::DockWidget *dockWidget, KDDockWidgets::Location location, KDDockWidgets::Core::DockWidget *relativeTo, KDDockWidgets::InitialOption initialOption)
{
    ::KDDockWidgets::Core::MainWindow::addDockWidget(dockWidget, location, relativeTo, initialOption);
}
void MainWindow_wrapper::addDockWidgetAsTab(KDDockWidgets::Core::DockWidget *dockwidget)
{
    ::KDDockWidgets::Core::MainWindow::addDockWidgetAsTab(dockwidget);
}
void MainWindow_wrapper::addDockWidgetToSide(KDDockWidgets::Core::DockWidget *dockWidget, KDDockWidgets::Location location, KDDockWidgets::InitialOption initialOption)
{
    ::KDDockWidgets::Core::MainWindow::addDockWidgetToSide(dockWidget, location, initialOption);
}
bool MainWindow_wrapper::anySideBarIsVisible() const
{
    return ::KDDockWidgets::Core::MainWindow::anySideBarIsVisible();
}
KDDockWidgets::Margins MainWindow_wrapper::centerWidgetMargins() const
{
    return ::KDDockWidgets::Core::MainWindow::centerWidgetMargins();
}
KDDockWidgets::Rect MainWindow_wrapper::centralAreaGeometry() const
{
    return ::KDDockWidgets::Core::MainWindow::centralAreaGeometry();
}
void MainWindow_wrapper::clearSideBarOverlay(bool deleteGroup)
{
    ::KDDockWidgets::Core::MainWindow::clearSideBarOverlay(deleteGroup);
}
bool MainWindow_wrapper::closeDockWidgets(bool force)
{
    return ::KDDockWidgets::Core::MainWindow::closeDockWidgets(force);
}
KDDockWidgets::Core::DropArea *MainWindow_wrapper::dropArea() const
{
    return ::KDDockWidgets::Core::MainWindow::dropArea();
}
void MainWindow_wrapper::init(const QString &name)
{
    ::KDDockWidgets::Core::MainWindow::init(name);
}
bool MainWindow_wrapper::isMDI() const
{
    return ::KDDockWidgets::Core::MainWindow::isMDI();
}
KDDockWidgets::Core::Layout *MainWindow_wrapper::layout() const
{
    return ::KDDockWidgets::Core::MainWindow::layout();
}
void MainWindow_wrapper::layoutEqually()
{
    ::KDDockWidgets::Core::MainWindow::layoutEqually();
}
void MainWindow_wrapper::layoutParentContainerEqually(KDDockWidgets::Core::DockWidget *dockWidget)
{
    ::KDDockWidgets::Core::MainWindow::layoutParentContainerEqually(dockWidget);
}
void MainWindow_wrapper::moveToSideBar(KDDockWidgets::Core::DockWidget *dw)
{
    ::KDDockWidgets::Core::MainWindow::moveToSideBar(dw);
}
KDDockWidgets::Core::DropArea *MainWindow_wrapper::multiSplitter() const
{
    return ::KDDockWidgets::Core::MainWindow::multiSplitter();
}
QFlags<KDDockWidgets::MainWindowOption> MainWindow_wrapper::options() const
{
    return ::KDDockWidgets::Core::MainWindow::options();
}
int MainWindow_wrapper::overlayMargin() const
{
    return ::KDDockWidgets::Core::MainWindow::overlayMargin();
}
void MainWindow_wrapper::overlayOnSideBar(KDDockWidgets::Core::DockWidget *dw)
{
    ::KDDockWidgets::Core::MainWindow::overlayOnSideBar(dw);
}
KDDockWidgets::Core::DockWidget *MainWindow_wrapper::overlayedDockWidget() const
{
    return ::KDDockWidgets::Core::MainWindow::overlayedDockWidget();
}
void MainWindow_wrapper::restoreFromSideBar(KDDockWidgets::Core::DockWidget *dw)
{
    ::KDDockWidgets::Core::MainWindow::restoreFromSideBar(dw);
}
void MainWindow_wrapper::setContentsMargins(int l, int t, int r, int b)
{
    ::KDDockWidgets::Core::MainWindow::setContentsMargins(l, t, r, b);
}
void MainWindow_wrapper::setOverlayMargin(int margin)
{
    ::KDDockWidgets::Core::MainWindow::setOverlayMargin(margin);
}
void MainWindow_wrapper::setParentView_impl(KDDockWidgets::Core::View *parent)
{
    if (m_setParentView_implCallback) {
        const void *thisPtr = this;
        m_setParentView_implCallback(const_cast<void *>(thisPtr), parent);
    } else {
        ::KDDockWidgets::Core::MainWindow::setParentView_impl(parent);
    }
}
void MainWindow_wrapper::setParentView_impl_nocallback(KDDockWidgets::Core::View *parent)
{
    ::KDDockWidgets::Core::MainWindow::setParentView_impl(parent);
}
void MainWindow_wrapper::setUniqueName(const QString &uniqueName)
{
    ::KDDockWidgets::Core::MainWindow::setUniqueName(uniqueName);
}
KDDockWidgets::Core::SideBar *MainWindow_wrapper::sideBarForDockWidget(const KDDockWidgets::Core::DockWidget *dw) const
{
    return ::KDDockWidgets::Core::MainWindow::sideBarForDockWidget(dw);
}
void MainWindow_wrapper::toggleOverlayOnSideBar(KDDockWidgets::Core::DockWidget *dw)
{
    ::KDDockWidgets::Core::MainWindow::toggleOverlayOnSideBar(dw);
}
QString MainWindow_wrapper::uniqueName() const
{
    return ::KDDockWidgets::Core::MainWindow::uniqueName();
}
MainWindow_wrapper::~MainWindow_wrapper()
{
}

}
}
static KDDockWidgets::Core::MainWindow *fromPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgets::Core::MainWindow *>(ptr);
}
static KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::MainWindow_wrapper *fromWrapperPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::MainWindow_wrapper *>(ptr);
}
extern "C" {
void c_KDDockWidgets__Core__MainWindow_Finalizer(void *cppObj)
{
    delete reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::MainWindow_wrapper *>(cppObj);
}
void *c_KDDockWidgets__Core__MainWindow__constructor_View_QString_MainWindowOptions(void *view_, const char *uniqueName_, int options_)
{
    auto view = reinterpret_cast<KDDockWidgets::Core::View *>(view_);
    const auto uniqueName = QString::fromUtf8(uniqueName_);
    auto options = static_cast<QFlags<KDDockWidgets::MainWindowOption>>(options_);
    auto ptr = new KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::MainWindow_wrapper(view, uniqueName, options);
    return reinterpret_cast<void *>(ptr);
}
// addDockWidget(KDDockWidgets::Core::DockWidget * dockWidget, KDDockWidgets::Location location, KDDockWidgets::Core::DockWidget * relativeTo, KDDockWidgets::InitialOption initialOption)
void c_KDDockWidgets__Core__MainWindow__addDockWidget_DockWidget_Location_DockWidget_InitialOption(void *thisObj, void *dockWidget_, int location, void *relativeTo_, void *initialOption_)
{
    auto dockWidget = reinterpret_cast<KDDockWidgets::Core::DockWidget *>(dockWidget_);
    auto relativeTo = reinterpret_cast<KDDockWidgets::Core::DockWidget *>(relativeTo_);
    assert(initialOption_);
    auto &initialOption = *reinterpret_cast<KDDockWidgets::InitialOption *>(initialOption_);
    fromPtr(thisObj)->addDockWidget(dockWidget, static_cast<KDDockWidgets::Location>(location), relativeTo, initialOption);
}
// addDockWidgetAsTab(KDDockWidgets::Core::DockWidget * dockwidget)
void c_KDDockWidgets__Core__MainWindow__addDockWidgetAsTab_DockWidget(void *thisObj, void *dockwidget_)
{
    auto dockwidget = reinterpret_cast<KDDockWidgets::Core::DockWidget *>(dockwidget_);
    fromPtr(thisObj)->addDockWidgetAsTab(dockwidget);
}
// addDockWidgetToSide(KDDockWidgets::Core::DockWidget * dockWidget, KDDockWidgets::Location location, KDDockWidgets::InitialOption initialOption)
void c_KDDockWidgets__Core__MainWindow__addDockWidgetToSide_DockWidget_Location_InitialOption(void *thisObj, void *dockWidget_, int location, void *initialOption_)
{
    auto dockWidget = reinterpret_cast<KDDockWidgets::Core::DockWidget *>(dockWidget_);
    assert(initialOption_);
    auto &initialOption = *reinterpret_cast<KDDockWidgets::InitialOption *>(initialOption_);
    fromPtr(thisObj)->addDockWidgetToSide(dockWidget, static_cast<KDDockWidgets::Location>(location), initialOption);
}
// anySideBarIsVisible() const
bool c_KDDockWidgets__Core__MainWindow__anySideBarIsVisible(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->anySideBarIsVisible();
    return result;
}
// centerWidgetMargins() const
void *c_KDDockWidgets__Core__MainWindow__centerWidgetMargins(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Margins> { fromPtr(thisObj)->centerWidgetMargins() };
    return result;
}
// centralAreaGeometry() const
void *c_KDDockWidgets__Core__MainWindow__centralAreaGeometry(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Rect> { fromWrapperPtr(thisObj)->centralAreaGeometry() };
    return result;
}
// clearSideBarOverlay(bool deleteGroup)
void c_KDDockWidgets__Core__MainWindow__clearSideBarOverlay_bool(void *thisObj, bool deleteGroup)
{
    fromPtr(thisObj)->clearSideBarOverlay(deleteGroup);
}
// closeDockWidgets(bool force)
bool c_KDDockWidgets__Core__MainWindow__closeDockWidgets_bool(void *thisObj, bool force)
{
    const auto &result = fromPtr(thisObj)->closeDockWidgets(force);
    return result;
}
// dropArea() const
void *c_KDDockWidgets__Core__MainWindow__dropArea(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->dropArea();
    return result;
}
// init(const QString & name)
void c_KDDockWidgets__Core__MainWindow__init_QString(void *thisObj, const char *name_)
{
    const auto name = QString::fromUtf8(name_);
    fromPtr(thisObj)->init(name);
    free(( char * )name_);
}
// isMDI() const
bool c_KDDockWidgets__Core__MainWindow__isMDI(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->isMDI();
    return result;
}
// layout() const
void *c_KDDockWidgets__Core__MainWindow__layout(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->layout();
    return result;
}
// layoutEqually()
void c_KDDockWidgets__Core__MainWindow__layoutEqually(void *thisObj)
{
    fromPtr(thisObj)->layoutEqually();
}
// layoutParentContainerEqually(KDDockWidgets::Core::DockWidget * dockWidget)
void c_KDDockWidgets__Core__MainWindow__layoutParentContainerEqually_DockWidget(void *thisObj, void *dockWidget_)
{
    auto dockWidget = reinterpret_cast<KDDockWidgets::Core::DockWidget *>(dockWidget_);
    fromPtr(thisObj)->layoutParentContainerEqually(dockWidget);
}
// moveToSideBar(KDDockWidgets::Core::DockWidget * dw)
void c_KDDockWidgets__Core__MainWindow__moveToSideBar_DockWidget(void *thisObj, void *dw_)
{
    auto dw = reinterpret_cast<KDDockWidgets::Core::DockWidget *>(dw_);
    fromPtr(thisObj)->moveToSideBar(dw);
}
// multiSplitter() const
void *c_KDDockWidgets__Core__MainWindow__multiSplitter(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->multiSplitter();
    return result;
}
// options() const
int c_KDDockWidgets__Core__MainWindow__options(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->options();
    return result;
}
// overlayMargin() const
int c_KDDockWidgets__Core__MainWindow__overlayMargin(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->overlayMargin();
    return result;
}
// overlayOnSideBar(KDDockWidgets::Core::DockWidget * dw)
void c_KDDockWidgets__Core__MainWindow__overlayOnSideBar_DockWidget(void *thisObj, void *dw_)
{
    auto dw = reinterpret_cast<KDDockWidgets::Core::DockWidget *>(dw_);
    fromPtr(thisObj)->overlayOnSideBar(dw);
}
// overlayedDockWidget() const
void *c_KDDockWidgets__Core__MainWindow__overlayedDockWidget(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->overlayedDockWidget();
    return result;
}
// restoreFromSideBar(KDDockWidgets::Core::DockWidget * dw)
void c_KDDockWidgets__Core__MainWindow__restoreFromSideBar_DockWidget(void *thisObj, void *dw_)
{
    auto dw = reinterpret_cast<KDDockWidgets::Core::DockWidget *>(dw_);
    fromPtr(thisObj)->restoreFromSideBar(dw);
}
// setContentsMargins(int l, int t, int r, int b)
void c_KDDockWidgets__Core__MainWindow__setContentsMargins_int_int_int_int(void *thisObj, int l, int t, int r, int b)
{
    fromPtr(thisObj)->setContentsMargins(l, t, r, b);
}
// setOverlayMargin(int margin)
void c_KDDockWidgets__Core__MainWindow__setOverlayMargin_int(void *thisObj, int margin)
{
    fromPtr(thisObj)->setOverlayMargin(margin);
}
// setParentView_impl(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__Core__MainWindow__setParentView_impl_View(void *thisObj, void *parent_)
{
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    fromWrapperPtr(thisObj)->setParentView_impl_nocallback(parent);
}
// setUniqueName(const QString & uniqueName)
void c_KDDockWidgets__Core__MainWindow__setUniqueName_QString(void *thisObj, const char *uniqueName_)
{
    const auto uniqueName = QString::fromUtf8(uniqueName_);
    fromWrapperPtr(thisObj)->setUniqueName(uniqueName);
    free(( char * )uniqueName_);
}
// sideBarForDockWidget(const KDDockWidgets::Core::DockWidget * dw) const
void *c_KDDockWidgets__Core__MainWindow__sideBarForDockWidget_DockWidget(void *thisObj, void *dw_)
{
    auto dw = reinterpret_cast<KDDockWidgets::Core::DockWidget *>(dw_);
    const auto &result = fromPtr(thisObj)->sideBarForDockWidget(dw);
    return result;
}
// toggleOverlayOnSideBar(KDDockWidgets::Core::DockWidget * dw)
void c_KDDockWidgets__Core__MainWindow__toggleOverlayOnSideBar_DockWidget(void *thisObj, void *dw_)
{
    auto dw = reinterpret_cast<KDDockWidgets::Core::DockWidget *>(dw_);
    fromPtr(thisObj)->toggleOverlayOnSideBar(dw);
}
// uniqueName() const
void *c_KDDockWidgets__Core__MainWindow__uniqueName(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<QString> { fromPtr(thisObj)->uniqueName() };
    return result;
}
void c_KDDockWidgets__Core__MainWindow__destructor(void *thisObj)
{
    delete fromPtr(thisObj);
}
void c_KDDockWidgets__Core__MainWindow__registerVirtualMethodCallback(void *ptr, void *callback, int methodId)
{
    auto wrapper = fromWrapperPtr(ptr);
    switch (methodId) {
    case 331:
        wrapper->m_setParentView_implCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::MainWindow_wrapper::Callback_setParentView_impl>(callback);
        break;
    }
}
}
