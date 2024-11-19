/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "DockRegistry_c.h"


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
void DockRegistry_wrapper::clear()
{
    ::KDDockWidgets::DockRegistry::clear();
}
bool DockRegistry_wrapper::containsDockWidget(const QString &uniqueName) const
{
    return ::KDDockWidgets::DockRegistry::containsDockWidget(uniqueName);
}
bool DockRegistry_wrapper::containsMainWindow(const QString &uniqueName) const
{
    return ::KDDockWidgets::DockRegistry::containsMainWindow(uniqueName);
}
KDDockWidgets::Core::DockWidget *DockRegistry_wrapper::dockByName(const QString &arg__1) const
{
    return ::KDDockWidgets::DockRegistry::dockByName(arg__1);
}
void DockRegistry_wrapper::ensureAllFloatingWidgetsAreMorphed()
{
    ::KDDockWidgets::DockRegistry::ensureAllFloatingWidgetsAreMorphed();
}
KDDockWidgets::Core::DockWidget *DockRegistry_wrapper::focusedDockWidget() const
{
    return ::KDDockWidgets::DockRegistry::focusedDockWidget();
}
KDDockWidgets::Core::Group *DockRegistry_wrapper::groupInMDIResize() const
{
    return ::KDDockWidgets::DockRegistry::groupInMDIResize();
}
bool DockRegistry_wrapper::hasFloatingWindows() const
{
    return ::KDDockWidgets::DockRegistry::hasFloatingWindows();
}
bool DockRegistry_wrapper::isEmpty(bool excludeBeingDeleted) const
{
    return ::KDDockWidgets::DockRegistry::isEmpty(excludeBeingDeleted);
}
bool DockRegistry_wrapper::isSane() const
{
    return ::KDDockWidgets::DockRegistry::isSane();
}
bool DockRegistry_wrapper::itemIsInMainWindow(const KDDockWidgets::Core::Item *arg__1) const
{
    return ::KDDockWidgets::DockRegistry::itemIsInMainWindow(arg__1);
}
KDDockWidgets::Core::Layout *DockRegistry_wrapper::layoutForItem(const KDDockWidgets::Core::Item *arg__1) const
{
    return ::KDDockWidgets::DockRegistry::layoutForItem(arg__1);
}
KDDockWidgets::Core::MainWindow *DockRegistry_wrapper::mainWindowByName(const QString &arg__1) const
{
    return ::KDDockWidgets::DockRegistry::mainWindowByName(arg__1);
}
void DockRegistry_wrapper::registerDockWidget(KDDockWidgets::Core::DockWidget *arg__1)
{
    ::KDDockWidgets::DockRegistry::registerDockWidget(arg__1);
}
void DockRegistry_wrapper::registerFloatingWindow(KDDockWidgets::Core::FloatingWindow *arg__1)
{
    ::KDDockWidgets::DockRegistry::registerFloatingWindow(arg__1);
}
void DockRegistry_wrapper::registerGroup(KDDockWidgets::Core::Group *arg__1)
{
    ::KDDockWidgets::DockRegistry::registerGroup(arg__1);
}
void DockRegistry_wrapper::registerLayoutSaver()
{
    ::KDDockWidgets::DockRegistry::registerLayoutSaver();
}
void DockRegistry_wrapper::registerMainWindow(KDDockWidgets::Core::MainWindow *arg__1)
{
    ::KDDockWidgets::DockRegistry::registerMainWindow(arg__1);
}
KDDockWidgets::DockRegistry *DockRegistry_wrapper::self()
{
    return ::KDDockWidgets::DockRegistry::self();
}
KDDockWidgets::Core::SideBar *DockRegistry_wrapper::sideBarForDockWidget(const KDDockWidgets::Core::DockWidget *arg__1) const
{
    return ::KDDockWidgets::DockRegistry::sideBarForDockWidget(arg__1);
}
void DockRegistry_wrapper::unregisterDockWidget(KDDockWidgets::Core::DockWidget *arg__1)
{
    ::KDDockWidgets::DockRegistry::unregisterDockWidget(arg__1);
}
void DockRegistry_wrapper::unregisterFloatingWindow(KDDockWidgets::Core::FloatingWindow *arg__1)
{
    ::KDDockWidgets::DockRegistry::unregisterFloatingWindow(arg__1);
}
void DockRegistry_wrapper::unregisterGroup(KDDockWidgets::Core::Group *arg__1)
{
    ::KDDockWidgets::DockRegistry::unregisterGroup(arg__1);
}
void DockRegistry_wrapper::unregisterLayoutSaver()
{
    ::KDDockWidgets::DockRegistry::unregisterLayoutSaver();
}
void DockRegistry_wrapper::unregisterMainWindow(KDDockWidgets::Core::MainWindow *arg__1)
{
    ::KDDockWidgets::DockRegistry::unregisterMainWindow(arg__1);
}
DockRegistry_wrapper::~DockRegistry_wrapper()
{
}

}
static KDDockWidgets::DockRegistry *fromPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgets::DockRegistry *>(ptr);
}
static KDDockWidgetsBindings_wrappersNS::DockRegistry_wrapper *fromWrapperPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgetsBindings_wrappersNS::DockRegistry_wrapper *>(ptr);
}
extern "C" {
void c_KDDockWidgets__DockRegistry_Finalizer(void *cppObj)
{
    delete reinterpret_cast<KDDockWidgetsBindings_wrappersNS::DockRegistry_wrapper *>(cppObj);
} // clear()
void c_KDDockWidgets__DockRegistry__clear(void *thisObj)
{
    fromPtr(thisObj)->clear();
}
// containsDockWidget(const QString & uniqueName) const
bool c_KDDockWidgets__DockRegistry__containsDockWidget_QString(void *thisObj, const char *uniqueName_)
{
    const auto uniqueName = QString::fromUtf8(uniqueName_);
    const auto &result = fromPtr(thisObj)->containsDockWidget(uniqueName);
    free(( char * )uniqueName_);
    return result;
}
// containsMainWindow(const QString & uniqueName) const
bool c_KDDockWidgets__DockRegistry__containsMainWindow_QString(void *thisObj, const char *uniqueName_)
{
    const auto uniqueName = QString::fromUtf8(uniqueName_);
    const auto &result = fromPtr(thisObj)->containsMainWindow(uniqueName);
    free(( char * )uniqueName_);
    return result;
}
// dockByName(const QString & arg__1) const
void *c_KDDockWidgets__DockRegistry__dockByName_QString(void *thisObj, const char *arg__1_)
{
    const auto arg__1 = QString::fromUtf8(arg__1_);
    const auto &result = fromPtr(thisObj)->dockByName(arg__1);
    free(( char * )arg__1_);
    return result;
}
// ensureAllFloatingWidgetsAreMorphed()
void c_KDDockWidgets__DockRegistry__ensureAllFloatingWidgetsAreMorphed(void *thisObj)
{
    fromPtr(thisObj)->ensureAllFloatingWidgetsAreMorphed();
}
// focusedDockWidget() const
void *c_KDDockWidgets__DockRegistry__focusedDockWidget(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->focusedDockWidget();
    return result;
}
// groupInMDIResize() const
void *c_KDDockWidgets__DockRegistry__groupInMDIResize(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->groupInMDIResize();
    return result;
}
// hasFloatingWindows() const
bool c_KDDockWidgets__DockRegistry__hasFloatingWindows(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->hasFloatingWindows();
    return result;
}
// isEmpty(bool excludeBeingDeleted) const
bool c_KDDockWidgets__DockRegistry__isEmpty_bool(void *thisObj, bool excludeBeingDeleted)
{
    const auto &result = fromPtr(thisObj)->isEmpty(excludeBeingDeleted);
    return result;
}
// isSane() const
bool c_KDDockWidgets__DockRegistry__isSane(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->isSane();
    return result;
}
// itemIsInMainWindow(const KDDockWidgets::Core::Item * arg__1) const
bool c_KDDockWidgets__DockRegistry__itemIsInMainWindow_Item(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::Item *>(arg__1_);
    const auto &result = fromPtr(thisObj)->itemIsInMainWindow(arg__1);
    return result;
}
// layoutForItem(const KDDockWidgets::Core::Item * arg__1) const
void *c_KDDockWidgets__DockRegistry__layoutForItem_Item(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::Item *>(arg__1_);
    const auto &result = fromPtr(thisObj)->layoutForItem(arg__1);
    return result;
}
// mainWindowByName(const QString & arg__1) const
void *c_KDDockWidgets__DockRegistry__mainWindowByName_QString(void *thisObj, const char *arg__1_)
{
    const auto arg__1 = QString::fromUtf8(arg__1_);
    const auto &result = fromPtr(thisObj)->mainWindowByName(arg__1);
    free(( char * )arg__1_);
    return result;
}
// registerDockWidget(KDDockWidgets::Core::DockWidget * arg__1)
void c_KDDockWidgets__DockRegistry__registerDockWidget_DockWidget(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::DockWidget *>(arg__1_);
    fromPtr(thisObj)->registerDockWidget(arg__1);
}
// registerFloatingWindow(KDDockWidgets::Core::FloatingWindow * arg__1)
void c_KDDockWidgets__DockRegistry__registerFloatingWindow_FloatingWindow(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::FloatingWindow *>(arg__1_);
    fromPtr(thisObj)->registerFloatingWindow(arg__1);
}
// registerGroup(KDDockWidgets::Core::Group * arg__1)
void c_KDDockWidgets__DockRegistry__registerGroup_Group(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::Group *>(arg__1_);
    fromPtr(thisObj)->registerGroup(arg__1);
}
// registerLayoutSaver()
void c_KDDockWidgets__DockRegistry__registerLayoutSaver(void *thisObj)
{
    fromPtr(thisObj)->registerLayoutSaver();
}
// registerMainWindow(KDDockWidgets::Core::MainWindow * arg__1)
void c_KDDockWidgets__DockRegistry__registerMainWindow_MainWindow(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::MainWindow *>(arg__1_);
    fromPtr(thisObj)->registerMainWindow(arg__1);
}
// self()
void *c_static_KDDockWidgets__DockRegistry__self()
{
    const auto &result = KDDockWidgetsBindings_wrappersNS::DockRegistry_wrapper::self();
    return result;
}
// sideBarForDockWidget(const KDDockWidgets::Core::DockWidget * arg__1) const
void *c_KDDockWidgets__DockRegistry__sideBarForDockWidget_DockWidget(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::DockWidget *>(arg__1_);
    const auto &result = fromPtr(thisObj)->sideBarForDockWidget(arg__1);
    return result;
}
// unregisterDockWidget(KDDockWidgets::Core::DockWidget * arg__1)
void c_KDDockWidgets__DockRegistry__unregisterDockWidget_DockWidget(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::DockWidget *>(arg__1_);
    fromPtr(thisObj)->unregisterDockWidget(arg__1);
}
// unregisterFloatingWindow(KDDockWidgets::Core::FloatingWindow * arg__1)
void c_KDDockWidgets__DockRegistry__unregisterFloatingWindow_FloatingWindow(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::FloatingWindow *>(arg__1_);
    fromPtr(thisObj)->unregisterFloatingWindow(arg__1);
}
// unregisterGroup(KDDockWidgets::Core::Group * arg__1)
void c_KDDockWidgets__DockRegistry__unregisterGroup_Group(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::Group *>(arg__1_);
    fromPtr(thisObj)->unregisterGroup(arg__1);
}
// unregisterLayoutSaver()
void c_KDDockWidgets__DockRegistry__unregisterLayoutSaver(void *thisObj)
{
    fromPtr(thisObj)->unregisterLayoutSaver();
}
// unregisterMainWindow(KDDockWidgets::Core::MainWindow * arg__1)
void c_KDDockWidgets__DockRegistry__unregisterMainWindow_MainWindow(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::MainWindow *>(arg__1_);
    fromPtr(thisObj)->unregisterMainWindow(arg__1);
}
void c_KDDockWidgets__DockRegistry__destructor(void *thisObj)
{
    delete fromPtr(thisObj);
}
void c_KDDockWidgets__DockRegistry__registerVirtualMethodCallback(void *ptr, void *callback, int methodId)
{
    auto wrapper = fromWrapperPtr(ptr);
    switch (methodId) {
    }
}
}
