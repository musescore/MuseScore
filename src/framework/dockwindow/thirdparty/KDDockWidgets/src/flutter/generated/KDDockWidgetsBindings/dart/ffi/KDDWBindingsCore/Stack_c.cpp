/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "Stack_c.h"


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
void Stack_wrapper::addDockWidget(KDDockWidgets::Core::DockWidget *arg__1)
{
    ::KDDockWidgets::Core::Stack::addDockWidget(arg__1);
}
bool Stack_wrapper::contains(KDDockWidgets::Core::DockWidget *dw) const
{
    return ::KDDockWidgets::Core::Stack::contains(dw);
}
KDDockWidgets::Core::Group *Stack_wrapper::group() const
{
    return ::KDDockWidgets::Core::Stack::group();
}
bool Stack_wrapper::insertDockWidget(KDDockWidgets::Core::DockWidget *dockwidget, int index)
{
    return ::KDDockWidgets::Core::Stack::insertDockWidget(dockwidget, index);
}
bool Stack_wrapper::isMDI() const
{
    if (m_isMDICallback) {
        const void *thisPtr = this;
        return m_isMDICallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::Core::Stack::isMDI();
    }
}
bool Stack_wrapper::isMDI_nocallback() const
{
    return ::KDDockWidgets::Core::Stack::isMDI();
}
bool Stack_wrapper::isPositionDraggable(KDDockWidgets::Point p) const
{
    if (m_isPositionDraggableCallback) {
        const void *thisPtr = this;
        return m_isPositionDraggableCallback(const_cast<void *>(thisPtr), &p);
    } else {
        return ::KDDockWidgets::Core::Stack::isPositionDraggable(p);
    }
}
bool Stack_wrapper::isPositionDraggable_nocallback(KDDockWidgets::Point p) const
{
    return ::KDDockWidgets::Core::Stack::isPositionDraggable(p);
}
bool Stack_wrapper::isWindow() const
{
    if (m_isWindowCallback) {
        const void *thisPtr = this;
        return m_isWindowCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::Core::Stack::isWindow();
    }
}
bool Stack_wrapper::isWindow_nocallback() const
{
    return ::KDDockWidgets::Core::Stack::isWindow();
}
int Stack_wrapper::numDockWidgets() const
{
    return ::KDDockWidgets::Core::Stack::numDockWidgets();
}
bool Stack_wrapper::onMouseDoubleClick(KDDockWidgets::Point localPos)
{
    return ::KDDockWidgets::Core::Stack::onMouseDoubleClick(localPos);
}
void Stack_wrapper::setDocumentMode(bool arg__1)
{
    ::KDDockWidgets::Core::Stack::setDocumentMode(arg__1);
}
void Stack_wrapper::setParentView_impl(KDDockWidgets::Core::View *parent)
{
    if (m_setParentView_implCallback) {
        const void *thisPtr = this;
        m_setParentView_implCallback(const_cast<void *>(thisPtr), parent);
    } else {
        ::KDDockWidgets::Core::Stack::setParentView_impl(parent);
    }
}
void Stack_wrapper::setParentView_impl_nocallback(KDDockWidgets::Core::View *parent)
{
    ::KDDockWidgets::Core::Stack::setParentView_impl(parent);
}
void Stack_wrapper::setTabBarAutoHide(bool arg__1)
{
    ::KDDockWidgets::Core::Stack::setTabBarAutoHide(arg__1);
}
KDDockWidgets::Core::TabBar *Stack_wrapper::tabBar() const
{
    return ::KDDockWidgets::Core::Stack::tabBar();
}
bool Stack_wrapper::tabBarAutoHide() const
{
    return ::KDDockWidgets::Core::Stack::tabBarAutoHide();
}
Stack_wrapper::~Stack_wrapper()
{
}

}
}
static KDDockWidgets::Core::Stack *fromPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgets::Core::Stack *>(ptr);
}
static KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::Stack_wrapper *fromWrapperPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::Stack_wrapper *>(ptr);
}
extern "C" {
void c_KDDockWidgets__Core__Stack_Finalizer(void *cppObj)
{
    delete reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::Stack_wrapper *>(cppObj);
} // addDockWidget(KDDockWidgets::Core::DockWidget * arg__1)
void c_KDDockWidgets__Core__Stack__addDockWidget_DockWidget(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::DockWidget *>(arg__1_);
    fromPtr(thisObj)->addDockWidget(arg__1);
}
// contains(KDDockWidgets::Core::DockWidget * dw) const
bool c_KDDockWidgets__Core__Stack__contains_DockWidget(void *thisObj, void *dw_)
{
    auto dw = reinterpret_cast<KDDockWidgets::Core::DockWidget *>(dw_);
    const auto &result = fromPtr(thisObj)->contains(dw);
    return result;
}
// group() const
void *c_KDDockWidgets__Core__Stack__group(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->group();
    return result;
}
// insertDockWidget(KDDockWidgets::Core::DockWidget * dockwidget, int index)
bool c_KDDockWidgets__Core__Stack__insertDockWidget_DockWidget_int(void *thisObj, void *dockwidget_, int index)
{
    auto dockwidget = reinterpret_cast<KDDockWidgets::Core::DockWidget *>(dockwidget_);
    const auto &result = fromPtr(thisObj)->insertDockWidget(dockwidget, index);
    return result;
}
// isMDI() const
bool c_KDDockWidgets__Core__Stack__isMDI(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::Stack_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->isMDI_nocallback();} else {    return targetPtr->isMDI();} }();
    return result;
}
// isPositionDraggable(KDDockWidgets::Point p) const
bool c_KDDockWidgets__Core__Stack__isPositionDraggable_Point(void *thisObj, void *p_)
{
    assert(p_);
    auto &p = *reinterpret_cast<KDDockWidgets::Point *>(p_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::Stack_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->isPositionDraggable_nocallback(p);} else {    return targetPtr->isPositionDraggable(p);} }();
    return result;
}
// isWindow() const
bool c_KDDockWidgets__Core__Stack__isWindow(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::Stack_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->isWindow_nocallback();} else {    return targetPtr->isWindow();} }();
    return result;
}
// numDockWidgets() const
int c_KDDockWidgets__Core__Stack__numDockWidgets(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->numDockWidgets();
    return result;
}
// onMouseDoubleClick(KDDockWidgets::Point localPos)
bool c_KDDockWidgets__Core__Stack__onMouseDoubleClick_Point(void *thisObj, void *localPos_)
{
    assert(localPos_);
    auto &localPos = *reinterpret_cast<KDDockWidgets::Point *>(localPos_);
    const auto &result = fromPtr(thisObj)->onMouseDoubleClick(localPos);
    return result;
}
// setDocumentMode(bool arg__1)
void c_KDDockWidgets__Core__Stack__setDocumentMode_bool(void *thisObj, bool arg__1)
{
    fromPtr(thisObj)->setDocumentMode(arg__1);
}
// setParentView_impl(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__Core__Stack__setParentView_impl_View(void *thisObj, void *parent_)
{
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    fromWrapperPtr(thisObj)->setParentView_impl_nocallback(parent);
}
// setTabBarAutoHide(bool arg__1)
void c_KDDockWidgets__Core__Stack__setTabBarAutoHide_bool(void *thisObj, bool arg__1)
{
    fromPtr(thisObj)->setTabBarAutoHide(arg__1);
}
// singleDockWidget() const
void *c_KDDockWidgets__Core__Stack__singleDockWidget(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->singleDockWidget();
    return result;
}
// tabBar() const
void *c_KDDockWidgets__Core__Stack__tabBar(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->tabBar();
    return result;
}
// tabBarAutoHide() const
bool c_KDDockWidgets__Core__Stack__tabBarAutoHide(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->tabBarAutoHide();
    return result;
}
void c_KDDockWidgets__Core__Stack__destructor(void *thisObj)
{
    delete fromPtr(thisObj);
}
void c_KDDockWidgets__Core__Stack__registerVirtualMethodCallback(void *ptr, void *callback, int methodId)
{
    auto wrapper = fromWrapperPtr(ptr);
    switch (methodId) {
    case 656:
        wrapper->m_isMDICallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::Stack_wrapper::Callback_isMDI>(callback);
        break;
    case 657:
        wrapper->m_isPositionDraggableCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::Stack_wrapper::Callback_isPositionDraggable>(callback);
        break;
    case 659:
        wrapper->m_isWindowCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::Stack_wrapper::Callback_isWindow>(callback);
        break;
    case 331:
        wrapper->m_setParentView_implCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::Stack_wrapper::Callback_setParentView_impl>(callback);
        break;
    case 670:
        wrapper->m_singleDockWidgetCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::Stack_wrapper::Callback_singleDockWidget>(callback);
        break;
    }
}
}
