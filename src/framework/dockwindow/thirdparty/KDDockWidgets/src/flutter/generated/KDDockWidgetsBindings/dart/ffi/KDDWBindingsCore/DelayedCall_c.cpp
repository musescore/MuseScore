/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "DelayedCall_c.h"


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
DelayedCall_wrapper::DelayedCall_wrapper()
    : ::KDDockWidgets::Core::DelayedCall()
{
}
void DelayedCall_wrapper::call()
{
    if (m_callCallback) {
        const void *thisPtr = this;
        m_callCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "call: Warning: Calling pure-virtual\n";
        return;
    }
}
void DelayedCall_wrapper::call_nocallback()
{
    std::cerr << "call: Warning: Calling pure-virtual\n";
    return;
}
DelayedCall_wrapper::~DelayedCall_wrapper()
{
}

}
}
static KDDockWidgets::Core::DelayedCall *fromPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgets::Core::DelayedCall *>(ptr);
}
static KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::DelayedCall_wrapper *fromWrapperPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::DelayedCall_wrapper *>(ptr);
}
extern "C" {
void c_KDDockWidgets__Core__DelayedCall_Finalizer(void *cppObj)
{
    delete reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::DelayedCall_wrapper *>(cppObj);
}
void *c_KDDockWidgets__Core__DelayedCall__constructor()
{
    auto ptr = new KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::DelayedCall_wrapper();
    return reinterpret_cast<void *>(ptr);
}
// call()
void c_KDDockWidgets__Core__DelayedCall__call(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::DelayedCall_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->call_nocallback();} else {    return targetPtr->call();} }();
}
void c_KDDockWidgets__Core__DelayedCall__destructor(void *thisObj)
{
    delete fromPtr(thisObj);
}
void c_KDDockWidgets__Core__DelayedCall__registerVirtualMethodCallback(void *ptr, void *callback, int methodId)
{
    auto wrapper = fromWrapperPtr(ptr);
    switch (methodId) {
    case 305:
        wrapper->m_callCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::DelayedCall_wrapper::Callback_call>(callback);
        break;
    }
}
}
