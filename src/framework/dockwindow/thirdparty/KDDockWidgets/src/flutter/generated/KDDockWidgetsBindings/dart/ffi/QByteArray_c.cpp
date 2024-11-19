/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "QByteArray_c.h"


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
QByteArray_wrapper::QByteArray_wrapper()
    : ::QByteArray()
{
}
const char *QByteArray_wrapper::constData() const
{
    return ::QByteArray::constData();
}
bool QByteArray_wrapper::isEmpty() const
{
    return ::QByteArray::isEmpty();
}
QByteArray_wrapper::~QByteArray_wrapper()
{
}

}
static QByteArray *fromPtr(void *ptr)
{
    return reinterpret_cast<QByteArray *>(ptr);
}
static KDDockWidgetsBindings_wrappersNS::QByteArray_wrapper *fromWrapperPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgetsBindings_wrappersNS::QByteArray_wrapper *>(ptr);
}
extern "C" {
void c_QByteArray_Finalizer(void *cppObj)
{
    delete reinterpret_cast<KDDockWidgetsBindings_wrappersNS::QByteArray_wrapper *>(cppObj);
}
void *c_QByteArray__constructor()
{
    auto ptr = new KDDockWidgetsBindings_wrappersNS::QByteArray_wrapper();
    return reinterpret_cast<void *>(ptr);
}
// constData() const
const char *c_QByteArray__constData(void *thisObj)
{
    return fromPtr(thisObj)->constData();
}
// isEmpty() const
bool c_QByteArray__isEmpty(void *thisObj)
{
    return fromPtr(thisObj)->isEmpty();
}
void c_QByteArray__destructor(void *thisObj)
{
    delete fromPtr(thisObj);
}
}
