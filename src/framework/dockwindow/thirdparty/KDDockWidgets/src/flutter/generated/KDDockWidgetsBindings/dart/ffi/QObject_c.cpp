/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "QObject_c.h"


#include <iostream>


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
QObject_wrapper::QObject_wrapper(QObject *parent)
    : ::QObject(parent)
{
}
bool QObject_wrapper::blockSignals(bool b)
{
    return ::QObject::blockSignals(b);
}
void QObject_wrapper::deleteLater()
{
    ::QObject::deleteLater();
}
void QObject_wrapper::destroyed(QObject *arg__1)
{
    ::QObject::destroyed(arg__1);
}
bool QObject_wrapper::disconnect(const QObject *receiver, const char *member) const
{
    return ::QObject::disconnect(receiver, member);
}
bool QObject_wrapper::disconnect(const QObject *sender, const char *signal, const QObject *receiver, const char *member)
{
    return ::QObject::disconnect(sender, signal, receiver, member);
}
bool QObject_wrapper::disconnect(const char *signal, const QObject *receiver, const char *member) const
{
    return ::QObject::disconnect(signal, receiver, member);
}
void QObject_wrapper::dumpObjectInfo()
{
    ::QObject::dumpObjectInfo();
}
void QObject_wrapper::dumpObjectTree()
{
    ::QObject::dumpObjectTree();
}
bool QObject_wrapper::inherits(const char *classname) const
{
    return ::QObject::inherits(classname);
}
void QObject_wrapper::installEventFilter(QObject *filterObj)
{
    ::QObject::installEventFilter(filterObj);
}
bool QObject_wrapper::isWidgetType() const
{
    return ::QObject::isWidgetType();
}
bool QObject_wrapper::isWindowType() const
{
    return ::QObject::isWindowType();
}
void QObject_wrapper::killTimer(int id)
{
    ::QObject::killTimer(id);
}
QString QObject_wrapper::objectName() const
{
    return ::QObject::objectName();
}
QObject *QObject_wrapper::parent() const
{
    return ::QObject::parent();
}
int QObject_wrapper::receivers(const char *signal) const
{
    return ::QObject::receivers(signal);
}
void QObject_wrapper::removeEventFilter(QObject *obj)
{
    ::QObject::removeEventFilter(obj);
}
QObject *QObject_wrapper::sender() const
{
    return ::QObject::sender();
}
int QObject_wrapper::senderSignalIndex() const
{
    return ::QObject::senderSignalIndex();
}
void QObject_wrapper::setObjectName(const QString &name)
{
    ::QObject::setObjectName(name);
}
void QObject_wrapper::setParent(QObject *parent)
{
    ::QObject::setParent(parent);
}
bool QObject_wrapper::signalsBlocked() const
{
    return ::QObject::signalsBlocked();
}
int QObject_wrapper::startTimer(int interval)
{
    return ::QObject::startTimer(interval);
}
QString QObject_wrapper::tr(const char *s, const char *c, int n)
{
    return ::QObject::tr(s, c, n);
}
QObject_wrapper::~QObject_wrapper()
{
}

}
static QObject *fromPtr(void *ptr)
{
    return reinterpret_cast<QObject *>(ptr);
}
static KDDockWidgetsBindings_wrappersNS::QObject_wrapper *fromWrapperPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgetsBindings_wrappersNS::QObject_wrapper *>(ptr);
}
extern "C" {
void c_QObject_Finalizer(void *cppObj)
{
    delete reinterpret_cast<KDDockWidgetsBindings_wrappersNS::QObject_wrapper *>(cppObj);
}
void *c_QObject__constructor_QObject(void *parent_)
{
    auto parent = reinterpret_cast<QObject *>(parent_);
    auto ptr = new KDDockWidgetsBindings_wrappersNS::QObject_wrapper(parent);
    return reinterpret_cast<void *>(ptr);
}
// blockSignals(bool b)
bool c_QObject__blockSignals_bool(void *thisObj, bool b)
{
    return fromPtr(thisObj)->blockSignals(b);
}
// deleteLater()
void c_QObject__deleteLater(void *thisObj)
{
    fromPtr(thisObj)->deleteLater();
}
// destroyed(QObject * arg__1)
void c_QObject__destroyed_QObject(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<QObject *>(arg__1_);
    fromPtr(thisObj)->destroyed(arg__1);
}
// disconnect(const QObject * receiver, const char * member) const
bool c_QObject__disconnect_QObject_char(void *thisObj, void *receiver_, const char *member)
{
    auto receiver = reinterpret_cast<QObject *>(receiver_);
    return fromPtr(thisObj)->disconnect(receiver, member);
}
// disconnect(const QObject * sender, const char * signal, const QObject * receiver, const char * member)
bool c_static_QObject__disconnect_QObject_char_QObject_char(void *sender_, const char *signal, void *receiver_, const char *member)
{
    auto sender = reinterpret_cast<QObject *>(sender_);
    auto receiver = reinterpret_cast<QObject *>(receiver_);
    return KDDockWidgetsBindings_wrappersNS::QObject_wrapper::disconnect(sender, signal, receiver, member);
}
// disconnect(const char * signal, const QObject * receiver, const char * member) const
bool c_QObject__disconnect_char_QObject_char(void *thisObj, const char *signal, void *receiver_, const char *member)
{
    auto receiver = reinterpret_cast<QObject *>(receiver_);
    return fromPtr(thisObj)->disconnect(signal, receiver, member);
}
// dumpObjectInfo()
void c_QObject__dumpObjectInfo(void *thisObj)
{
    fromPtr(thisObj)->dumpObjectInfo();
}
// dumpObjectTree()
void c_QObject__dumpObjectTree(void *thisObj)
{
    fromPtr(thisObj)->dumpObjectTree();
}
// inherits(const char * classname) const
bool c_QObject__inherits_char(void *thisObj, const char *classname)
{
    return fromPtr(thisObj)->inherits(classname);
}
// installEventFilter(QObject * filterObj)
void c_QObject__installEventFilter_QObject(void *thisObj, void *filterObj_)
{
    auto filterObj = reinterpret_cast<QObject *>(filterObj_);
    fromPtr(thisObj)->installEventFilter(filterObj);
}
// isWidgetType() const
bool c_QObject__isWidgetType(void *thisObj)
{
    return fromPtr(thisObj)->isWidgetType();
}
// isWindowType() const
bool c_QObject__isWindowType(void *thisObj)
{
    return fromPtr(thisObj)->isWindowType();
}
// killTimer(int id)
void c_QObject__killTimer_int(void *thisObj, int id)
{
    fromPtr(thisObj)->killTimer(id);
}
// objectName() const
void *c_QObject__objectName(void *thisObj)
{
    return new Dartagnan::ValueWrapper<QString> { fromPtr(thisObj)->objectName() };
}
// parent() const
void *c_QObject__parent(void *thisObj)
{
    return fromPtr(thisObj)->parent();
}
// receivers(const char * signal) const
int c_QObject__receivers_char(void *thisObj, const char *signal)
{
    return fromWrapperPtr(thisObj)->receivers(signal);
}
// removeEventFilter(QObject * obj)
void c_QObject__removeEventFilter_QObject(void *thisObj, void *obj_)
{
    auto obj = reinterpret_cast<QObject *>(obj_);
    fromPtr(thisObj)->removeEventFilter(obj);
}
// sender() const
void *c_QObject__sender(void *thisObj)
{
    return fromWrapperPtr(thisObj)->sender();
}
// senderSignalIndex() const
int c_QObject__senderSignalIndex(void *thisObj)
{
    return fromWrapperPtr(thisObj)->senderSignalIndex();
}
// setObjectName(const QString & name)
void c_QObject__setObjectName_QString(void *thisObj, const char *name_)
{
    const auto name = QString::fromUtf8(name_);
    fromPtr(thisObj)->setObjectName(name);
}
// setParent(QObject * parent)
void c_QObject__setParent_QObject(void *thisObj, void *parent_)
{
    auto parent = reinterpret_cast<QObject *>(parent_);
    fromPtr(thisObj)->setParent(parent);
}
// signalsBlocked() const
bool c_QObject__signalsBlocked(void *thisObj)
{
    return fromPtr(thisObj)->signalsBlocked();
}
// startTimer(int interval)
int c_QObject__startTimer_int(void *thisObj, int interval)
{
    return fromPtr(thisObj)->startTimer(interval);
}
// tr(const char * s, const char * c, int n)
void *c_static_QObject__tr_char_char_int(const char *s, const char *c, int n)
{
    return new Dartagnan::ValueWrapper<QString> { KDDockWidgetsBindings_wrappersNS::QObject_wrapper::tr(s, c, n) };
}
void c_QObject__destructor(void *thisObj)
{
    delete fromPtr(thisObj);
}
void c_QObject__registerVirtualMethodCallback(void *ptr, void *callback, int methodId)
{
    auto wrapper = fromWrapperPtr(ptr);
    switch (methodId) {
    }
}
}
