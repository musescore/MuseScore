/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "Margins_c.h"


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
Margins_wrapper::Margins_wrapper()
    : ::KDDockWidgets::Margins()
{
}
Margins_wrapper::Margins_wrapper(int l, int t, int r, int b)
    : ::KDDockWidgets::Margins(l, t, r, b)
{
}
int Margins_wrapper::bottom() const
{
    return ::KDDockWidgets::Margins::bottom();
}
int Margins_wrapper::left() const
{
    return ::KDDockWidgets::Margins::left();
}
int Margins_wrapper::right() const
{
    return ::KDDockWidgets::Margins::right();
}
int Margins_wrapper::top() const
{
    return ::KDDockWidgets::Margins::top();
}
Margins_wrapper::~Margins_wrapper()
{
}

}
static KDDockWidgets::Margins *fromPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgets::Margins *>(ptr);
}
static KDDockWidgetsBindings_wrappersNS::Margins_wrapper *fromWrapperPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgetsBindings_wrappersNS::Margins_wrapper *>(ptr);
}
extern "C" {
void c_KDDockWidgets__Margins_Finalizer(void *cppObj)
{
    delete reinterpret_cast<KDDockWidgetsBindings_wrappersNS::Margins_wrapper *>(cppObj);
}
void *c_KDDockWidgets__Margins__constructor()
{
    auto ptr = new KDDockWidgetsBindings_wrappersNS::Margins_wrapper();
    return reinterpret_cast<void *>(ptr);
}
void *c_KDDockWidgets__Margins__constructor_int_int_int_int(int l, int t, int r, int b)
{
    auto ptr = new KDDockWidgetsBindings_wrappersNS::Margins_wrapper(l, t, r, b);
    return reinterpret_cast<void *>(ptr);
}
// bottom() const
int c_KDDockWidgets__Margins__bottom(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->bottom();
    return result;
}
// left() const
int c_KDDockWidgets__Margins__left(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->left();
    return result;
}
// right() const
int c_KDDockWidgets__Margins__right(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->right();
    return result;
}
// top() const
int c_KDDockWidgets__Margins__top(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->top();
    return result;
}
void c_KDDockWidgets__Margins__destructor(void *thisObj)
{
    delete fromPtr(thisObj);
}
}
