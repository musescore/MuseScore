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
#include <SideBar.h>
#include "core/DropArea.h"
#include <ClassicIndicatorWindowViewInterface.h>
#include <ClassicDropIndicatorOverlay.h>
#include <ClassicIndicatorsWindow.h>

namespace KDDockWidgetsBindings_wrappersNS {
namespace KDDWBindingsFlutter {
class ViewFactory_wrapper : public ::KDDockWidgets::flutter::ViewFactory
{
public:
    ~ViewFactory_wrapper();
    ViewFactory_wrapper();
    virtual KDDockWidgets::Core::ClassicIndicatorWindowViewInterface *createClassicIndicatorWindow(KDDockWidgets::Core::ClassicDropIndicatorOverlay *arg__1, KDDockWidgets::Core::View *parent = 0) const;
    virtual KDDockWidgets::Core::ClassicIndicatorWindowViewInterface *createClassicIndicatorWindow_nocallback(KDDockWidgets::Core::ClassicDropIndicatorOverlay *arg__1, KDDockWidgets::Core::View *parent = 0) const;
    virtual KDDockWidgets::flutter::IndicatorWindow *createClassicIndicatorWindow_flutter(KDDockWidgets::Core::ClassicDropIndicatorOverlay *arg__1, KDDockWidgets::Core::View *parent = 0) const;
    virtual KDDockWidgets::flutter::IndicatorWindow *createClassicIndicatorWindow_flutter_nocallback(KDDockWidgets::Core::ClassicDropIndicatorOverlay *arg__1, KDDockWidgets::Core::View *parent = 0) const;
    virtual KDDockWidgets::Core::View *createDockWidget(const QString &uniqueName, QFlags<KDDockWidgets::DockWidgetOption> arg__2 = {}, QFlags<KDDockWidgets::LayoutSaverOption> arg__3 = {}, Qt::WindowFlags arg__4 = {}) const;
    virtual KDDockWidgets::Core::View *createDockWidget_nocallback(const QString &uniqueName, QFlags<KDDockWidgets::DockWidgetOption> arg__2 = {}, QFlags<KDDockWidgets::LayoutSaverOption> arg__3 = {}, Qt::WindowFlags arg__4 = {}) const;
    virtual KDDockWidgets::Core::View *createDropArea(KDDockWidgets::Core::DropArea *arg__1, KDDockWidgets::Core::View *parent) const;
    virtual KDDockWidgets::Core::View *createDropArea_nocallback(KDDockWidgets::Core::DropArea *arg__1, KDDockWidgets::Core::View *parent) const;
    virtual KDDockWidgets::Core::View *createFloatingWindow(KDDockWidgets::Core::FloatingWindow *arg__1, KDDockWidgets::Core::MainWindow *parent = nullptr, Qt::WindowFlags windowFlags = {}) const;
    virtual KDDockWidgets::Core::View *createFloatingWindow_nocallback(KDDockWidgets::Core::FloatingWindow *arg__1, KDDockWidgets::Core::MainWindow *parent = nullptr, Qt::WindowFlags windowFlags = {}) const;
    virtual KDDockWidgets::Core::View *createGroup(KDDockWidgets::Core::Group *arg__1, KDDockWidgets::Core::View *parent = nullptr) const;
    virtual KDDockWidgets::Core::View *createGroup_nocallback(KDDockWidgets::Core::Group *arg__1, KDDockWidgets::Core::View *parent = nullptr) const;
    virtual KDDockWidgets::Core::View *createRubberBand(KDDockWidgets::Core::View *parent) const;
    virtual KDDockWidgets::Core::View *createRubberBand_nocallback(KDDockWidgets::Core::View *parent) const;
    virtual KDDockWidgets::Core::View *createSeparator(KDDockWidgets::Core::Separator *arg__1, KDDockWidgets::Core::View *parent = nullptr) const;
    virtual KDDockWidgets::Core::View *createSeparator_nocallback(KDDockWidgets::Core::Separator *arg__1, KDDockWidgets::Core::View *parent = nullptr) const;
    virtual KDDockWidgets::Core::View *createSideBar(KDDockWidgets::Core::SideBar *arg__1, KDDockWidgets::Core::View *parent) const;
    virtual KDDockWidgets::Core::View *createSideBar_nocallback(KDDockWidgets::Core::SideBar *arg__1, KDDockWidgets::Core::View *parent) const;
    virtual KDDockWidgets::Core::View *createStack(KDDockWidgets::Core::Stack *arg__1, KDDockWidgets::Core::View *parent) const;
    virtual KDDockWidgets::Core::View *createStack_nocallback(KDDockWidgets::Core::Stack *arg__1, KDDockWidgets::Core::View *parent) const;
    virtual KDDockWidgets::Core::View *createTabBar(KDDockWidgets::Core::TabBar *tabBar, KDDockWidgets::Core::View *parent = nullptr) const;
    virtual KDDockWidgets::Core::View *createTabBar_nocallback(KDDockWidgets::Core::TabBar *tabBar, KDDockWidgets::Core::View *parent = nullptr) const;
    virtual KDDockWidgets::Core::View *createTitleBar(KDDockWidgets::Core::TitleBar *arg__1, KDDockWidgets::Core::View *parent) const;
    virtual KDDockWidgets::Core::View *createTitleBar_nocallback(KDDockWidgets::Core::TitleBar *arg__1, KDDockWidgets::Core::View *parent) const;
    typedef KDDockWidgets::Core::ClassicIndicatorWindowViewInterface *(*Callback_createClassicIndicatorWindow)(void *, KDDockWidgets::Core::ClassicDropIndicatorOverlay *arg__1, KDDockWidgets::Core::View *parent);
    Callback_createClassicIndicatorWindow m_createClassicIndicatorWindowCallback = nullptr;
    typedef KDDockWidgets::flutter::IndicatorWindow *(*Callback_createClassicIndicatorWindow_flutter)(void *, KDDockWidgets::Core::ClassicDropIndicatorOverlay *arg__1, KDDockWidgets::Core::View *parent);
    Callback_createClassicIndicatorWindow_flutter m_createClassicIndicatorWindow_flutterCallback = nullptr;
    typedef KDDockWidgets::Core::View *(*Callback_createDockWidget)(void *, const QString &uniqueName, QFlags<KDDockWidgets::DockWidgetOption> arg__2, QFlags<KDDockWidgets::LayoutSaverOption> arg__3, Qt::WindowFlags arg__4);
    Callback_createDockWidget m_createDockWidgetCallback = nullptr;
    typedef KDDockWidgets::Core::View *(*Callback_createDropArea)(void *, KDDockWidgets::Core::DropArea *arg__1, KDDockWidgets::Core::View *parent);
    Callback_createDropArea m_createDropAreaCallback = nullptr;
    typedef KDDockWidgets::Core::View *(*Callback_createFloatingWindow)(void *, KDDockWidgets::Core::FloatingWindow *arg__1, KDDockWidgets::Core::MainWindow *parent, Qt::WindowFlags windowFlags);
    Callback_createFloatingWindow m_createFloatingWindowCallback = nullptr;
    typedef KDDockWidgets::Core::View *(*Callback_createGroup)(void *, KDDockWidgets::Core::Group *arg__1, KDDockWidgets::Core::View *parent);
    Callback_createGroup m_createGroupCallback = nullptr;
    typedef KDDockWidgets::Core::View *(*Callback_createRubberBand)(void *, KDDockWidgets::Core::View *parent);
    Callback_createRubberBand m_createRubberBandCallback = nullptr;
    typedef KDDockWidgets::Core::View *(*Callback_createSeparator)(void *, KDDockWidgets::Core::Separator *arg__1, KDDockWidgets::Core::View *parent);
    Callback_createSeparator m_createSeparatorCallback = nullptr;
    typedef KDDockWidgets::Core::View *(*Callback_createSideBar)(void *, KDDockWidgets::Core::SideBar *arg__1, KDDockWidgets::Core::View *parent);
    Callback_createSideBar m_createSideBarCallback = nullptr;
    typedef KDDockWidgets::Core::View *(*Callback_createStack)(void *, KDDockWidgets::Core::Stack *arg__1, KDDockWidgets::Core::View *parent);
    Callback_createStack m_createStackCallback = nullptr;
    typedef KDDockWidgets::Core::View *(*Callback_createTabBar)(void *, KDDockWidgets::Core::TabBar *tabBar, KDDockWidgets::Core::View *parent);
    Callback_createTabBar m_createTabBarCallback = nullptr;
    typedef KDDockWidgets::Core::View *(*Callback_createTitleBar)(void *, KDDockWidgets::Core::TitleBar *arg__1, KDDockWidgets::Core::View *parent);
    Callback_createTitleBar m_createTitleBarCallback = nullptr;
};
}
}
extern "C" {
// KDDockWidgets::flutter::ViewFactory::ViewFactory()
DOCKS_EXPORT void *c_KDDockWidgets__flutter__ViewFactory__constructor();
// KDDockWidgets::flutter::ViewFactory::createClassicIndicatorWindow(KDDockWidgets::Core::ClassicDropIndicatorOverlay * arg__1, KDDockWidgets::Core::View * parent) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__ViewFactory__createClassicIndicatorWindow_ClassicDropIndicatorOverlay_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::flutter::ViewFactory::createClassicIndicatorWindow_flutter(KDDockWidgets::Core::ClassicDropIndicatorOverlay * arg__1, KDDockWidgets::Core::View * parent) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__ViewFactory__createClassicIndicatorWindow_flutter_ClassicDropIndicatorOverlay_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::flutter::ViewFactory::createDockWidget(const QString & uniqueName, QFlags<KDDockWidgets::DockWidgetOption> arg__2, QFlags<KDDockWidgets::LayoutSaverOption> arg__3, Qt::WindowFlags arg__4) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__ViewFactory__createDockWidget_QString_DockWidgetOptions_LayoutSaverOptions_WindowFlags(void *thisObj, const char *uniqueName_, int arg__2_, int arg__3_, int arg__4);
// KDDockWidgets::flutter::ViewFactory::createDropArea(KDDockWidgets::Core::DropArea * arg__1, KDDockWidgets::Core::View * parent) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__ViewFactory__createDropArea_DropArea_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::flutter::ViewFactory::createFloatingWindow(KDDockWidgets::Core::FloatingWindow * arg__1, KDDockWidgets::Core::MainWindow * parent, Qt::WindowFlags windowFlags) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__ViewFactory__createFloatingWindow_FloatingWindow_MainWindow_WindowFlags(void *thisObj, void *arg__1_, void *parent_, int windowFlags);
// KDDockWidgets::flutter::ViewFactory::createGroup(KDDockWidgets::Core::Group * arg__1, KDDockWidgets::Core::View * parent) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__ViewFactory__createGroup_Group_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::flutter::ViewFactory::createRubberBand(KDDockWidgets::Core::View * parent) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__ViewFactory__createRubberBand_View(void *thisObj, void *parent_);
// KDDockWidgets::flutter::ViewFactory::createSeparator(KDDockWidgets::Core::Separator * arg__1, KDDockWidgets::Core::View * parent) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__ViewFactory__createSeparator_Separator_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::flutter::ViewFactory::createSideBar(KDDockWidgets::Core::SideBar * arg__1, KDDockWidgets::Core::View * parent) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__ViewFactory__createSideBar_SideBar_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::flutter::ViewFactory::createStack(KDDockWidgets::Core::Stack * arg__1, KDDockWidgets::Core::View * parent) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__ViewFactory__createStack_Stack_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::flutter::ViewFactory::createTabBar(KDDockWidgets::Core::TabBar * tabBar, KDDockWidgets::Core::View * parent) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__ViewFactory__createTabBar_TabBar_View(void *thisObj, void *tabBar_, void *parent_);
// KDDockWidgets::flutter::ViewFactory::createTitleBar(KDDockWidgets::Core::TitleBar * arg__1, KDDockWidgets::Core::View * parent) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__ViewFactory__createTitleBar_TitleBar_View(void *thisObj, void *arg__1_, void *parent_);
DOCKS_EXPORT void c_KDDockWidgets__flutter__ViewFactory__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__flutter__ViewFactory__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
DOCKS_EXPORT void c_KDDockWidgets__flutter__ViewFactory_Finalizer(void *cppObj);
}
