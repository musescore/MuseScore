/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "Size_c.h"


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
Size_wrapper::Size_wrapper()
    : ::KDDockWidgets::Size()
{
}
Size_wrapper::Size_wrapper(int width, int height)
    : ::KDDockWidgets::Size(width, height)
{
}
KDDockWidgets::Size Size_wrapper::boundedTo(KDDockWidgets::Size sz) const
{
    return ::KDDockWidgets::Size::boundedTo(sz);
}
KDDockWidgets::Size Size_wrapper::expandedTo(KDDockWidgets::Size sz) const
{
    return ::KDDockWidgets::Size::expandedTo(sz);
}
int Size_wrapper::height() const
{
    return ::KDDockWidgets::Size::height();
}
bool Size_wrapper::isEmpty() const
{
    return ::KDDockWidgets::Size::isEmpty();
}
bool Size_wrapper::isNull() const
{
    return ::KDDockWidgets::Size::isNull();
}
bool Size_wrapper::isValid() const
{
    return ::KDDockWidgets::Size::isValid();
}
void Size_wrapper::setHeight(int h)
{
    ::KDDockWidgets::Size::setHeight(h);
}
void Size_wrapper::setWidth(int w)
{
    ::KDDockWidgets::Size::setWidth(w);
}
int Size_wrapper::width() const
{
    return ::KDDockWidgets::Size::width();
}
Size_wrapper::~Size_wrapper()
{
}

}
static KDDockWidgets::Size *fromPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgets::Size *>(ptr);
}
static KDDockWidgetsBindings_wrappersNS::Size_wrapper *fromWrapperPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgetsBindings_wrappersNS::Size_wrapper *>(ptr);
}
extern "C" {
void c_KDDockWidgets__Size_Finalizer(void *cppObj)
{
    delete reinterpret_cast<KDDockWidgetsBindings_wrappersNS::Size_wrapper *>(cppObj);
}
void *c_KDDockWidgets__Size__constructor()
{
    auto ptr = new KDDockWidgetsBindings_wrappersNS::Size_wrapper();
    return reinterpret_cast<void *>(ptr);
}
void *c_KDDockWidgets__Size__constructor_int_int(int width, int height)
{
    auto ptr = new KDDockWidgetsBindings_wrappersNS::Size_wrapper(width, height);
    return reinterpret_cast<void *>(ptr);
}
// boundedTo(KDDockWidgets::Size sz) const
void *c_KDDockWidgets__Size__boundedTo_Size(void *thisObj, void *sz_)
{
    assert(sz_);
    auto &sz = *reinterpret_cast<KDDockWidgets::Size *>(sz_);
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Size> { fromPtr(thisObj)->boundedTo(sz) };
    return result;
}
// expandedTo(KDDockWidgets::Size sz) const
void *c_KDDockWidgets__Size__expandedTo_Size(void *thisObj, void *sz_)
{
    assert(sz_);
    auto &sz = *reinterpret_cast<KDDockWidgets::Size *>(sz_);
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Size> { fromPtr(thisObj)->expandedTo(sz) };
    return result;
}
// height() const
int c_KDDockWidgets__Size__height(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->height();
    return result;
}
// isEmpty() const
bool c_KDDockWidgets__Size__isEmpty(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->isEmpty();
    return result;
}
// isNull() const
bool c_KDDockWidgets__Size__isNull(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->isNull();
    return result;
}
// isValid() const
bool c_KDDockWidgets__Size__isValid(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->isValid();
    return result;
}
// setHeight(int h)
void c_KDDockWidgets__Size__setHeight_int(void *thisObj, int h)
{
    fromPtr(thisObj)->setHeight(h);
}
// setWidth(int w)
void c_KDDockWidgets__Size__setWidth_int(void *thisObj, int w)
{
    fromPtr(thisObj)->setWidth(w);
}
// width() const
int c_KDDockWidgets__Size__width(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->width();
    return result;
}
void c_KDDockWidgets__Size__destructor(void *thisObj)
{
    delete fromPtr(thisObj);
}
}
