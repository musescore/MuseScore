/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <ViewFactory.h>
#include <core/View.h>
#include <string_p.h>
#include <core/Group.h>
#include "core/TitleBar.h"
#include "core/Stack.h"
#include <TabBar.h>
#include <Separator.h>
#include <FloatingWindow.h>
#include "core/MainWindow.h"
#include <ClassicIndicatorWindowViewInterface.h>
#include <ClassicDropIndicatorOverlay.h>
#include "core/DropArea.h"
#include <SideBar.h>
#include <Object_p.h>

namespace KDDockWidgetsBindings_wrappersNS {
namespace KDDWBindingsCore {
class ViewFactory_wrapper : public ::KDDockWidgets::Core::ViewFactory
{
public:
    ~ViewFactory_wrapper();
    ViewFactory_wrapper();
    virtual KDDockWidgets::Core::ClassicIndicatorWindowViewInterface *createClassicIndicatorWindow(KDDockWidgets::Core::ClassicDropIndicatorOverlay *arg__1, KDDockWidgets::Core::View *parent = nullptr) const;
    virtual KDDockWidgets::Core::ClassicIndicatorWindowViewInterface *createClassicIndicatorWindow_nocallback(KDDockWidgets::Core::ClassicDropIndicatorOverlay *arg__1, KDDockWidgets::Core::View *parent = nullptr) const;
    virtual KDDockWidgets::Core::View *createDockWidget(const QString &uniqueName, QFlags<KDDockWidgets::DockWidgetOption> options = {}, QFlags<KDDockWidgets::LayoutSaverOption> layoutSaverOptions = {}, Qt::WindowFlags windowFlags = {}) const;
    virtual KDDockWidgets::Core::View *createDockWidget_nocallback(const QString &uniqueName, QFlags<KDDockWidgets::DockWidgetOption> options = {}, QFlags<KDDockWidgets::LayoutSaverOption> layoutSaverOptions = {}, Qt::WindowFlags windowFlags = {}) const;
    virtual KDDockWidgets::Core::View *createDropArea(KDDockWidgets::Core::DropArea *arg__1, KDDockWidgets::Core::View *parent) const;
    virtual KDDockWidgets::Core::View *createDropArea_nocallback(KDDockWidgets::Core::DropArea *arg__1, KDDockWidgets::Core::View *parent) const;
    virtual KDDockWidgets::Core::View *createFloatingWindow(KDDockWidgets::Core::FloatingWindow *controller, KDDockWidgets::Core::MainWindow *parent = nullptr, Qt::WindowFlags windowFlags = {}) const;
    virtual KDDockWidgets::Core::View *createFloatingWindow_nocallback(KDDockWidgets::Core::FloatingWindow *controller, KDDockWidgets::Core::MainWindow *parent = nullptr, Qt::WindowFlags windowFlags = {}) const;
    virtual KDDockWidgets::Core::View *createGroup(KDDockWidgets::Core::Group *arg__1, KDDockWidgets::Core::View *parent = nullptr) const;
    virtual KDDockWidgets::Core::View *createGroup_nocallback(KDDockWidgets::Core::Group *arg__1, KDDockWidgets::Core::View *parent = nullptr) const;
    virtual KDDockWidgets::Core::View *createRubberBand(KDDockWidgets::Core::View *parent) const;
    virtual KDDockWidgets::Core::View *createRubberBand_nocallback(KDDockWidgets::Core::View *parent) const;
    virtual KDDockWidgets::Core::View *createSeparator(KDDockWidgets::Core::Separator *arg__1, KDDockWidgets::Core::View *parent = nullptr) const;
    virtual KDDockWidgets::Core::View *createSeparator_nocallback(KDDockWidgets::Core::Separator *arg__1, KDDockWidgets::Core::View *parent = nullptr) const;
    virtual KDDockWidgets::Core::View *createSideBar(KDDockWidgets::Core::SideBar *arg__1, KDDockWidgets::Core::View *parent) const;
    virtual KDDockWidgets::Core::View *createSideBar_nocallback(KDDockWidgets::Core::SideBar *arg__1, KDDockWidgets::Core::View *parent) const;
    virtual KDDockWidgets::Core::View *createStack(KDDockWidgets::Core::Stack *stack, KDDockWidgets::Core::View *parent) const;
    virtual KDDockWidgets::Core::View *createStack_nocallback(KDDockWidgets::Core::Stack *stack, KDDockWidgets::Core::View *parent) const;
    virtual KDDockWidgets::Core::View *createTabBar(KDDockWidgets::Core::TabBar *tabBar, KDDockWidgets::Core::View *parent = nullptr) const;
    virtual KDDockWidgets::Core::View *createTabBar_nocallback(KDDockWidgets::Core::TabBar *tabBar, KDDockWidgets::Core::View *parent = nullptr) const;
    virtual KDDockWidgets::Core::View *createTitleBar(KDDockWidgets::Core::TitleBar *controller, KDDockWidgets::Core::View *parent) const;
    virtual KDDockWidgets::Core::View *createTitleBar_nocallback(KDDockWidgets::Core::TitleBar *controller, KDDockWidgets::Core::View *parent) const;
    typedef KDDockWidgets::Core::ClassicIndicatorWindowViewInterface *(*Callback_createClassicIndicatorWindow)(void *, KDDockWidgets::Core::ClassicDropIndicatorOverlay *arg__1, KDDockWidgets::Core::View *parent);
    Callback_createClassicIndicatorWindow m_createClassicIndicatorWindowCallback = nullptr;
    typedef KDDockWidgets::Core::View *(*Callback_createDockWidget)(void *, const QString &uniqueName, QFlags<KDDockWidgets::DockWidgetOption> options, QFlags<KDDockWidgets::LayoutSaverOption> layoutSaverOptions, Qt::WindowFlags windowFlags);
    Callback_createDockWidget m_createDockWidgetCallback = nullptr;
    typedef KDDockWidgets::Core::View *(*Callback_createDropArea)(void *, KDDockWidgets::Core::DropArea *arg__1, KDDockWidgets::Core::View *parent);
    Callback_createDropArea m_createDropAreaCallback = nullptr;
    typedef KDDockWidgets::Core::View *(*Callback_createFloatingWindow)(void *, KDDockWidgets::Core::FloatingWindow *controller, KDDockWidgets::Core::MainWindow *parent, Qt::WindowFlags windowFlags);
    Callback_createFloatingWindow m_createFloatingWindowCallback = nullptr;
    typedef KDDockWidgets::Core::View *(*Callback_createGroup)(void *, KDDockWidgets::Core::Group *arg__1, KDDockWidgets::Core::View *parent);
    Callback_createGroup m_createGroupCallback = nullptr;
    typedef KDDockWidgets::Core::View *(*Callback_createRubberBand)(void *, KDDockWidgets::Core::View *parent);
    Callback_createRubberBand m_createRubberBandCallback = nullptr;
    typedef KDDockWidgets::Core::View *(*Callback_createSeparator)(void *, KDDockWidgets::Core::Separator *arg__1, KDDockWidgets::Core::View *parent);
    Callback_createSeparator m_createSeparatorCallback = nullptr;
    typedef KDDockWidgets::Core::View *(*Callback_createSideBar)(void *, KDDockWidgets::Core::SideBar *arg__1, KDDockWidgets::Core::View *parent);
    Callback_createSideBar m_createSideBarCallback = nullptr;
    typedef KDDockWidgets::Core::View *(*Callback_createStack)(void *, KDDockWidgets::Core::Stack *stack, KDDockWidgets::Core::View *parent);
    Callback_createStack m_createStackCallback = nullptr;
    typedef KDDockWidgets::Core::View *(*Callback_createTabBar)(void *, KDDockWidgets::Core::TabBar *tabBar, KDDockWidgets::Core::View *parent);
    Callback_createTabBar m_createTabBarCallback = nullptr;
    typedef KDDockWidgets::Core::View *(*Callback_createTitleBar)(void *, KDDockWidgets::Core::TitleBar *controller, KDDockWidgets::Core::View *parent);
    Callback_createTitleBar m_createTitleBarCallback = nullptr;
};
}
}
extern "C" {
// KDDockWidgets::Core::ViewFactory::ViewFactory()
DOCKS_EXPORT void *c_KDDockWidgets__Core__ViewFactory__constructor();
// KDDockWidgets::Core::ViewFactory::createClassicIndicatorWindow(KDDockWidgets::Core::ClassicDropIndicatorOverlay * arg__1, KDDockWidgets::Core::View * parent) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__ViewFactory__createClassicIndicatorWindow_ClassicDropIndicatorOverlay_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::Core::ViewFactory::createDockWidget(const QString & uniqueName, QFlags<KDDockWidgets::DockWidgetOption> options, QFlags<KDDockWidgets::LayoutSaverOption> layoutSaverOptions, Qt::WindowFlags windowFlags) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__ViewFactory__createDockWidget_QString_DockWidgetOptions_LayoutSaverOptions_WindowFlags(void *thisObj, const char *uniqueName_, int options_, int layoutSaverOptions_, int windowFlags);
// KDDockWidgets::Core::ViewFactory::createDropArea(KDDockWidgets::Core::DropArea * arg__1, KDDockWidgets::Core::View * parent) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__ViewFactory__createDropArea_DropArea_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::Core::ViewFactory::createFloatingWindow(KDDockWidgets::Core::FloatingWindow * controller, KDDockWidgets::Core::MainWindow * parent, Qt::WindowFlags windowFlags) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__ViewFactory__createFloatingWindow_FloatingWindow_MainWindow_WindowFlags(void *thisObj, void *controller_, void *parent_, int windowFlags);
// KDDockWidgets::Core::ViewFactory::createGroup(KDDockWidgets::Core::Group * arg__1, KDDockWidgets::Core::View * parent) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__ViewFactory__createGroup_Group_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::Core::ViewFactory::createRubberBand(KDDockWidgets::Core::View * parent) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__ViewFactory__createRubberBand_View(void *thisObj, void *parent_);
// KDDockWidgets::Core::ViewFactory::createSeparator(KDDockWidgets::Core::Separator * arg__1, KDDockWidgets::Core::View * parent) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__ViewFactory__createSeparator_Separator_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::Core::ViewFactory::createSideBar(KDDockWidgets::Core::SideBar * arg__1, KDDockWidgets::Core::View * parent) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__ViewFactory__createSideBar_SideBar_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::Core::ViewFactory::createStack(KDDockWidgets::Core::Stack * stack, KDDockWidgets::Core::View * parent) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__ViewFactory__createStack_Stack_View(void *thisObj, void *stack_, void *parent_);
// KDDockWidgets::Core::ViewFactory::createTabBar(KDDockWidgets::Core::TabBar * tabBar, KDDockWidgets::Core::View * parent) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__ViewFactory__createTabBar_TabBar_View(void *thisObj, void *tabBar_, void *parent_);
// KDDockWidgets::Core::ViewFactory::createTitleBar(KDDockWidgets::Core::TitleBar * controller, KDDockWidgets::Core::View * parent) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__ViewFactory__createTitleBar_TitleBar_View(void *thisObj, void *controller_, void *parent_);
DOCKS_EXPORT void c_KDDockWidgets__Core__ViewFactory__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__ViewFactory__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
DOCKS_EXPORT void c_KDDockWidgets__Core__ViewFactory_Finalizer(void *cppObj);
}
