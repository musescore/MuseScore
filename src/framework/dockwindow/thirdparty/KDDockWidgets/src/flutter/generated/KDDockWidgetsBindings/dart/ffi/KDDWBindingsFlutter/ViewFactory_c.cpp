/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "ViewFactory_c.h"


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
namespace KDDWBindingsFlutter {
ViewFactory_wrapper::ViewFactory_wrapper()
    : ::KDDockWidgets::flutter::ViewFactory()
{
}
KDDockWidgets::Core::ClassicIndicatorWindowViewInterface *ViewFactory_wrapper::createClassicIndicatorWindow(KDDockWidgets::Core::ClassicDropIndicatorOverlay *arg__1, KDDockWidgets::Core::View *parent) const
{
    if (m_createClassicIndicatorWindowCallback) {
        const void *thisPtr = this;
        return m_createClassicIndicatorWindowCallback(const_cast<void *>(thisPtr), arg__1, parent);
    } else {
        return ::KDDockWidgets::flutter::ViewFactory::createClassicIndicatorWindow(arg__1, parent);
    }
}
KDDockWidgets::Core::ClassicIndicatorWindowViewInterface *ViewFactory_wrapper::createClassicIndicatorWindow_nocallback(KDDockWidgets::Core::ClassicDropIndicatorOverlay *arg__1, KDDockWidgets::Core::View *parent) const
{
    return ::KDDockWidgets::flutter::ViewFactory::createClassicIndicatorWindow(arg__1, parent);
}
KDDockWidgets::flutter::IndicatorWindow *ViewFactory_wrapper::createClassicIndicatorWindow_flutter(KDDockWidgets::Core::ClassicDropIndicatorOverlay *arg__1, KDDockWidgets::Core::View *parent) const
{
    if (m_createClassicIndicatorWindow_flutterCallback) {
        const void *thisPtr = this;
        return m_createClassicIndicatorWindow_flutterCallback(const_cast<void *>(thisPtr), arg__1, parent);
    } else {
        return ::KDDockWidgets::flutter::ViewFactory::createClassicIndicatorWindow_flutter(arg__1, parent);
    }
}
KDDockWidgets::flutter::IndicatorWindow *ViewFactory_wrapper::createClassicIndicatorWindow_flutter_nocallback(KDDockWidgets::Core::ClassicDropIndicatorOverlay *arg__1, KDDockWidgets::Core::View *parent) const
{
    return ::KDDockWidgets::flutter::ViewFactory::createClassicIndicatorWindow_flutter(arg__1, parent);
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createDockWidget(const QString &uniqueName, QFlags<KDDockWidgets::DockWidgetOption> arg__2, QFlags<KDDockWidgets::LayoutSaverOption> arg__3, Qt::WindowFlags arg__4) const
{
    if (m_createDockWidgetCallback) {
        const void *thisPtr = this;
        return m_createDockWidgetCallback(const_cast<void *>(thisPtr), uniqueName, arg__2, arg__3, arg__4);
    } else {
        return ::KDDockWidgets::flutter::ViewFactory::createDockWidget(uniqueName, arg__2, arg__3, arg__4);
    }
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createDockWidget_nocallback(const QString &uniqueName, QFlags<KDDockWidgets::DockWidgetOption> arg__2, QFlags<KDDockWidgets::LayoutSaverOption> arg__3, Qt::WindowFlags arg__4) const
{
    return ::KDDockWidgets::flutter::ViewFactory::createDockWidget(uniqueName, arg__2, arg__3, arg__4);
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createDropArea(KDDockWidgets::Core::DropArea *arg__1, KDDockWidgets::Core::View *parent) const
{
    if (m_createDropAreaCallback) {
        const void *thisPtr = this;
        return m_createDropAreaCallback(const_cast<void *>(thisPtr), arg__1, parent);
    } else {
        return ::KDDockWidgets::flutter::ViewFactory::createDropArea(arg__1, parent);
    }
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createDropArea_nocallback(KDDockWidgets::Core::DropArea *arg__1, KDDockWidgets::Core::View *parent) const
{
    return ::KDDockWidgets::flutter::ViewFactory::createDropArea(arg__1, parent);
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createFloatingWindow(KDDockWidgets::Core::FloatingWindow *arg__1, KDDockWidgets::Core::MainWindow *parent, Qt::WindowFlags windowFlags) const
{
    if (m_createFloatingWindowCallback) {
        const void *thisPtr = this;
        return m_createFloatingWindowCallback(const_cast<void *>(thisPtr), arg__1, parent, windowFlags);
    } else {
        return ::KDDockWidgets::flutter::ViewFactory::createFloatingWindow(arg__1, parent, windowFlags);
    }
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createFloatingWindow_nocallback(KDDockWidgets::Core::FloatingWindow *arg__1, KDDockWidgets::Core::MainWindow *parent, Qt::WindowFlags windowFlags) const
{
    return ::KDDockWidgets::flutter::ViewFactory::createFloatingWindow(arg__1, parent, windowFlags);
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createGroup(KDDockWidgets::Core::Group *arg__1, KDDockWidgets::Core::View *parent) const
{
    if (m_createGroupCallback) {
        const void *thisPtr = this;
        return m_createGroupCallback(const_cast<void *>(thisPtr), arg__1, parent);
    } else {
        return ::KDDockWidgets::flutter::ViewFactory::createGroup(arg__1, parent);
    }
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createGroup_nocallback(KDDockWidgets::Core::Group *arg__1, KDDockWidgets::Core::View *parent) const
{
    return ::KDDockWidgets::flutter::ViewFactory::createGroup(arg__1, parent);
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createRubberBand(KDDockWidgets::Core::View *parent) const
{
    if (m_createRubberBandCallback) {
        const void *thisPtr = this;
        return m_createRubberBandCallback(const_cast<void *>(thisPtr), parent);
    } else {
        return ::KDDockWidgets::flutter::ViewFactory::createRubberBand(parent);
    }
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createRubberBand_nocallback(KDDockWidgets::Core::View *parent) const
{
    return ::KDDockWidgets::flutter::ViewFactory::createRubberBand(parent);
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createSeparator(KDDockWidgets::Core::Separator *arg__1, KDDockWidgets::Core::View *parent) const
{
    if (m_createSeparatorCallback) {
        const void *thisPtr = this;
        return m_createSeparatorCallback(const_cast<void *>(thisPtr), arg__1, parent);
    } else {
        return ::KDDockWidgets::flutter::ViewFactory::createSeparator(arg__1, parent);
    }
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createSeparator_nocallback(KDDockWidgets::Core::Separator *arg__1, KDDockWidgets::Core::View *parent) const
{
    return ::KDDockWidgets::flutter::ViewFactory::createSeparator(arg__1, parent);
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createSideBar(KDDockWidgets::Core::SideBar *arg__1, KDDockWidgets::Core::View *parent) const
{
    if (m_createSideBarCallback) {
        const void *thisPtr = this;
        return m_createSideBarCallback(const_cast<void *>(thisPtr), arg__1, parent);
    } else {
        return ::KDDockWidgets::flutter::ViewFactory::createSideBar(arg__1, parent);
    }
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createSideBar_nocallback(KDDockWidgets::Core::SideBar *arg__1, KDDockWidgets::Core::View *parent) const
{
    return ::KDDockWidgets::flutter::ViewFactory::createSideBar(arg__1, parent);
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createStack(KDDockWidgets::Core::Stack *arg__1, KDDockWidgets::Core::View *parent) const
{
    if (m_createStackCallback) {
        const void *thisPtr = this;
        return m_createStackCallback(const_cast<void *>(thisPtr), arg__1, parent);
    } else {
        return ::KDDockWidgets::flutter::ViewFactory::createStack(arg__1, parent);
    }
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createStack_nocallback(KDDockWidgets::Core::Stack *arg__1, KDDockWidgets::Core::View *parent) const
{
    return ::KDDockWidgets::flutter::ViewFactory::createStack(arg__1, parent);
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createTabBar(KDDockWidgets::Core::TabBar *tabBar, KDDockWidgets::Core::View *parent) const
{
    if (m_createTabBarCallback) {
        const void *thisPtr = this;
        return m_createTabBarCallback(const_cast<void *>(thisPtr), tabBar, parent);
    } else {
        return ::KDDockWidgets::flutter::ViewFactory::createTabBar(tabBar, parent);
    }
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createTabBar_nocallback(KDDockWidgets::Core::TabBar *tabBar, KDDockWidgets::Core::View *parent) const
{
    return ::KDDockWidgets::flutter::ViewFactory::createTabBar(tabBar, parent);
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createTitleBar(KDDockWidgets::Core::TitleBar *arg__1, KDDockWidgets::Core::View *parent) const
{
    if (m_createTitleBarCallback) {
        const void *thisPtr = this;
        return m_createTitleBarCallback(const_cast<void *>(thisPtr), arg__1, parent);
    } else {
        return ::KDDockWidgets::flutter::ViewFactory::createTitleBar(arg__1, parent);
    }
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createTitleBar_nocallback(KDDockWidgets::Core::TitleBar *arg__1, KDDockWidgets::Core::View *parent) const
{
    return ::KDDockWidgets::flutter::ViewFactory::createTitleBar(arg__1, parent);
}
ViewFactory_wrapper::~ViewFactory_wrapper()
{
}

}
}
static KDDockWidgets::flutter::ViewFactory *fromPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgets::flutter::ViewFactory *>(ptr);
}
static KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::ViewFactory_wrapper *fromWrapperPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::ViewFactory_wrapper *>(ptr);
}
extern "C" {
void c_KDDockWidgets__flutter__ViewFactory_Finalizer(void *cppObj)
{
    delete reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::ViewFactory_wrapper *>(cppObj);
}
void *c_KDDockWidgets__flutter__ViewFactory__constructor()
{
    auto ptr = new KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::ViewFactory_wrapper();
    return reinterpret_cast<void *>(ptr);
}
// createClassicIndicatorWindow(KDDockWidgets::Core::ClassicDropIndicatorOverlay * arg__1, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__flutter__ViewFactory__createClassicIndicatorWindow_ClassicDropIndicatorOverlay_View(void *thisObj, void *arg__1_, void *parent_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::ClassicDropIndicatorOverlay *>(arg__1_);
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::ViewFactory_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->createClassicIndicatorWindow_nocallback(arg__1,parent);} else {    return targetPtr->createClassicIndicatorWindow(arg__1,parent);} }();
    return result;
}
// createClassicIndicatorWindow_flutter(KDDockWidgets::Core::ClassicDropIndicatorOverlay * arg__1, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__flutter__ViewFactory__createClassicIndicatorWindow_flutter_ClassicDropIndicatorOverlay_View(void *thisObj, void *arg__1_, void *parent_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::ClassicDropIndicatorOverlay *>(arg__1_);
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::ViewFactory_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->createClassicIndicatorWindow_flutter_nocallback(arg__1,parent);} else {    return targetPtr->createClassicIndicatorWindow_flutter(arg__1,parent);} }();
    return result;
}
// createDockWidget(const QString & uniqueName, QFlags<KDDockWidgets::DockWidgetOption> arg__2, QFlags<KDDockWidgets::LayoutSaverOption> arg__3, Qt::WindowFlags arg__4) const
void *c_KDDockWidgets__flutter__ViewFactory__createDockWidget_QString_DockWidgetOptions_LayoutSaverOptions_WindowFlags(void *thisObj, const char *uniqueName_, int arg__2_, int arg__3_, int arg__4)
{
    const auto uniqueName = QString::fromUtf8(uniqueName_);
    auto arg__2 = static_cast<QFlags<KDDockWidgets::DockWidgetOption>>(arg__2_);
    auto arg__3 = static_cast<QFlags<KDDockWidgets::LayoutSaverOption>>(arg__3_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::ViewFactory_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->createDockWidget_nocallback(uniqueName,arg__2,arg__3,static_cast<Qt::WindowFlags>(arg__4));} else {    return targetPtr->createDockWidget(uniqueName,arg__2,arg__3,static_cast<Qt::WindowFlags>(arg__4));} }();
    free(( char * )uniqueName_);
    return result;
}
// createDropArea(KDDockWidgets::Core::DropArea * arg__1, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__flutter__ViewFactory__createDropArea_DropArea_View(void *thisObj, void *arg__1_, void *parent_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::DropArea *>(arg__1_);
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::ViewFactory_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->createDropArea_nocallback(arg__1,parent);} else {    return targetPtr->createDropArea(arg__1,parent);} }();
    return result;
}
// createFloatingWindow(KDDockWidgets::Core::FloatingWindow * arg__1, KDDockWidgets::Core::MainWindow * parent, Qt::WindowFlags windowFlags) const
void *c_KDDockWidgets__flutter__ViewFactory__createFloatingWindow_FloatingWindow_MainWindow_WindowFlags(void *thisObj, void *arg__1_, void *parent_, int windowFlags)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::FloatingWindow *>(arg__1_);
    auto parent = reinterpret_cast<KDDockWidgets::Core::MainWindow *>(parent_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::ViewFactory_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->createFloatingWindow_nocallback(arg__1,parent,static_cast<Qt::WindowFlags>(windowFlags));} else {    return targetPtr->createFloatingWindow(arg__1,parent,static_cast<Qt::WindowFlags>(windowFlags));} }();
    return result;
}
// createGroup(KDDockWidgets::Core::Group * arg__1, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__flutter__ViewFactory__createGroup_Group_View(void *thisObj, void *arg__1_, void *parent_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::Group *>(arg__1_);
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::ViewFactory_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->createGroup_nocallback(arg__1,parent);} else {    return targetPtr->createGroup(arg__1,parent);} }();
    return result;
}
// createRubberBand(KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__flutter__ViewFactory__createRubberBand_View(void *thisObj, void *parent_)
{
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::ViewFactory_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->createRubberBand_nocallback(parent);} else {    return targetPtr->createRubberBand(parent);} }();
    return result;
}
// createSeparator(KDDockWidgets::Core::Separator * arg__1, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__flutter__ViewFactory__createSeparator_Separator_View(void *thisObj, void *arg__1_, void *parent_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::Separator *>(arg__1_);
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::ViewFactory_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->createSeparator_nocallback(arg__1,parent);} else {    return targetPtr->createSeparator(arg__1,parent);} }();
    return result;
}
// createSideBar(KDDockWidgets::Core::SideBar * arg__1, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__flutter__ViewFactory__createSideBar_SideBar_View(void *thisObj, void *arg__1_, void *parent_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::SideBar *>(arg__1_);
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::ViewFactory_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->createSideBar_nocallback(arg__1,parent);} else {    return targetPtr->createSideBar(arg__1,parent);} }();
    return result;
}
// createStack(KDDockWidgets::Core::Stack * arg__1, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__flutter__ViewFactory__createStack_Stack_View(void *thisObj, void *arg__1_, void *parent_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::Stack *>(arg__1_);
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::ViewFactory_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->createStack_nocallback(arg__1,parent);} else {    return targetPtr->createStack(arg__1,parent);} }();
    return result;
}
// createTabBar(KDDockWidgets::Core::TabBar * tabBar, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__flutter__ViewFactory__createTabBar_TabBar_View(void *thisObj, void *tabBar_, void *parent_)
{
    auto tabBar = reinterpret_cast<KDDockWidgets::Core::TabBar *>(tabBar_);
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::ViewFactory_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->createTabBar_nocallback(tabBar,parent);} else {    return targetPtr->createTabBar(tabBar,parent);} }();
    return result;
}
// createTitleBar(KDDockWidgets::Core::TitleBar * arg__1, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__flutter__ViewFactory__createTitleBar_TitleBar_View(void *thisObj, void *arg__1_, void *parent_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::TitleBar *>(arg__1_);
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::ViewFactory_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->createTitleBar_nocallback(arg__1,parent);} else {    return targetPtr->createTitleBar(arg__1,parent);} }();
    return result;
}
void c_KDDockWidgets__flutter__ViewFactory__destructor(void *thisObj)
{
    delete fromPtr(thisObj);
}
void c_KDDockWidgets__flutter__ViewFactory__registerVirtualMethodCallback(void *ptr, void *callback, int methodId)
{
    auto wrapper = fromWrapperPtr(ptr);
    switch (methodId) {
    case 235:
        wrapper->m_createClassicIndicatorWindowCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::ViewFactory_wrapper::Callback_createClassicIndicatorWindow>(callback);
        break;
    case 253:
        wrapper->m_createClassicIndicatorWindow_flutterCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::ViewFactory_wrapper::Callback_createClassicIndicatorWindow_flutter>(callback);
        break;
    case 236:
        wrapper->m_createDockWidgetCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::ViewFactory_wrapper::Callback_createDockWidget>(callback);
        break;
    case 237:
        wrapper->m_createDropAreaCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::ViewFactory_wrapper::Callback_createDropArea>(callback);
        break;
    case 238:
        wrapper->m_createFloatingWindowCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::ViewFactory_wrapper::Callback_createFloatingWindow>(callback);
        break;
    case 239:
        wrapper->m_createGroupCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::ViewFactory_wrapper::Callback_createGroup>(callback);
        break;
    case 240:
        wrapper->m_createRubberBandCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::ViewFactory_wrapper::Callback_createRubberBand>(callback);
        break;
    case 241:
        wrapper->m_createSeparatorCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::ViewFactory_wrapper::Callback_createSeparator>(callback);
        break;
    case 242:
        wrapper->m_createSideBarCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::ViewFactory_wrapper::Callback_createSideBar>(callback);
        break;
    case 243:
        wrapper->m_createStackCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::ViewFactory_wrapper::Callback_createStack>(callback);
        break;
    case 244:
        wrapper->m_createTabBarCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::ViewFactory_wrapper::Callback_createTabBar>(callback);
        break;
    case 245:
        wrapper->m_createTitleBarCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::ViewFactory_wrapper::Callback_createTitleBar>(callback);
        break;
    }
}
}
