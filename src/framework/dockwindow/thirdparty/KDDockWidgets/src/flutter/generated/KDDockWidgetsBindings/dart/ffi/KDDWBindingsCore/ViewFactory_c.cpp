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
namespace KDDWBindingsCore {
ViewFactory_wrapper::ViewFactory_wrapper()
    : ::KDDockWidgets::Core::ViewFactory()
{
}
KDDockWidgets::Core::ClassicIndicatorWindowViewInterface *ViewFactory_wrapper::createClassicIndicatorWindow(KDDockWidgets::Core::ClassicDropIndicatorOverlay *arg__1, KDDockWidgets::Core::View *parent) const
{
    if (m_createClassicIndicatorWindowCallback) {
        const void *thisPtr = this;
        return m_createClassicIndicatorWindowCallback(const_cast<void *>(thisPtr), arg__1, parent);
    } else {
        std::cerr << "createClassicIndicatorWindow: Warning: Calling pure-virtual\n";
        return {};
    }
}
KDDockWidgets::Core::ClassicIndicatorWindowViewInterface *ViewFactory_wrapper::createClassicIndicatorWindow_nocallback(KDDockWidgets::Core::ClassicDropIndicatorOverlay *arg__1, KDDockWidgets::Core::View *parent) const
{
    std::cerr << "createClassicIndicatorWindow: Warning: Calling pure-virtual\n";
    return {};
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createDockWidget(const QString &uniqueName, QFlags<KDDockWidgets::DockWidgetOption> options, QFlags<KDDockWidgets::LayoutSaverOption> layoutSaverOptions, Qt::WindowFlags windowFlags) const
{
    if (m_createDockWidgetCallback) {
        const void *thisPtr = this;
        return m_createDockWidgetCallback(const_cast<void *>(thisPtr), uniqueName, options, layoutSaverOptions, windowFlags);
    } else {
        std::cerr << "createDockWidget: Warning: Calling pure-virtual\n";
        return {};
    }
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createDockWidget_nocallback(const QString &uniqueName, QFlags<KDDockWidgets::DockWidgetOption> options, QFlags<KDDockWidgets::LayoutSaverOption> layoutSaverOptions, Qt::WindowFlags windowFlags) const
{
    std::cerr << "createDockWidget: Warning: Calling pure-virtual\n";
    return {};
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createDropArea(KDDockWidgets::Core::DropArea *arg__1, KDDockWidgets::Core::View *parent) const
{
    if (m_createDropAreaCallback) {
        const void *thisPtr = this;
        return m_createDropAreaCallback(const_cast<void *>(thisPtr), arg__1, parent);
    } else {
        std::cerr << "createDropArea: Warning: Calling pure-virtual\n";
        return {};
    }
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createDropArea_nocallback(KDDockWidgets::Core::DropArea *arg__1, KDDockWidgets::Core::View *parent) const
{
    std::cerr << "createDropArea: Warning: Calling pure-virtual\n";
    return {};
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createFloatingWindow(KDDockWidgets::Core::FloatingWindow *controller, KDDockWidgets::Core::MainWindow *parent, Qt::WindowFlags windowFlags) const
{
    if (m_createFloatingWindowCallback) {
        const void *thisPtr = this;
        return m_createFloatingWindowCallback(const_cast<void *>(thisPtr), controller, parent, windowFlags);
    } else {
        std::cerr << "createFloatingWindow: Warning: Calling pure-virtual\n";
        return {};
    }
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createFloatingWindow_nocallback(KDDockWidgets::Core::FloatingWindow *controller, KDDockWidgets::Core::MainWindow *parent, Qt::WindowFlags windowFlags) const
{
    std::cerr << "createFloatingWindow: Warning: Calling pure-virtual\n";
    return {};
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createGroup(KDDockWidgets::Core::Group *arg__1, KDDockWidgets::Core::View *parent) const
{
    if (m_createGroupCallback) {
        const void *thisPtr = this;
        return m_createGroupCallback(const_cast<void *>(thisPtr), arg__1, parent);
    } else {
        std::cerr << "createGroup: Warning: Calling pure-virtual\n";
        return {};
    }
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createGroup_nocallback(KDDockWidgets::Core::Group *arg__1, KDDockWidgets::Core::View *parent) const
{
    std::cerr << "createGroup: Warning: Calling pure-virtual\n";
    return {};
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createRubberBand(KDDockWidgets::Core::View *parent) const
{
    if (m_createRubberBandCallback) {
        const void *thisPtr = this;
        return m_createRubberBandCallback(const_cast<void *>(thisPtr), parent);
    } else {
        std::cerr << "createRubberBand: Warning: Calling pure-virtual\n";
        return {};
    }
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createRubberBand_nocallback(KDDockWidgets::Core::View *parent) const
{
    std::cerr << "createRubberBand: Warning: Calling pure-virtual\n";
    return {};
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createSeparator(KDDockWidgets::Core::Separator *arg__1, KDDockWidgets::Core::View *parent) const
{
    if (m_createSeparatorCallback) {
        const void *thisPtr = this;
        return m_createSeparatorCallback(const_cast<void *>(thisPtr), arg__1, parent);
    } else {
        std::cerr << "createSeparator: Warning: Calling pure-virtual\n";
        return {};
    }
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createSeparator_nocallback(KDDockWidgets::Core::Separator *arg__1, KDDockWidgets::Core::View *parent) const
{
    std::cerr << "createSeparator: Warning: Calling pure-virtual\n";
    return {};
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createSideBar(KDDockWidgets::Core::SideBar *arg__1, KDDockWidgets::Core::View *parent) const
{
    if (m_createSideBarCallback) {
        const void *thisPtr = this;
        return m_createSideBarCallback(const_cast<void *>(thisPtr), arg__1, parent);
    } else {
        std::cerr << "createSideBar: Warning: Calling pure-virtual\n";
        return {};
    }
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createSideBar_nocallback(KDDockWidgets::Core::SideBar *arg__1, KDDockWidgets::Core::View *parent) const
{
    std::cerr << "createSideBar: Warning: Calling pure-virtual\n";
    return {};
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createStack(KDDockWidgets::Core::Stack *stack, KDDockWidgets::Core::View *parent) const
{
    if (m_createStackCallback) {
        const void *thisPtr = this;
        return m_createStackCallback(const_cast<void *>(thisPtr), stack, parent);
    } else {
        std::cerr << "createStack: Warning: Calling pure-virtual\n";
        return {};
    }
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createStack_nocallback(KDDockWidgets::Core::Stack *stack, KDDockWidgets::Core::View *parent) const
{
    std::cerr << "createStack: Warning: Calling pure-virtual\n";
    return {};
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createTabBar(KDDockWidgets::Core::TabBar *tabBar, KDDockWidgets::Core::View *parent) const
{
    if (m_createTabBarCallback) {
        const void *thisPtr = this;
        return m_createTabBarCallback(const_cast<void *>(thisPtr), tabBar, parent);
    } else {
        std::cerr << "createTabBar: Warning: Calling pure-virtual\n";
        return {};
    }
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createTabBar_nocallback(KDDockWidgets::Core::TabBar *tabBar, KDDockWidgets::Core::View *parent) const
{
    std::cerr << "createTabBar: Warning: Calling pure-virtual\n";
    return {};
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createTitleBar(KDDockWidgets::Core::TitleBar *controller, KDDockWidgets::Core::View *parent) const
{
    if (m_createTitleBarCallback) {
        const void *thisPtr = this;
        return m_createTitleBarCallback(const_cast<void *>(thisPtr), controller, parent);
    } else {
        std::cerr << "createTitleBar: Warning: Calling pure-virtual\n";
        return {};
    }
}
KDDockWidgets::Core::View *ViewFactory_wrapper::createTitleBar_nocallback(KDDockWidgets::Core::TitleBar *controller, KDDockWidgets::Core::View *parent) const
{
    std::cerr << "createTitleBar: Warning: Calling pure-virtual\n";
    return {};
}
ViewFactory_wrapper::~ViewFactory_wrapper()
{
}

}
}
static KDDockWidgets::Core::ViewFactory *fromPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgets::Core::ViewFactory *>(ptr);
}
static KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ViewFactory_wrapper *fromWrapperPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ViewFactory_wrapper *>(ptr);
}
extern "C" {
void c_KDDockWidgets__Core__ViewFactory_Finalizer(void *cppObj)
{
    delete reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ViewFactory_wrapper *>(cppObj);
}
void *c_KDDockWidgets__Core__ViewFactory__constructor()
{
    auto ptr = new KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ViewFactory_wrapper();
    return reinterpret_cast<void *>(ptr);
}
// createClassicIndicatorWindow(KDDockWidgets::Core::ClassicDropIndicatorOverlay * arg__1, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__Core__ViewFactory__createClassicIndicatorWindow_ClassicDropIndicatorOverlay_View(void *thisObj, void *arg__1_, void *parent_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::ClassicDropIndicatorOverlay *>(arg__1_);
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ViewFactory_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->createClassicIndicatorWindow_nocallback(arg__1,parent);} else {    return targetPtr->createClassicIndicatorWindow(arg__1,parent);} }();
    return result;
}
// createDockWidget(const QString & uniqueName, QFlags<KDDockWidgets::DockWidgetOption> options, QFlags<KDDockWidgets::LayoutSaverOption> layoutSaverOptions, Qt::WindowFlags windowFlags) const
void *c_KDDockWidgets__Core__ViewFactory__createDockWidget_QString_DockWidgetOptions_LayoutSaverOptions_WindowFlags(void *thisObj, const char *uniqueName_, int options_, int layoutSaverOptions_, int windowFlags)
{
    const auto uniqueName = QString::fromUtf8(uniqueName_);
    auto options = static_cast<QFlags<KDDockWidgets::DockWidgetOption>>(options_);
    auto layoutSaverOptions = static_cast<QFlags<KDDockWidgets::LayoutSaverOption>>(layoutSaverOptions_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ViewFactory_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->createDockWidget_nocallback(uniqueName,options,layoutSaverOptions,static_cast<Qt::WindowFlags>(windowFlags));} else {    return targetPtr->createDockWidget(uniqueName,options,layoutSaverOptions,static_cast<Qt::WindowFlags>(windowFlags));} }();
    free(( char * )uniqueName_);
    return result;
}
// createDropArea(KDDockWidgets::Core::DropArea * arg__1, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__Core__ViewFactory__createDropArea_DropArea_View(void *thisObj, void *arg__1_, void *parent_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::DropArea *>(arg__1_);
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ViewFactory_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->createDropArea_nocallback(arg__1,parent);} else {    return targetPtr->createDropArea(arg__1,parent);} }();
    return result;
}
// createFloatingWindow(KDDockWidgets::Core::FloatingWindow * controller, KDDockWidgets::Core::MainWindow * parent, Qt::WindowFlags windowFlags) const
void *c_KDDockWidgets__Core__ViewFactory__createFloatingWindow_FloatingWindow_MainWindow_WindowFlags(void *thisObj, void *controller_, void *parent_, int windowFlags)
{
    auto controller = reinterpret_cast<KDDockWidgets::Core::FloatingWindow *>(controller_);
    auto parent = reinterpret_cast<KDDockWidgets::Core::MainWindow *>(parent_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ViewFactory_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->createFloatingWindow_nocallback(controller,parent,static_cast<Qt::WindowFlags>(windowFlags));} else {    return targetPtr->createFloatingWindow(controller,parent,static_cast<Qt::WindowFlags>(windowFlags));} }();
    return result;
}
// createGroup(KDDockWidgets::Core::Group * arg__1, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__Core__ViewFactory__createGroup_Group_View(void *thisObj, void *arg__1_, void *parent_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::Group *>(arg__1_);
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ViewFactory_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->createGroup_nocallback(arg__1,parent);} else {    return targetPtr->createGroup(arg__1,parent);} }();
    return result;
}
// createRubberBand(KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__Core__ViewFactory__createRubberBand_View(void *thisObj, void *parent_)
{
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ViewFactory_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->createRubberBand_nocallback(parent);} else {    return targetPtr->createRubberBand(parent);} }();
    return result;
}
// createSeparator(KDDockWidgets::Core::Separator * arg__1, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__Core__ViewFactory__createSeparator_Separator_View(void *thisObj, void *arg__1_, void *parent_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::Separator *>(arg__1_);
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ViewFactory_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->createSeparator_nocallback(arg__1,parent);} else {    return targetPtr->createSeparator(arg__1,parent);} }();
    return result;
}
// createSideBar(KDDockWidgets::Core::SideBar * arg__1, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__Core__ViewFactory__createSideBar_SideBar_View(void *thisObj, void *arg__1_, void *parent_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::SideBar *>(arg__1_);
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ViewFactory_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->createSideBar_nocallback(arg__1,parent);} else {    return targetPtr->createSideBar(arg__1,parent);} }();
    return result;
}
// createStack(KDDockWidgets::Core::Stack * stack, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__Core__ViewFactory__createStack_Stack_View(void *thisObj, void *stack_, void *parent_)
{
    auto stack = reinterpret_cast<KDDockWidgets::Core::Stack *>(stack_);
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ViewFactory_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->createStack_nocallback(stack,parent);} else {    return targetPtr->createStack(stack,parent);} }();
    return result;
}
// createTabBar(KDDockWidgets::Core::TabBar * tabBar, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__Core__ViewFactory__createTabBar_TabBar_View(void *thisObj, void *tabBar_, void *parent_)
{
    auto tabBar = reinterpret_cast<KDDockWidgets::Core::TabBar *>(tabBar_);
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ViewFactory_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->createTabBar_nocallback(tabBar,parent);} else {    return targetPtr->createTabBar(tabBar,parent);} }();
    return result;
}
// createTitleBar(KDDockWidgets::Core::TitleBar * controller, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__Core__ViewFactory__createTitleBar_TitleBar_View(void *thisObj, void *controller_, void *parent_)
{
    auto controller = reinterpret_cast<KDDockWidgets::Core::TitleBar *>(controller_);
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ViewFactory_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->createTitleBar_nocallback(controller,parent);} else {    return targetPtr->createTitleBar(controller,parent);} }();
    return result;
}
void c_KDDockWidgets__Core__ViewFactory__destructor(void *thisObj)
{
    delete fromPtr(thisObj);
}
void c_KDDockWidgets__Core__ViewFactory__registerVirtualMethodCallback(void *ptr, void *callback, int methodId)
{
    auto wrapper = fromWrapperPtr(ptr);
    switch (methodId) {
    case 235:
        wrapper->m_createClassicIndicatorWindowCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ViewFactory_wrapper::Callback_createClassicIndicatorWindow>(callback);
        break;
    case 236:
        wrapper->m_createDockWidgetCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ViewFactory_wrapper::Callback_createDockWidget>(callback);
        break;
    case 237:
        wrapper->m_createDropAreaCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ViewFactory_wrapper::Callback_createDropArea>(callback);
        break;
    case 238:
        wrapper->m_createFloatingWindowCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ViewFactory_wrapper::Callback_createFloatingWindow>(callback);
        break;
    case 239:
        wrapper->m_createGroupCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ViewFactory_wrapper::Callback_createGroup>(callback);
        break;
    case 240:
        wrapper->m_createRubberBandCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ViewFactory_wrapper::Callback_createRubberBand>(callback);
        break;
    case 241:
        wrapper->m_createSeparatorCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ViewFactory_wrapper::Callback_createSeparator>(callback);
        break;
    case 242:
        wrapper->m_createSideBarCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ViewFactory_wrapper::Callback_createSideBar>(callback);
        break;
    case 243:
        wrapper->m_createStackCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ViewFactory_wrapper::Callback_createStack>(callback);
        break;
    case 244:
        wrapper->m_createTabBarCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ViewFactory_wrapper::Callback_createTabBar>(callback);
        break;
    case 245:
        wrapper->m_createTitleBarCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ViewFactory_wrapper::Callback_createTitleBar>(callback);
        break;
    }
}
}
