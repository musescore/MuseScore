/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <DockRegistry.h>
#include <core/DockWidget.h>
#include "core/MainWindow.h"
#include <FloatingWindow.h>
#include <core/Group.h>
#include <string_p.h>
#include <core/Layout.h>
#include <Item_p.h>
#include <SideBar.h>
#include <Object_p.h>

namespace KDDockWidgetsBindings_wrappersNS {
class DockRegistry_wrapper : public ::KDDockWidgets::DockRegistry
{
public:
    ~DockRegistry_wrapper();
    void clear();
    bool containsDockWidget(const QString &uniqueName) const;
    bool containsMainWindow(const QString &uniqueName) const;
    KDDockWidgets::Core::DockWidget *dockByName(const QString &arg__1) const;
    void ensureAllFloatingWidgetsAreMorphed();
    KDDockWidgets::Core::DockWidget *focusedDockWidget() const;
    KDDockWidgets::Core::Group *groupInMDIResize() const;
    bool hasFloatingWindows() const;
    bool isEmpty(bool excludeBeingDeleted = false) const;
    bool isSane() const;
    bool itemIsInMainWindow(const KDDockWidgets::Core::Item *arg__1) const;
    KDDockWidgets::Core::Layout *layoutForItem(const KDDockWidgets::Core::Item *arg__1) const;
    KDDockWidgets::Core::MainWindow *mainWindowByName(const QString &arg__1) const;
    void registerDockWidget(KDDockWidgets::Core::DockWidget *arg__1);
    void registerFloatingWindow(KDDockWidgets::Core::FloatingWindow *arg__1);
    void registerGroup(KDDockWidgets::Core::Group *arg__1);
    void registerLayoutSaver();
    void registerMainWindow(KDDockWidgets::Core::MainWindow *arg__1);
    static KDDockWidgets::DockRegistry *self();
    KDDockWidgets::Core::SideBar *sideBarForDockWidget(const KDDockWidgets::Core::DockWidget *arg__1) const;
    void unregisterDockWidget(KDDockWidgets::Core::DockWidget *arg__1);
    void unregisterFloatingWindow(KDDockWidgets::Core::FloatingWindow *arg__1);
    void unregisterGroup(KDDockWidgets::Core::Group *arg__1);
    void unregisterLayoutSaver();
    void unregisterMainWindow(KDDockWidgets::Core::MainWindow *arg__1);
};
}
extern "C" {
// KDDockWidgets::DockRegistry::clear()
DOCKS_EXPORT void c_KDDockWidgets__DockRegistry__clear(void *thisObj);
// KDDockWidgets::DockRegistry::containsDockWidget(const QString & uniqueName) const
DOCKS_EXPORT bool c_KDDockWidgets__DockRegistry__containsDockWidget_QString(void *thisObj, const char *uniqueName_);
// KDDockWidgets::DockRegistry::containsMainWindow(const QString & uniqueName) const
DOCKS_EXPORT bool c_KDDockWidgets__DockRegistry__containsMainWindow_QString(void *thisObj, const char *uniqueName_);
// KDDockWidgets::DockRegistry::dockByName(const QString & arg__1) const
DOCKS_EXPORT void *c_KDDockWidgets__DockRegistry__dockByName_QString(void *thisObj, const char *arg__1_);
// KDDockWidgets::DockRegistry::ensureAllFloatingWidgetsAreMorphed()
DOCKS_EXPORT void c_KDDockWidgets__DockRegistry__ensureAllFloatingWidgetsAreMorphed(void *thisObj);
// KDDockWidgets::DockRegistry::focusedDockWidget() const
DOCKS_EXPORT void *c_KDDockWidgets__DockRegistry__focusedDockWidget(void *thisObj);
// KDDockWidgets::DockRegistry::groupInMDIResize() const
DOCKS_EXPORT void *c_KDDockWidgets__DockRegistry__groupInMDIResize(void *thisObj);
// KDDockWidgets::DockRegistry::hasFloatingWindows() const
DOCKS_EXPORT bool c_KDDockWidgets__DockRegistry__hasFloatingWindows(void *thisObj);
// KDDockWidgets::DockRegistry::isEmpty(bool excludeBeingDeleted) const
DOCKS_EXPORT bool c_KDDockWidgets__DockRegistry__isEmpty_bool(void *thisObj, bool excludeBeingDeleted);
// KDDockWidgets::DockRegistry::isSane() const
DOCKS_EXPORT bool c_KDDockWidgets__DockRegistry__isSane(void *thisObj);
// KDDockWidgets::DockRegistry::itemIsInMainWindow(const KDDockWidgets::Core::Item * arg__1) const
DOCKS_EXPORT bool c_KDDockWidgets__DockRegistry__itemIsInMainWindow_Item(void *thisObj, void *arg__1_);
// KDDockWidgets::DockRegistry::layoutForItem(const KDDockWidgets::Core::Item * arg__1) const
DOCKS_EXPORT void *c_KDDockWidgets__DockRegistry__layoutForItem_Item(void *thisObj, void *arg__1_);
// KDDockWidgets::DockRegistry::mainWindowByName(const QString & arg__1) const
DOCKS_EXPORT void *c_KDDockWidgets__DockRegistry__mainWindowByName_QString(void *thisObj, const char *arg__1_);
// KDDockWidgets::DockRegistry::registerDockWidget(KDDockWidgets::Core::DockWidget * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__DockRegistry__registerDockWidget_DockWidget(void *thisObj, void *arg__1_);
// KDDockWidgets::DockRegistry::registerFloatingWindow(KDDockWidgets::Core::FloatingWindow * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__DockRegistry__registerFloatingWindow_FloatingWindow(void *thisObj, void *arg__1_);
// KDDockWidgets::DockRegistry::registerGroup(KDDockWidgets::Core::Group * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__DockRegistry__registerGroup_Group(void *thisObj, void *arg__1_);
// KDDockWidgets::DockRegistry::registerLayoutSaver()
DOCKS_EXPORT void c_KDDockWidgets__DockRegistry__registerLayoutSaver(void *thisObj);
// KDDockWidgets::DockRegistry::registerMainWindow(KDDockWidgets::Core::MainWindow * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__DockRegistry__registerMainWindow_MainWindow(void *thisObj, void *arg__1_);
// KDDockWidgets::DockRegistry::self()
DOCKS_EXPORT void *c_static_KDDockWidgets__DockRegistry__self();
// KDDockWidgets::DockRegistry::sideBarForDockWidget(const KDDockWidgets::Core::DockWidget * arg__1) const
DOCKS_EXPORT void *c_KDDockWidgets__DockRegistry__sideBarForDockWidget_DockWidget(void *thisObj, void *arg__1_);
// KDDockWidgets::DockRegistry::unregisterDockWidget(KDDockWidgets::Core::DockWidget * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__DockRegistry__unregisterDockWidget_DockWidget(void *thisObj, void *arg__1_);
// KDDockWidgets::DockRegistry::unregisterFloatingWindow(KDDockWidgets::Core::FloatingWindow * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__DockRegistry__unregisterFloatingWindow_FloatingWindow(void *thisObj, void *arg__1_);
// KDDockWidgets::DockRegistry::unregisterGroup(KDDockWidgets::Core::Group * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__DockRegistry__unregisterGroup_Group(void *thisObj, void *arg__1_);
// KDDockWidgets::DockRegistry::unregisterLayoutSaver()
DOCKS_EXPORT void c_KDDockWidgets__DockRegistry__unregisterLayoutSaver(void *thisObj);
// KDDockWidgets::DockRegistry::unregisterMainWindow(KDDockWidgets::Core::MainWindow * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__DockRegistry__unregisterMainWindow_MainWindow(void *thisObj, void *arg__1_);
DOCKS_EXPORT void c_KDDockWidgets__DockRegistry__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__DockRegistry__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
DOCKS_EXPORT void c_KDDockWidgets__DockRegistry_Finalizer(void *cppObj);
}
