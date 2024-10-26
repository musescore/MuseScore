/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "KDDockWidgetsBindings_exports.h"
#include <qobject.h>
#include <qstring.h>

namespace KDDockWidgetsBindings_wrappersNS {
class QObject_wrapper : public ::QObject
{
public:
    ~QObject_wrapper();
    QObject_wrapper(QObject *parent = nullptr);
    bool blockSignals(bool b);
    void deleteLater();
    void destroyed(QObject *arg__1 = nullptr);
    bool disconnect(const QObject *receiver, const char *member = nullptr) const;
    static bool disconnect(const QObject *sender, const char *signal, const QObject *receiver, const char *member);
    bool disconnect(const char *signal = nullptr, const QObject *receiver = nullptr, const char *member = nullptr) const;
    void dumpObjectInfo();
    void dumpObjectTree();
    bool inherits(const char *classname) const;
    void installEventFilter(QObject *filterObj);
    bool isWidgetType() const;
    bool isWindowType() const;
    void killTimer(int id);
    QString objectName() const;
    QObject *parent() const;
    int receivers(const char *signal) const;
    void removeEventFilter(QObject *obj);
    QObject *sender() const;
    int senderSignalIndex() const;
    void setObjectName(const QString &name);
    void setParent(QObject *parent);
    bool signalsBlocked() const;
    int startTimer(int interval);
    static QString tr(const char *s, const char *c, int n);
};
}
extern "C" {
// QObject::QObject(QObject * parent)
KDDockWidgetsBindings_EXPORT void *c_QObject__constructor_QObject(void *parent_);
// QObject::blockSignals(bool b)
KDDockWidgetsBindings_EXPORT bool c_QObject__blockSignals_bool(void *thisObj, bool b);
// QObject::deleteLater()
KDDockWidgetsBindings_EXPORT void c_QObject__deleteLater(void *thisObj);
// QObject::destroyed(QObject * arg__1)
KDDockWidgetsBindings_EXPORT void c_QObject__destroyed_QObject(void *thisObj, void *arg__1_);
// QObject::disconnect(const QObject * receiver, const char * member) const
KDDockWidgetsBindings_EXPORT bool c_QObject__disconnect_QObject_char(void *thisObj, void *receiver_, const char *member);
// QObject::disconnect(const QObject * sender, const char * signal, const QObject * receiver, const char * member)
KDDockWidgetsBindings_EXPORT bool c_static_QObject__disconnect_QObject_char_QObject_char(void *sender_, const char *signal, void *receiver_, const char *member);
// QObject::disconnect(const char * signal, const QObject * receiver, const char * member) const
KDDockWidgetsBindings_EXPORT bool c_QObject__disconnect_char_QObject_char(void *thisObj, const char *signal, void *receiver_, const char *member);
// QObject::dumpObjectInfo()
KDDockWidgetsBindings_EXPORT void c_QObject__dumpObjectInfo(void *thisObj);
// QObject::dumpObjectTree()
KDDockWidgetsBindings_EXPORT void c_QObject__dumpObjectTree(void *thisObj);
// QObject::inherits(const char * classname) const
KDDockWidgetsBindings_EXPORT bool c_QObject__inherits_char(void *thisObj, const char *classname);
// QObject::installEventFilter(QObject * filterObj)
KDDockWidgetsBindings_EXPORT void c_QObject__installEventFilter_QObject(void *thisObj, void *filterObj_);
// QObject::isWidgetType() const
KDDockWidgetsBindings_EXPORT bool c_QObject__isWidgetType(void *thisObj);
// QObject::isWindowType() const
KDDockWidgetsBindings_EXPORT bool c_QObject__isWindowType(void *thisObj);
// QObject::killTimer(int id)
KDDockWidgetsBindings_EXPORT void c_QObject__killTimer_int(void *thisObj, int id);
// QObject::objectName() const
KDDockWidgetsBindings_EXPORT void *c_QObject__objectName(void *thisObj);
// QObject::parent() const
KDDockWidgetsBindings_EXPORT void *c_QObject__parent(void *thisObj);
// QObject::receivers(const char * signal) const
KDDockWidgetsBindings_EXPORT int c_QObject__receivers_char(void *thisObj, const char *signal);
// QObject::removeEventFilter(QObject * obj)
KDDockWidgetsBindings_EXPORT void c_QObject__removeEventFilter_QObject(void *thisObj, void *obj_);
// QObject::sender() const
KDDockWidgetsBindings_EXPORT void *c_QObject__sender(void *thisObj);
// QObject::senderSignalIndex() const
KDDockWidgetsBindings_EXPORT int c_QObject__senderSignalIndex(void *thisObj);
// QObject::setObjectName(const QString & name)
KDDockWidgetsBindings_EXPORT void c_QObject__setObjectName_QString(void *thisObj, const char *name_);
// QObject::setParent(QObject * parent)
KDDockWidgetsBindings_EXPORT void c_QObject__setParent_QObject(void *thisObj, void *parent_);
// QObject::signalsBlocked() const
KDDockWidgetsBindings_EXPORT bool c_QObject__signalsBlocked(void *thisObj);
// QObject::startTimer(int interval)
KDDockWidgetsBindings_EXPORT int c_QObject__startTimer_int(void *thisObj, int interval);
// QObject::tr(const char * s, const char * c, int n)
KDDockWidgetsBindings_EXPORT void *c_static_QObject__tr_char_char_int(const char *s, const char *c, int n);
KDDockWidgetsBindings_EXPORT void c_QObject__destructor(void *thisObj);
KDDockWidgetsBindings_EXPORT void c_QObject__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
KDDockWidgetsBindings_EXPORT void c_QObject_Finalizer(void *cppObj);
}
