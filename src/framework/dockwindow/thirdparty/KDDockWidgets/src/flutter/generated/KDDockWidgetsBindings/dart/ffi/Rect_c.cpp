/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "Rect_c.h"


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
Rect_wrapper::Rect_wrapper()
    : ::KDDockWidgets::Rect()
{
}
Rect_wrapper::Rect_wrapper(KDDockWidgets::Point pos, KDDockWidgets::Size size)
    : ::KDDockWidgets::Rect(pos, size)
{
}
Rect_wrapper::Rect_wrapper(int x, int y, int width, int height)
    : ::KDDockWidgets::Rect(x, y, width, height)
{
}
void Rect_wrapper::adjust(int l, int t, int r, int b)
{
    ::KDDockWidgets::Rect::adjust(l, t, r, b);
}
KDDockWidgets::Rect Rect_wrapper::adjusted(int l, int t, int r, int b) const
{
    return ::KDDockWidgets::Rect::adjusted(l, t, r, b);
}
int Rect_wrapper::bottom() const
{
    return ::KDDockWidgets::Rect::bottom();
}
KDDockWidgets::Point Rect_wrapper::bottomLeft() const
{
    return ::KDDockWidgets::Rect::bottomLeft();
}
KDDockWidgets::Point Rect_wrapper::bottomRight() const
{
    return ::KDDockWidgets::Rect::bottomRight();
}
KDDockWidgets::Point Rect_wrapper::center() const
{
    return ::KDDockWidgets::Rect::center();
}
bool Rect_wrapper::contains(KDDockWidgets::Point pt) const
{
    return ::KDDockWidgets::Rect::contains(pt);
}
bool Rect_wrapper::contains(KDDockWidgets::Rect other) const
{
    return ::KDDockWidgets::Rect::contains(other);
}
int Rect_wrapper::height() const
{
    return ::KDDockWidgets::Rect::height();
}
KDDockWidgets::Rect Rect_wrapper::intersected(KDDockWidgets::Rect other) const
{
    return ::KDDockWidgets::Rect::intersected(other);
}
bool Rect_wrapper::intersects(KDDockWidgets::Rect other) const
{
    return ::KDDockWidgets::Rect::intersects(other);
}
bool Rect_wrapper::isEmpty() const
{
    return ::KDDockWidgets::Rect::isEmpty();
}
bool Rect_wrapper::isNull() const
{
    return ::KDDockWidgets::Rect::isNull();
}
bool Rect_wrapper::isValid() const
{
    return ::KDDockWidgets::Rect::isValid();
}
int Rect_wrapper::left() const
{
    return ::KDDockWidgets::Rect::left();
}
KDDockWidgets::Rect Rect_wrapper::marginsAdded(KDDockWidgets::Margins m) const
{
    return ::KDDockWidgets::Rect::marginsAdded(m);
}
void Rect_wrapper::moveBottom(int b)
{
    ::KDDockWidgets::Rect::moveBottom(b);
}
void Rect_wrapper::moveCenter(KDDockWidgets::Point pt)
{
    ::KDDockWidgets::Rect::moveCenter(pt);
}
void Rect_wrapper::moveLeft(int x)
{
    ::KDDockWidgets::Rect::moveLeft(x);
}
void Rect_wrapper::moveRight(int r)
{
    ::KDDockWidgets::Rect::moveRight(r);
}
void Rect_wrapper::moveTo(KDDockWidgets::Point pt)
{
    ::KDDockWidgets::Rect::moveTo(pt);
}
void Rect_wrapper::moveTo(int x, int y)
{
    ::KDDockWidgets::Rect::moveTo(x, y);
}
void Rect_wrapper::moveTop(int y)
{
    ::KDDockWidgets::Rect::moveTop(y);
}
void Rect_wrapper::moveTopLeft(KDDockWidgets::Point pt)
{
    ::KDDockWidgets::Rect::moveTopLeft(pt);
}
KDDockWidgets::Point Rect_wrapper::pos() const
{
    return ::KDDockWidgets::Rect::pos();
}
int Rect_wrapper::right() const
{
    return ::KDDockWidgets::Rect::right();
}
void Rect_wrapper::setBottom(int b)
{
    ::KDDockWidgets::Rect::setBottom(b);
}
void Rect_wrapper::setHeight(int h)
{
    ::KDDockWidgets::Rect::setHeight(h);
}
void Rect_wrapper::setLeft(int x)
{
    ::KDDockWidgets::Rect::setLeft(x);
}
void Rect_wrapper::setRight(int r)
{
    ::KDDockWidgets::Rect::setRight(r);
}
void Rect_wrapper::setSize(KDDockWidgets::Size sz)
{
    ::KDDockWidgets::Rect::setSize(sz);
}
void Rect_wrapper::setTop(int y)
{
    ::KDDockWidgets::Rect::setTop(y);
}
void Rect_wrapper::setTopLeft(KDDockWidgets::Point pt)
{
    ::KDDockWidgets::Rect::setTopLeft(pt);
}
void Rect_wrapper::setWidth(int w)
{
    ::KDDockWidgets::Rect::setWidth(w);
}
void Rect_wrapper::setX(int x)
{
    ::KDDockWidgets::Rect::setX(x);
}
void Rect_wrapper::setY(int y)
{
    ::KDDockWidgets::Rect::setY(y);
}
KDDockWidgets::Size Rect_wrapper::size() const
{
    return ::KDDockWidgets::Rect::size();
}
int Rect_wrapper::top() const
{
    return ::KDDockWidgets::Rect::top();
}
KDDockWidgets::Point Rect_wrapper::topLeft() const
{
    return ::KDDockWidgets::Rect::topLeft();
}
KDDockWidgets::Point Rect_wrapper::topRight() const
{
    return ::KDDockWidgets::Rect::topRight();
}
void Rect_wrapper::translate(KDDockWidgets::Point pt)
{
    ::KDDockWidgets::Rect::translate(pt);
}
int Rect_wrapper::width() const
{
    return ::KDDockWidgets::Rect::width();
}
int Rect_wrapper::x() const
{
    return ::KDDockWidgets::Rect::x();
}
int Rect_wrapper::y() const
{
    return ::KDDockWidgets::Rect::y();
}
Rect_wrapper::~Rect_wrapper()
{
}

}
static KDDockWidgets::Rect *fromPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgets::Rect *>(ptr);
}
static KDDockWidgetsBindings_wrappersNS::Rect_wrapper *fromWrapperPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgetsBindings_wrappersNS::Rect_wrapper *>(ptr);
}
extern "C" {
void c_KDDockWidgets__Rect_Finalizer(void *cppObj)
{
    delete reinterpret_cast<KDDockWidgetsBindings_wrappersNS::Rect_wrapper *>(cppObj);
}
void *c_KDDockWidgets__Rect__constructor()
{
    auto ptr = new KDDockWidgetsBindings_wrappersNS::Rect_wrapper();
    return reinterpret_cast<void *>(ptr);
}
void *c_KDDockWidgets__Rect__constructor_Point_Size(void *pos_, void *size_)
{
    assert(pos_);
    auto &pos = *reinterpret_cast<KDDockWidgets::Point *>(pos_);
    assert(size_);
    auto &size = *reinterpret_cast<KDDockWidgets::Size *>(size_);
    auto ptr = new KDDockWidgetsBindings_wrappersNS::Rect_wrapper(pos, size);
    return reinterpret_cast<void *>(ptr);
}
void *c_KDDockWidgets__Rect__constructor_int_int_int_int(int x, int y, int width, int height)
{
    auto ptr = new KDDockWidgetsBindings_wrappersNS::Rect_wrapper(x, y, width, height);
    return reinterpret_cast<void *>(ptr);
}
// adjust(int l, int t, int r, int b)
void c_KDDockWidgets__Rect__adjust_int_int_int_int(void *thisObj, int l, int t, int r, int b)
{
    fromPtr(thisObj)->adjust(l, t, r, b);
}
// adjusted(int l, int t, int r, int b) const
void *c_KDDockWidgets__Rect__adjusted_int_int_int_int(void *thisObj, int l, int t, int r, int b)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Rect> { fromPtr(thisObj)->adjusted(l, t, r, b) };
    return result;
}
// bottom() const
int c_KDDockWidgets__Rect__bottom(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->bottom();
    return result;
}
// bottomLeft() const
void *c_KDDockWidgets__Rect__bottomLeft(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Point> { fromPtr(thisObj)->bottomLeft() };
    return result;
}
// bottomRight() const
void *c_KDDockWidgets__Rect__bottomRight(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Point> { fromPtr(thisObj)->bottomRight() };
    return result;
}
// center() const
void *c_KDDockWidgets__Rect__center(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Point> { fromPtr(thisObj)->center() };
    return result;
}
// contains(KDDockWidgets::Point pt) const
bool c_KDDockWidgets__Rect__contains_Point(void *thisObj, void *pt_)
{
    assert(pt_);
    auto &pt = *reinterpret_cast<KDDockWidgets::Point *>(pt_);
    const auto &result = fromPtr(thisObj)->contains(pt);
    return result;
}
// contains(KDDockWidgets::Rect other) const
bool c_KDDockWidgets__Rect__contains_Rect(void *thisObj, void *other_)
{
    assert(other_);
    auto &other = *reinterpret_cast<KDDockWidgets::Rect *>(other_);
    const auto &result = fromPtr(thisObj)->contains(other);
    return result;
}
// height() const
int c_KDDockWidgets__Rect__height(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->height();
    return result;
}
// intersected(KDDockWidgets::Rect other) const
void *c_KDDockWidgets__Rect__intersected_Rect(void *thisObj, void *other_)
{
    assert(other_);
    auto &other = *reinterpret_cast<KDDockWidgets::Rect *>(other_);
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Rect> { fromPtr(thisObj)->intersected(other) };
    return result;
}
// intersects(KDDockWidgets::Rect other) const
bool c_KDDockWidgets__Rect__intersects_Rect(void *thisObj, void *other_)
{
    assert(other_);
    auto &other = *reinterpret_cast<KDDockWidgets::Rect *>(other_);
    const auto &result = fromPtr(thisObj)->intersects(other);
    return result;
}
// isEmpty() const
bool c_KDDockWidgets__Rect__isEmpty(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->isEmpty();
    return result;
}
// isNull() const
bool c_KDDockWidgets__Rect__isNull(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->isNull();
    return result;
}
// isValid() const
bool c_KDDockWidgets__Rect__isValid(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->isValid();
    return result;
}
// left() const
int c_KDDockWidgets__Rect__left(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->left();
    return result;
}
// marginsAdded(KDDockWidgets::Margins m) const
void *c_KDDockWidgets__Rect__marginsAdded_Margins(void *thisObj, void *m_)
{
    assert(m_);
    auto &m = *reinterpret_cast<KDDockWidgets::Margins *>(m_);
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Rect> { fromPtr(thisObj)->marginsAdded(m) };
    return result;
}
// moveBottom(int b)
void c_KDDockWidgets__Rect__moveBottom_int(void *thisObj, int b)
{
    fromPtr(thisObj)->moveBottom(b);
}
// moveCenter(KDDockWidgets::Point pt)
void c_KDDockWidgets__Rect__moveCenter_Point(void *thisObj, void *pt_)
{
    assert(pt_);
    auto &pt = *reinterpret_cast<KDDockWidgets::Point *>(pt_);
    fromPtr(thisObj)->moveCenter(pt);
}
// moveLeft(int x)
void c_KDDockWidgets__Rect__moveLeft_int(void *thisObj, int x)
{
    fromPtr(thisObj)->moveLeft(x);
}
// moveRight(int r)
void c_KDDockWidgets__Rect__moveRight_int(void *thisObj, int r)
{
    fromPtr(thisObj)->moveRight(r);
}
// moveTo(KDDockWidgets::Point pt)
void c_KDDockWidgets__Rect__moveTo_Point(void *thisObj, void *pt_)
{
    assert(pt_);
    auto &pt = *reinterpret_cast<KDDockWidgets::Point *>(pt_);
    fromPtr(thisObj)->moveTo(pt);
}
// moveTo(int x, int y)
void c_KDDockWidgets__Rect__moveTo_int_int(void *thisObj, int x, int y)
{
    fromPtr(thisObj)->moveTo(x, y);
}
// moveTop(int y)
void c_KDDockWidgets__Rect__moveTop_int(void *thisObj, int y)
{
    fromPtr(thisObj)->moveTop(y);
}
// moveTopLeft(KDDockWidgets::Point pt)
void c_KDDockWidgets__Rect__moveTopLeft_Point(void *thisObj, void *pt_)
{
    assert(pt_);
    auto &pt = *reinterpret_cast<KDDockWidgets::Point *>(pt_);
    fromPtr(thisObj)->moveTopLeft(pt);
}
// pos() const
void *c_KDDockWidgets__Rect__pos(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Point> { fromPtr(thisObj)->pos() };
    return result;
}
// right() const
int c_KDDockWidgets__Rect__right(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->right();
    return result;
}
// setBottom(int b)
void c_KDDockWidgets__Rect__setBottom_int(void *thisObj, int b)
{
    fromPtr(thisObj)->setBottom(b);
}
// setHeight(int h)
void c_KDDockWidgets__Rect__setHeight_int(void *thisObj, int h)
{
    fromPtr(thisObj)->setHeight(h);
}
// setLeft(int x)
void c_KDDockWidgets__Rect__setLeft_int(void *thisObj, int x)
{
    fromPtr(thisObj)->setLeft(x);
}
// setRight(int r)
void c_KDDockWidgets__Rect__setRight_int(void *thisObj, int r)
{
    fromPtr(thisObj)->setRight(r);
}
// setSize(KDDockWidgets::Size sz)
void c_KDDockWidgets__Rect__setSize_Size(void *thisObj, void *sz_)
{
    assert(sz_);
    auto &sz = *reinterpret_cast<KDDockWidgets::Size *>(sz_);
    fromPtr(thisObj)->setSize(sz);
}
// setTop(int y)
void c_KDDockWidgets__Rect__setTop_int(void *thisObj, int y)
{
    fromPtr(thisObj)->setTop(y);
}
// setTopLeft(KDDockWidgets::Point pt)
void c_KDDockWidgets__Rect__setTopLeft_Point(void *thisObj, void *pt_)
{
    assert(pt_);
    auto &pt = *reinterpret_cast<KDDockWidgets::Point *>(pt_);
    fromPtr(thisObj)->setTopLeft(pt);
}
// setWidth(int w)
void c_KDDockWidgets__Rect__setWidth_int(void *thisObj, int w)
{
    fromPtr(thisObj)->setWidth(w);
}
// setX(int x)
void c_KDDockWidgets__Rect__setX_int(void *thisObj, int x)
{
    fromPtr(thisObj)->setX(x);
}
// setY(int y)
void c_KDDockWidgets__Rect__setY_int(void *thisObj, int y)
{
    fromPtr(thisObj)->setY(y);
}
// size() const
void *c_KDDockWidgets__Rect__size(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Size> { fromPtr(thisObj)->size() };
    return result;
}
// top() const
int c_KDDockWidgets__Rect__top(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->top();
    return result;
}
// topLeft() const
void *c_KDDockWidgets__Rect__topLeft(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Point> { fromPtr(thisObj)->topLeft() };
    return result;
}
// topRight() const
void *c_KDDockWidgets__Rect__topRight(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Point> { fromPtr(thisObj)->topRight() };
    return result;
}
// translate(KDDockWidgets::Point pt)
void c_KDDockWidgets__Rect__translate_Point(void *thisObj, void *pt_)
{
    assert(pt_);
    auto &pt = *reinterpret_cast<KDDockWidgets::Point *>(pt_);
    fromPtr(thisObj)->translate(pt);
}
// width() const
int c_KDDockWidgets__Rect__width(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->width();
    return result;
}
// x() const
int c_KDDockWidgets__Rect__x(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->x();
    return result;
}
// y() const
int c_KDDockWidgets__Rect__y(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->y();
    return result;
}
void c_KDDockWidgets__Rect__destructor(void *thisObj)
{
    delete fromPtr(thisObj);
}
}
