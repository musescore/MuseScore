/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "TabBar_c.h"


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
TabBar_wrapper::TabBar_wrapper(KDDockWidgets::Core::Stack *tabWidget)
    : ::KDDockWidgets::Core::TabBar(tabWidget)
{
}
KDDockWidgets::Core::DockWidget *TabBar_wrapper::currentDockWidget() const
{
    return ::KDDockWidgets::Core::TabBar::currentDockWidget();
}
int TabBar_wrapper::currentIndex() const
{
    return ::KDDockWidgets::Core::TabBar::currentIndex();
}
KDDockWidgets::Core::DockWidget *TabBar_wrapper::dockWidgetAt(KDDockWidgets::Point localPos) const
{
    return ::KDDockWidgets::Core::TabBar::dockWidgetAt(localPos);
}
KDDockWidgets::Core::DockWidget *TabBar_wrapper::dockWidgetAt(int index) const
{
    return ::KDDockWidgets::Core::TabBar::dockWidgetAt(index);
}
bool TabBar_wrapper::dragCanStart(KDDockWidgets::Point pressPos, KDDockWidgets::Point pos) const
{
    if (m_dragCanStartCallback) {
        const void *thisPtr = this;
        return m_dragCanStartCallback(const_cast<void *>(thisPtr), &pressPos, &pos);
    } else {
        return ::KDDockWidgets::Core::TabBar::dragCanStart(pressPos, pos);
    }
}
bool TabBar_wrapper::dragCanStart_nocallback(KDDockWidgets::Point pressPos, KDDockWidgets::Point pos) const
{
    return ::KDDockWidgets::Core::TabBar::dragCanStart(pressPos, pos);
}
KDDockWidgets::Core::Group *TabBar_wrapper::group() const
{
    return ::KDDockWidgets::Core::TabBar::group();
}
bool TabBar_wrapper::hasSingleDockWidget() const
{
    return ::KDDockWidgets::Core::TabBar::hasSingleDockWidget();
}
int TabBar_wrapper::indexOfDockWidget(const KDDockWidgets::Core::DockWidget *dw) const
{
    return ::KDDockWidgets::Core::TabBar::indexOfDockWidget(dw);
}
bool TabBar_wrapper::isMDI() const
{
    if (m_isMDICallback) {
        const void *thisPtr = this;
        return m_isMDICallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::Core::TabBar::isMDI();
    }
}
bool TabBar_wrapper::isMDI_nocallback() const
{
    return ::KDDockWidgets::Core::TabBar::isMDI();
}
bool TabBar_wrapper::isMovingTab() const
{
    return ::KDDockWidgets::Core::TabBar::isMovingTab();
}
bool TabBar_wrapper::isWindow() const
{
    if (m_isWindowCallback) {
        const void *thisPtr = this;
        return m_isWindowCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::Core::TabBar::isWindow();
    }
}
bool TabBar_wrapper::isWindow_nocallback() const
{
    return ::KDDockWidgets::Core::TabBar::isWindow();
}
void TabBar_wrapper::moveTabTo(int from, int to)
{
    ::KDDockWidgets::Core::TabBar::moveTabTo(from, to);
}
int TabBar_wrapper::numDockWidgets() const
{
    return ::KDDockWidgets::Core::TabBar::numDockWidgets();
}
void TabBar_wrapper::onMouseDoubleClick(KDDockWidgets::Point localPos)
{
    ::KDDockWidgets::Core::TabBar::onMouseDoubleClick(localPos);
}
void TabBar_wrapper::onMousePress(KDDockWidgets::Point localPos)
{
    ::KDDockWidgets::Core::TabBar::onMousePress(localPos);
}
KDDockWidgets::Rect TabBar_wrapper::rectForTab(int index) const
{
    return ::KDDockWidgets::Core::TabBar::rectForTab(index);
}
void TabBar_wrapper::removeDockWidget(KDDockWidgets::Core::DockWidget *dw)
{
    ::KDDockWidgets::Core::TabBar::removeDockWidget(dw);
}
void TabBar_wrapper::renameTab(int index, const QString &arg__2)
{
    ::KDDockWidgets::Core::TabBar::renameTab(index, arg__2);
}
void TabBar_wrapper::setCurrentDockWidget(KDDockWidgets::Core::DockWidget *dw)
{
    ::KDDockWidgets::Core::TabBar::setCurrentDockWidget(dw);
}
void TabBar_wrapper::setCurrentIndex(int index)
{
    ::KDDockWidgets::Core::TabBar::setCurrentIndex(index);
}
void TabBar_wrapper::setParentView_impl(KDDockWidgets::Core::View *parent)
{
    if (m_setParentView_implCallback) {
        const void *thisPtr = this;
        m_setParentView_implCallback(const_cast<void *>(thisPtr), parent);
    } else {
        ::KDDockWidgets::Core::TabBar::setParentView_impl(parent);
    }
}
void TabBar_wrapper::setParentView_impl_nocallback(KDDockWidgets::Core::View *parent)
{
    ::KDDockWidgets::Core::TabBar::setParentView_impl(parent);
}
KDDockWidgets::Core::Stack *TabBar_wrapper::stack() const
{
    return ::KDDockWidgets::Core::TabBar::stack();
}
bool TabBar_wrapper::tabsAreMovable() const
{
    return ::KDDockWidgets::Core::TabBar::tabsAreMovable();
}
QString TabBar_wrapper::text(int index) const
{
    return ::KDDockWidgets::Core::TabBar::text(index);
}
TabBar_wrapper::~TabBar_wrapper()
{
}

}
}
static KDDockWidgets::Core::TabBar *fromPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgets::Core::TabBar *>(ptr);
}
static KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::TabBar_wrapper *fromWrapperPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::TabBar_wrapper *>(ptr);
}
extern "C" {
void c_KDDockWidgets__Core__TabBar_Finalizer(void *cppObj)
{
    delete reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::TabBar_wrapper *>(cppObj);
}
void *c_KDDockWidgets__Core__TabBar__constructor_Stack(void *tabWidget_)
{
    auto tabWidget = reinterpret_cast<KDDockWidgets::Core::Stack *>(tabWidget_);
    auto ptr = new KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::TabBar_wrapper(tabWidget);
    return reinterpret_cast<void *>(ptr);
}
// currentDockWidget() const
void *c_KDDockWidgets__Core__TabBar__currentDockWidget(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->currentDockWidget();
    return result;
}
// currentIndex() const
int c_KDDockWidgets__Core__TabBar__currentIndex(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->currentIndex();
    return result;
}
// dockWidgetAt(KDDockWidgets::Point localPos) const
void *c_KDDockWidgets__Core__TabBar__dockWidgetAt_Point(void *thisObj, void *localPos_)
{
    assert(localPos_);
    auto &localPos = *reinterpret_cast<KDDockWidgets::Point *>(localPos_);
    const auto &result = fromPtr(thisObj)->dockWidgetAt(localPos);
    return result;
}
// dockWidgetAt(int index) const
void *c_KDDockWidgets__Core__TabBar__dockWidgetAt_int(void *thisObj, int index)
{
    const auto &result = fromPtr(thisObj)->dockWidgetAt(index);
    return result;
}
// dragCanStart(KDDockWidgets::Point pressPos, KDDockWidgets::Point pos) const
bool c_KDDockWidgets__Core__TabBar__dragCanStart_Point_Point(void *thisObj, void *pressPos_, void *pos_)
{
    assert(pressPos_);
    auto &pressPos = *reinterpret_cast<KDDockWidgets::Point *>(pressPos_);
    assert(pos_);
    auto &pos = *reinterpret_cast<KDDockWidgets::Point *>(pos_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::TabBar_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->dragCanStart_nocallback(pressPos,pos);} else {    return targetPtr->dragCanStart(pressPos,pos);} }();
    return result;
}
// group() const
void *c_KDDockWidgets__Core__TabBar__group(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->group();
    return result;
}
// hasSingleDockWidget() const
bool c_KDDockWidgets__Core__TabBar__hasSingleDockWidget(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->hasSingleDockWidget();
    return result;
}
// indexOfDockWidget(const KDDockWidgets::Core::DockWidget * dw) const
int c_KDDockWidgets__Core__TabBar__indexOfDockWidget_DockWidget(void *thisObj, void *dw_)
{
    auto dw = reinterpret_cast<KDDockWidgets::Core::DockWidget *>(dw_);
    const auto &result = fromPtr(thisObj)->indexOfDockWidget(dw);
    return result;
}
// isMDI() const
bool c_KDDockWidgets__Core__TabBar__isMDI(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::TabBar_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->isMDI_nocallback();} else {    return targetPtr->isMDI();} }();
    return result;
}
// isMovingTab() const
bool c_KDDockWidgets__Core__TabBar__isMovingTab(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->isMovingTab();
    return result;
}
// isWindow() const
bool c_KDDockWidgets__Core__TabBar__isWindow(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::TabBar_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->isWindow_nocallback();} else {    return targetPtr->isWindow();} }();
    return result;
}
// moveTabTo(int from, int to)
void c_KDDockWidgets__Core__TabBar__moveTabTo_int_int(void *thisObj, int from, int to)
{
    fromPtr(thisObj)->moveTabTo(from, to);
}
// numDockWidgets() const
int c_KDDockWidgets__Core__TabBar__numDockWidgets(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->numDockWidgets();
    return result;
}
// onMouseDoubleClick(KDDockWidgets::Point localPos)
void c_KDDockWidgets__Core__TabBar__onMouseDoubleClick_Point(void *thisObj, void *localPos_)
{
    assert(localPos_);
    auto &localPos = *reinterpret_cast<KDDockWidgets::Point *>(localPos_);
    fromPtr(thisObj)->onMouseDoubleClick(localPos);
}
// onMousePress(KDDockWidgets::Point localPos)
void c_KDDockWidgets__Core__TabBar__onMousePress_Point(void *thisObj, void *localPos_)
{
    assert(localPos_);
    auto &localPos = *reinterpret_cast<KDDockWidgets::Point *>(localPos_);
    fromPtr(thisObj)->onMousePress(localPos);
}
// rectForTab(int index) const
void *c_KDDockWidgets__Core__TabBar__rectForTab_int(void *thisObj, int index)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Rect> { fromPtr(thisObj)->rectForTab(index) };
    return result;
}
// removeDockWidget(KDDockWidgets::Core::DockWidget * dw)
void c_KDDockWidgets__Core__TabBar__removeDockWidget_DockWidget(void *thisObj, void *dw_)
{
    auto dw = reinterpret_cast<KDDockWidgets::Core::DockWidget *>(dw_);
    fromPtr(thisObj)->removeDockWidget(dw);
}
// renameTab(int index, const QString & arg__2)
void c_KDDockWidgets__Core__TabBar__renameTab_int_QString(void *thisObj, int index, const char *arg__2_)
{
    const auto arg__2 = QString::fromUtf8(arg__2_);
    fromPtr(thisObj)->renameTab(index, arg__2);
    free(( char * )arg__2_);
}
// setCurrentDockWidget(KDDockWidgets::Core::DockWidget * dw)
void c_KDDockWidgets__Core__TabBar__setCurrentDockWidget_DockWidget(void *thisObj, void *dw_)
{
    auto dw = reinterpret_cast<KDDockWidgets::Core::DockWidget *>(dw_);
    fromPtr(thisObj)->setCurrentDockWidget(dw);
}
// setCurrentIndex(int index)
void c_KDDockWidgets__Core__TabBar__setCurrentIndex_int(void *thisObj, int index)
{
    fromPtr(thisObj)->setCurrentIndex(index);
}
// setParentView_impl(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__Core__TabBar__setParentView_impl_View(void *thisObj, void *parent_)
{
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    fromWrapperPtr(thisObj)->setParentView_impl_nocallback(parent);
}
// singleDockWidget() const
void *c_KDDockWidgets__Core__TabBar__singleDockWidget(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->singleDockWidget();
    return result;
}
// stack() const
void *c_KDDockWidgets__Core__TabBar__stack(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->stack();
    return result;
}
// tabsAreMovable() const
bool c_KDDockWidgets__Core__TabBar__tabsAreMovable(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->tabsAreMovable();
    return result;
}
// text(int index) const
void *c_KDDockWidgets__Core__TabBar__text_int(void *thisObj, int index)
{
    const auto &result = new Dartagnan::ValueWrapper<QString> { fromPtr(thisObj)->text(index) };
    return result;
}
void c_KDDockWidgets__Core__TabBar__destructor(void *thisObj)
{
    delete fromPtr(thisObj);
}
void c_KDDockWidgets__Core__TabBar__registerVirtualMethodCallback(void *ptr, void *callback, int methodId)
{
    auto wrapper = fromWrapperPtr(ptr);
    switch (methodId) {
    case 605:
        wrapper->m_dragCanStartCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::TabBar_wrapper::Callback_dragCanStart>(callback);
        break;
    case 615:
        wrapper->m_isMDICallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::TabBar_wrapper::Callback_isMDI>(callback);
        break;
    case 618:
        wrapper->m_isWindowCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::TabBar_wrapper::Callback_isWindow>(callback);
        break;
    case 331:
        wrapper->m_setParentView_implCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::TabBar_wrapper::Callback_setParentView_impl>(callback);
        break;
    case 634:
        wrapper->m_singleDockWidgetCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::TabBar_wrapper::Callback_singleDockWidget>(callback);
        break;
    }
}
}
