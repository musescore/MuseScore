/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "Point_c.h"


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
Point_wrapper::Point_wrapper()
    : ::KDDockWidgets::Point()
{
}
Point_wrapper::Point_wrapper(int x, int y)
    : ::KDDockWidgets::Point(x, y)
{
}
bool Point_wrapper::isNull() const
{
    return ::KDDockWidgets::Point::isNull();
}
int Point_wrapper::manhattanLength() const
{
    return ::KDDockWidgets::Point::manhattanLength();
}
void Point_wrapper::setX(int x)
{
    ::KDDockWidgets::Point::setX(x);
}
void Point_wrapper::setY(int y)
{
    ::KDDockWidgets::Point::setY(y);
}
int Point_wrapper::x() const
{
    return ::KDDockWidgets::Point::x();
}
int Point_wrapper::y() const
{
    return ::KDDockWidgets::Point::y();
}
Point_wrapper::~Point_wrapper()
{
}

}
static KDDockWidgets::Point *fromPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgets::Point *>(ptr);
}
static KDDockWidgetsBindings_wrappersNS::Point_wrapper *fromWrapperPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgetsBindings_wrappersNS::Point_wrapper *>(ptr);
}
extern "C" {
void c_KDDockWidgets__Point_Finalizer(void *cppObj)
{
    delete reinterpret_cast<KDDockWidgetsBindings_wrappersNS::Point_wrapper *>(cppObj);
}
void *c_KDDockWidgets__Point__constructor()
{
    auto ptr = new KDDockWidgetsBindings_wrappersNS::Point_wrapper();
    return reinterpret_cast<void *>(ptr);
}
void *c_KDDockWidgets__Point__constructor_int_int(int x, int y)
{
    auto ptr = new KDDockWidgetsBindings_wrappersNS::Point_wrapper(x, y);
    return reinterpret_cast<void *>(ptr);
}
// isNull() const
bool c_KDDockWidgets__Point__isNull(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->isNull();
    return result;
}
// manhattanLength() const
int c_KDDockWidgets__Point__manhattanLength(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->manhattanLength();
    return result;
}
// setX(int x)
void c_KDDockWidgets__Point__setX_int(void *thisObj, int x)
{
    fromPtr(thisObj)->setX(x);
}
// setY(int y)
void c_KDDockWidgets__Point__setY_int(void *thisObj, int y)
{
    fromPtr(thisObj)->setY(y);
}
// x() const
int c_KDDockWidgets__Point__x(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->x();
    return result;
}
// y() const
int c_KDDockWidgets__Point__y(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->y();
    return result;
}
void c_KDDockWidgets__Point__destructor(void *thisObj)
{
    delete fromPtr(thisObj);
}
}
