/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "Event_c.h"


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
Event_wrapper::Event_wrapper(KDDockWidgets::Event::Type type)
    : ::KDDockWidgets::Event(type)
{
}
void Event_wrapper::accept()
{
    ::KDDockWidgets::Event::accept();
}
void Event_wrapper::ignore()
{
    ::KDDockWidgets::Event::ignore();
}
bool Event_wrapper::isAccepted() const
{
    return ::KDDockWidgets::Event::isAccepted();
}
bool Event_wrapper::spontaneous() const
{
    return ::KDDockWidgets::Event::spontaneous();
}
KDDockWidgets::Event::Type Event_wrapper::type() const
{
    return ::KDDockWidgets::Event::type();
}
Event_wrapper::~Event_wrapper()
{
}

}
static KDDockWidgets::Event *fromPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgets::Event *>(ptr);
}
static KDDockWidgetsBindings_wrappersNS::Event_wrapper *fromWrapperPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgetsBindings_wrappersNS::Event_wrapper *>(ptr);
}
extern "C" {
void c_KDDockWidgets__Event_Finalizer(void *cppObj)
{
    delete reinterpret_cast<KDDockWidgetsBindings_wrappersNS::Event_wrapper *>(cppObj);
}
void *c_KDDockWidgets__Event__constructor_Type(int type)
{
    auto ptr = new KDDockWidgetsBindings_wrappersNS::Event_wrapper(static_cast<KDDockWidgets::Event::Type>(type));
    return reinterpret_cast<void *>(ptr);
}
// accept()
void c_KDDockWidgets__Event__accept(void *thisObj)
{
    fromPtr(thisObj)->accept();
}
// ignore()
void c_KDDockWidgets__Event__ignore(void *thisObj)
{
    fromPtr(thisObj)->ignore();
}
// isAccepted() const
bool c_KDDockWidgets__Event__isAccepted(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->isAccepted();
    return result;
}
// spontaneous() const
bool c_KDDockWidgets__Event__spontaneous(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->spontaneous();
    return result;
}
// type() const
int c_KDDockWidgets__Event__type(void *thisObj)
{
    const auto &result = int(fromPtr(thisObj)->type());
    return result;
}
void c_KDDockWidgets__Event__destructor(void *thisObj)
{
    delete fromPtr(thisObj);
}
bool c_KDDockWidgets__Event___get_m_accepted(void *thisObj)
{
    return fromPtr(thisObj)->m_accepted;
}
bool c_KDDockWidgets__Event___get_m_spontaneous(void *thisObj)
{
    return fromPtr(thisObj)->m_spontaneous;
}
void c_KDDockWidgets__Event___set_m_accepted_bool(void *thisObj, bool m_accepted_)
{
    fromPtr(thisObj)->m_accepted = m_accepted_;
}
void c_KDDockWidgets__Event___set_m_spontaneous_bool(void *thisObj, bool m_spontaneous_)
{
    fromPtr(thisObj)->m_spontaneous = m_spontaneous_;
}
}
