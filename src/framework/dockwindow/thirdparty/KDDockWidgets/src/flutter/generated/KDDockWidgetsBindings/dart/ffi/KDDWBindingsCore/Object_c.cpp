/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "Object_c.h"


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
Object_wrapper::Object_wrapper(KDDockWidgets::Core::Object *parent)
    : ::KDDockWidgets::Core::Object(parent)
{
}
QString Object_wrapper::objectName() const
{
    return ::KDDockWidgets::Core::Object::objectName();
}
KDDockWidgets::Core::Object *Object_wrapper::parent() const
{
    return ::KDDockWidgets::Core::Object::parent();
}
void Object_wrapper::setObjectName(const QString &arg__1)
{
    ::KDDockWidgets::Core::Object::setObjectName(arg__1);
}
void Object_wrapper::setParent(KDDockWidgets::Core::Object *parent)
{
    ::KDDockWidgets::Core::Object::setParent(parent);
}
QString Object_wrapper::tr(const char *arg__1)
{
    return ::KDDockWidgets::Core::Object::tr(arg__1);
}
Object_wrapper::~Object_wrapper()
{
}

}
}
static KDDockWidgets::Core::Object *fromPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgets::Core::Object *>(ptr);
}
static KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::Object_wrapper *fromWrapperPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::Object_wrapper *>(ptr);
}
extern "C" {
void c_KDDockWidgets__Core__Object_Finalizer(void *cppObj)
{
    delete reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::Object_wrapper *>(cppObj);
}
void *c_KDDockWidgets__Core__Object__constructor_Object(void *parent_)
{
    auto parent = reinterpret_cast<KDDockWidgets::Core::Object *>(parent_);
    auto ptr = new KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::Object_wrapper(parent);
    return reinterpret_cast<void *>(ptr);
}
// objectName() const
void *c_KDDockWidgets__Core__Object__objectName(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<QString> { fromPtr(thisObj)->objectName() };
    return result;
}
// parent() const
void *c_KDDockWidgets__Core__Object__parent(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->parent();
    return result;
}
// setObjectName(const QString & arg__1)
void c_KDDockWidgets__Core__Object__setObjectName_QString(void *thisObj, const char *arg__1_)
{
    const auto arg__1 = QString::fromUtf8(arg__1_);
    fromPtr(thisObj)->setObjectName(arg__1);
    free(( char * )arg__1_);
}
// setParent(KDDockWidgets::Core::Object * parent)
void c_KDDockWidgets__Core__Object__setParent_Object(void *thisObj, void *parent_)
{
    auto parent = reinterpret_cast<KDDockWidgets::Core::Object *>(parent_);
    fromPtr(thisObj)->setParent(parent);
}
// tr(const char * arg__1)
void *c_static_KDDockWidgets__Core__Object__tr_char(const char *arg__1)
{
    const auto &result = new Dartagnan::ValueWrapper<QString> { KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::Object_wrapper::tr(arg__1) };
    free(( char * )arg__1);
    return result;
}
void c_KDDockWidgets__Core__Object__destructor(void *thisObj)
{
    delete fromPtr(thisObj);
}
void c_KDDockWidgets__Core__Object__registerVirtualMethodCallback(void *ptr, void *callback, int methodId)
{
    auto wrapper = fromWrapperPtr(ptr);
    switch (methodId) {
    }
}
}
