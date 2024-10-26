/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "Separator_c.h"


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
bool Separator_wrapper::isResizing()
{
    return ::KDDockWidgets::Core::Separator::isResizing();
}
bool Separator_wrapper::isVertical() const
{
    return ::KDDockWidgets::Core::Separator::isVertical();
}
int Separator_wrapper::numSeparators()
{
    return ::KDDockWidgets::Core::Separator::numSeparators();
}
void Separator_wrapper::onMouseDoubleClick()
{
    ::KDDockWidgets::Core::Separator::onMouseDoubleClick();
}
void Separator_wrapper::onMouseMove(KDDockWidgets::Point pos)
{
    ::KDDockWidgets::Core::Separator::onMouseMove(pos);
}
void Separator_wrapper::onMousePress()
{
    ::KDDockWidgets::Core::Separator::onMousePress();
}
void Separator_wrapper::onMouseReleased()
{
    ::KDDockWidgets::Core::Separator::onMouseReleased();
}
int Separator_wrapper::position() const
{
    return ::KDDockWidgets::Core::Separator::position();
}
void Separator_wrapper::setGeometry(KDDockWidgets::Rect r)
{
    ::KDDockWidgets::Core::Separator::setGeometry(r);
}
void Separator_wrapper::setParentView_impl(KDDockWidgets::Core::View *parent)
{
    if (m_setParentView_implCallback) {
        const void *thisPtr = this;
        m_setParentView_implCallback(const_cast<void *>(thisPtr), parent);
    } else {
        ::KDDockWidgets::Core::Separator::setParentView_impl(parent);
    }
}
void Separator_wrapper::setParentView_impl_nocallback(KDDockWidgets::Core::View *parent)
{
    ::KDDockWidgets::Core::Separator::setParentView_impl(parent);
}
Separator_wrapper::~Separator_wrapper()
{
}

}
}
static KDDockWidgets::Core::Separator *fromPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgets::Core::Separator *>(ptr);
}
static KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::Separator_wrapper *fromWrapperPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::Separator_wrapper *>(ptr);
}
extern "C" {
void c_KDDockWidgets__Core__Separator_Finalizer(void *cppObj)
{
    delete reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::Separator_wrapper *>(cppObj);
} // isResizing()
bool c_static_KDDockWidgets__Core__Separator__isResizing()
{
    const auto &result = KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::Separator_wrapper::isResizing();
    return result;
}
// isVertical() const
bool c_KDDockWidgets__Core__Separator__isVertical(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->isVertical();
    return result;
}
// numSeparators()
int c_static_KDDockWidgets__Core__Separator__numSeparators()
{
    const auto &result = KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::Separator_wrapper::numSeparators();
    return result;
}
// onMouseDoubleClick()
void c_KDDockWidgets__Core__Separator__onMouseDoubleClick(void *thisObj)
{
    fromPtr(thisObj)->onMouseDoubleClick();
}
// onMouseMove(KDDockWidgets::Point pos)
void c_KDDockWidgets__Core__Separator__onMouseMove_Point(void *thisObj, void *pos_)
{
    assert(pos_);
    auto &pos = *reinterpret_cast<KDDockWidgets::Point *>(pos_);
    fromPtr(thisObj)->onMouseMove(pos);
}
// onMousePress()
void c_KDDockWidgets__Core__Separator__onMousePress(void *thisObj)
{
    fromPtr(thisObj)->onMousePress();
}
// onMouseReleased()
void c_KDDockWidgets__Core__Separator__onMouseReleased(void *thisObj)
{
    fromPtr(thisObj)->onMouseReleased();
}
// position() const
int c_KDDockWidgets__Core__Separator__position(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->position();
    return result;
}
// setGeometry(KDDockWidgets::Rect r)
void c_KDDockWidgets__Core__Separator__setGeometry_Rect(void *thisObj, void *r_)
{
    assert(r_);
    auto &r = *reinterpret_cast<KDDockWidgets::Rect *>(r_);
    fromPtr(thisObj)->setGeometry(r);
}
// setParentView_impl(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__Core__Separator__setParentView_impl_View(void *thisObj, void *parent_)
{
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    fromWrapperPtr(thisObj)->setParentView_impl_nocallback(parent);
}
void c_KDDockWidgets__Core__Separator__destructor(void *thisObj)
{
    delete fromPtr(thisObj);
}
void c_KDDockWidgets__Core__Separator__registerVirtualMethodCallback(void *ptr, void *callback, int methodId)
{
    auto wrapper = fromWrapperPtr(ptr);
    switch (methodId) {
    case 331:
        wrapper->m_setParentView_implCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::Separator_wrapper::Callback_setParentView_impl>(callback);
        break;
    }
}
}
