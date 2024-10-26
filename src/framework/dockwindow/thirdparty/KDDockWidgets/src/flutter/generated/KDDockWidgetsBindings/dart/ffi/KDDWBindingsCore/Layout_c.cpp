/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "Layout_c.h"


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
Layout_wrapper::Layout_wrapper(KDDockWidgets::Core::ViewType arg__1, KDDockWidgets::Core::View *arg__2)
    : ::KDDockWidgets::Core::Layout(arg__1, arg__2)
{
}
KDDockWidgets::Core::DropArea *Layout_wrapper::asDropArea() const
{
    return ::KDDockWidgets::Core::Layout::asDropArea();
}
bool Layout_wrapper::checkSanity() const
{
    return ::KDDockWidgets::Core::Layout::checkSanity();
}
void Layout_wrapper::clearLayout()
{
    ::KDDockWidgets::Core::Layout::clearLayout();
}
bool Layout_wrapper::containsGroup(const KDDockWidgets::Core::Group *arg__1) const
{
    return ::KDDockWidgets::Core::Layout::containsGroup(arg__1);
}
bool Layout_wrapper::containsItem(const KDDockWidgets::Core::Item *arg__1) const
{
    return ::KDDockWidgets::Core::Layout::containsItem(arg__1);
}
int Layout_wrapper::count() const
{
    return ::KDDockWidgets::Core::Layout::count();
}
void Layout_wrapper::dumpLayout() const
{
    ::KDDockWidgets::Core::Layout::dumpLayout();
}
KDDockWidgets::Core::FloatingWindow *Layout_wrapper::floatingWindow() const
{
    return ::KDDockWidgets::Core::Layout::floatingWindow();
}
bool Layout_wrapper::isInMainWindow(bool honourNesting) const
{
    return ::KDDockWidgets::Core::Layout::isInMainWindow(honourNesting);
}
KDDockWidgets::Core::Item *Layout_wrapper::itemForGroup(const KDDockWidgets::Core::Group *group) const
{
    return ::KDDockWidgets::Core::Layout::itemForGroup(group);
}
int Layout_wrapper::layoutHeight() const
{
    return ::KDDockWidgets::Core::Layout::layoutHeight();
}
KDDockWidgets::Size Layout_wrapper::layoutMaximumSizeHint() const
{
    return ::KDDockWidgets::Core::Layout::layoutMaximumSizeHint();
}
KDDockWidgets::Size Layout_wrapper::layoutMinimumSize() const
{
    return ::KDDockWidgets::Core::Layout::layoutMinimumSize();
}
KDDockWidgets::Size Layout_wrapper::layoutSize() const
{
    return ::KDDockWidgets::Core::Layout::layoutSize();
}
int Layout_wrapper::layoutWidth() const
{
    return ::KDDockWidgets::Core::Layout::layoutWidth();
}
KDDockWidgets::Core::MainWindow *Layout_wrapper::mainWindow(bool honourNesting) const
{
    return ::KDDockWidgets::Core::Layout::mainWindow(honourNesting);
}
int Layout_wrapper::placeholderCount() const
{
    return ::KDDockWidgets::Core::Layout::placeholderCount();
}
void Layout_wrapper::removeItem(KDDockWidgets::Core::Item *item)
{
    ::KDDockWidgets::Core::Layout::removeItem(item);
}
void Layout_wrapper::restorePlaceholder(KDDockWidgets::Core::DockWidget *dw, KDDockWidgets::Core::Item *arg__2, int tabIndex)
{
    ::KDDockWidgets::Core::Layout::restorePlaceholder(dw, arg__2, tabIndex);
}
void Layout_wrapper::setLayoutMinimumSize(KDDockWidgets::Size arg__1)
{
    ::KDDockWidgets::Core::Layout::setLayoutMinimumSize(arg__1);
}
void Layout_wrapper::setLayoutSize(KDDockWidgets::Size arg__1)
{
    ::KDDockWidgets::Core::Layout::setLayoutSize(arg__1);
}
void Layout_wrapper::setParentView_impl(KDDockWidgets::Core::View *parent)
{
    if (m_setParentView_implCallback) {
        const void *thisPtr = this;
        m_setParentView_implCallback(const_cast<void *>(thisPtr), parent);
    } else {
        ::KDDockWidgets::Core::Layout::setParentView_impl(parent);
    }
}
void Layout_wrapper::setParentView_impl_nocallback(KDDockWidgets::Core::View *parent)
{
    ::KDDockWidgets::Core::Layout::setParentView_impl(parent);
}
void Layout_wrapper::updateSizeConstraints()
{
    ::KDDockWidgets::Core::Layout::updateSizeConstraints();
}
void Layout_wrapper::viewAboutToBeDeleted()
{
    ::KDDockWidgets::Core::Layout::viewAboutToBeDeleted();
}
int Layout_wrapper::visibleCount() const
{
    return ::KDDockWidgets::Core::Layout::visibleCount();
}
Layout_wrapper::~Layout_wrapper()
{
}

}
}
static KDDockWidgets::Core::Layout *fromPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgets::Core::Layout *>(ptr);
}
static KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::Layout_wrapper *fromWrapperPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::Layout_wrapper *>(ptr);
}
extern "C" {
void c_KDDockWidgets__Core__Layout_Finalizer(void *cppObj)
{
    delete reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::Layout_wrapper *>(cppObj);
}
void *c_KDDockWidgets__Core__Layout__constructor_ViewType_View(int arg__1, void *arg__2_)
{
    auto arg__2 = reinterpret_cast<KDDockWidgets::Core::View *>(arg__2_);
    auto ptr = new KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::Layout_wrapper(static_cast<KDDockWidgets::Core::ViewType>(arg__1), arg__2);
    return reinterpret_cast<void *>(ptr);
}
// asDropArea() const
void *c_KDDockWidgets__Core__Layout__asDropArea(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->asDropArea();
    return result;
}
// checkSanity() const
bool c_KDDockWidgets__Core__Layout__checkSanity(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->checkSanity();
    return result;
}
// clearLayout()
void c_KDDockWidgets__Core__Layout__clearLayout(void *thisObj)
{
    fromPtr(thisObj)->clearLayout();
}
// containsGroup(const KDDockWidgets::Core::Group * arg__1) const
bool c_KDDockWidgets__Core__Layout__containsGroup_Group(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::Group *>(arg__1_);
    const auto &result = fromPtr(thisObj)->containsGroup(arg__1);
    return result;
}
// containsItem(const KDDockWidgets::Core::Item * arg__1) const
bool c_KDDockWidgets__Core__Layout__containsItem_Item(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::Item *>(arg__1_);
    const auto &result = fromPtr(thisObj)->containsItem(arg__1);
    return result;
}
// count() const
int c_KDDockWidgets__Core__Layout__count(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->count();
    return result;
}
// dumpLayout() const
void c_KDDockWidgets__Core__Layout__dumpLayout(void *thisObj)
{
    fromPtr(thisObj)->dumpLayout();
}
// floatingWindow() const
void *c_KDDockWidgets__Core__Layout__floatingWindow(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->floatingWindow();
    return result;
}
// isInMainWindow(bool honourNesting) const
bool c_KDDockWidgets__Core__Layout__isInMainWindow_bool(void *thisObj, bool honourNesting)
{
    const auto &result = fromPtr(thisObj)->isInMainWindow(honourNesting);
    return result;
}
// itemForGroup(const KDDockWidgets::Core::Group * group) const
void *c_KDDockWidgets__Core__Layout__itemForGroup_Group(void *thisObj, void *group_)
{
    auto group = reinterpret_cast<KDDockWidgets::Core::Group *>(group_);
    const auto &result = fromPtr(thisObj)->itemForGroup(group);
    return result;
}
// layoutHeight() const
int c_KDDockWidgets__Core__Layout__layoutHeight(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->layoutHeight();
    return result;
}
// layoutMaximumSizeHint() const
void *c_KDDockWidgets__Core__Layout__layoutMaximumSizeHint(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Size> { fromPtr(thisObj)->layoutMaximumSizeHint() };
    return result;
}
// layoutMinimumSize() const
void *c_KDDockWidgets__Core__Layout__layoutMinimumSize(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Size> { fromPtr(thisObj)->layoutMinimumSize() };
    return result;
}
// layoutSize() const
void *c_KDDockWidgets__Core__Layout__layoutSize(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Size> { fromPtr(thisObj)->layoutSize() };
    return result;
}
// layoutWidth() const
int c_KDDockWidgets__Core__Layout__layoutWidth(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->layoutWidth();
    return result;
}
// mainWindow(bool honourNesting) const
void *c_KDDockWidgets__Core__Layout__mainWindow_bool(void *thisObj, bool honourNesting)
{
    const auto &result = fromPtr(thisObj)->mainWindow(honourNesting);
    return result;
}
// placeholderCount() const
int c_KDDockWidgets__Core__Layout__placeholderCount(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->placeholderCount();
    return result;
}
// removeItem(KDDockWidgets::Core::Item * item)
void c_KDDockWidgets__Core__Layout__removeItem_Item(void *thisObj, void *item_)
{
    auto item = reinterpret_cast<KDDockWidgets::Core::Item *>(item_);
    fromPtr(thisObj)->removeItem(item);
}
// restorePlaceholder(KDDockWidgets::Core::DockWidget * dw, KDDockWidgets::Core::Item * arg__2, int tabIndex)
void c_KDDockWidgets__Core__Layout__restorePlaceholder_DockWidget_Item_int(void *thisObj, void *dw_, void *arg__2_, int tabIndex)
{
    auto dw = reinterpret_cast<KDDockWidgets::Core::DockWidget *>(dw_);
    auto arg__2 = reinterpret_cast<KDDockWidgets::Core::Item *>(arg__2_);
    fromPtr(thisObj)->restorePlaceholder(dw, arg__2, tabIndex);
}
// setLayoutMinimumSize(KDDockWidgets::Size arg__1)
void c_KDDockWidgets__Core__Layout__setLayoutMinimumSize_Size(void *thisObj, void *arg__1_)
{
    assert(arg__1_);
    auto &arg__1 = *reinterpret_cast<KDDockWidgets::Size *>(arg__1_);
    fromWrapperPtr(thisObj)->setLayoutMinimumSize(arg__1);
}
// setLayoutSize(KDDockWidgets::Size arg__1)
void c_KDDockWidgets__Core__Layout__setLayoutSize_Size(void *thisObj, void *arg__1_)
{
    assert(arg__1_);
    auto &arg__1 = *reinterpret_cast<KDDockWidgets::Size *>(arg__1_);
    fromPtr(thisObj)->setLayoutSize(arg__1);
}
// setParentView_impl(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__Core__Layout__setParentView_impl_View(void *thisObj, void *parent_)
{
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    fromWrapperPtr(thisObj)->setParentView_impl_nocallback(parent);
}
// updateSizeConstraints()
void c_KDDockWidgets__Core__Layout__updateSizeConstraints(void *thisObj)
{
    fromPtr(thisObj)->updateSizeConstraints();
}
// viewAboutToBeDeleted()
void c_KDDockWidgets__Core__Layout__viewAboutToBeDeleted(void *thisObj)
{
    fromPtr(thisObj)->viewAboutToBeDeleted();
}
// visibleCount() const
int c_KDDockWidgets__Core__Layout__visibleCount(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->visibleCount();
    return result;
}
void c_KDDockWidgets__Core__Layout__destructor(void *thisObj)
{
    delete fromPtr(thisObj);
}
void c_KDDockWidgets__Core__Layout__registerVirtualMethodCallback(void *ptr, void *callback, int methodId)
{
    auto wrapper = fromWrapperPtr(ptr);
    switch (methodId) {
    case 331:
        wrapper->m_setParentView_implCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::Layout_wrapper::Callback_setParentView_impl>(callback);
        break;
    }
}
}
