/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "QString_c.h"


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
QString_wrapper::QString_wrapper()
    : ::QString()
{
}
QString_wrapper::QString_wrapper(const char *str)
    : ::QString(str)
{
}
const char *QString_wrapper::data() const
{
    return ::QString::data();
}
QString QString_wrapper::fromUtf8(const char *str)
{
    return ::QString::fromUtf8(str);
}
bool QString_wrapper::isEmpty() const
{
    return ::QString::isEmpty();
}
QString_wrapper::~QString_wrapper()
{
}

}
static QString *fromPtr(void *ptr)
{
    return reinterpret_cast<QString *>(ptr);
}
static KDDockWidgetsBindings_wrappersNS::QString_wrapper *fromWrapperPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgetsBindings_wrappersNS::QString_wrapper *>(ptr);
}
extern "C" {
void c_QString_Finalizer(void *cppObj)
{
    delete reinterpret_cast<KDDockWidgetsBindings_wrappersNS::QString_wrapper *>(cppObj);
}
void *c_QString__constructor()
{
    auto ptr = new KDDockWidgetsBindings_wrappersNS::QString_wrapper();
    return reinterpret_cast<void *>(ptr);
}
void *c_QString__constructor_char(const char *str)
{
    auto ptr = new KDDockWidgetsBindings_wrappersNS::QString_wrapper(str);
    return reinterpret_cast<void *>(ptr);
}
// data() const
const char *c_QString__data(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->data();
    return result;
}
// fromUtf8(const char * str)
void *c_static_QString__fromUtf8_char(const char *str)
{
    const auto &result = new Dartagnan::ValueWrapper<QString> { KDDockWidgetsBindings_wrappersNS::QString_wrapper::fromUtf8(str) };
    free(( char * )str);
    return result;
}
// isEmpty() const
bool c_QString__isEmpty(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->isEmpty();
    return result;
}
void c_QString__destructor(void *thisObj)
{
    delete fromPtr(thisObj);
}
}
