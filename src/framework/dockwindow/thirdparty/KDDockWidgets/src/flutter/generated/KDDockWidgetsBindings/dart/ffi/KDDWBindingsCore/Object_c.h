/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <Object_p.h>
#include <string_p.h>

namespace KDDockWidgetsBindings_wrappersNS {
namespace KDDWBindingsCore {
class Object_wrapper : public ::KDDockWidgets::Core::Object
{
public:
    ~Object_wrapper();
    Object_wrapper(KDDockWidgets::Core::Object *parent = nullptr);
    QString objectName() const;
    KDDockWidgets::Core::Object *parent() const;
    void setObjectName(const QString &arg__1);
    void setParent(KDDockWidgets::Core::Object *parent);
    static QString tr(const char *arg__1);
};
}
}
extern "C" {
// KDDockWidgets::Core::Object::Object(KDDockWidgets::Core::Object * parent)
DOCKS_EXPORT void *c_KDDockWidgets__Core__Object__constructor_Object(void *parent_);
// KDDockWidgets::Core::Object::objectName() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Object__objectName(void *thisObj);
// KDDockWidgets::Core::Object::parent() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Object__parent(void *thisObj);
// KDDockWidgets::Core::Object::setObjectName(const QString & arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__Object__setObjectName_QString(void *thisObj, const char *arg__1_);
// KDDockWidgets::Core::Object::setParent(KDDockWidgets::Core::Object * parent)
DOCKS_EXPORT void c_KDDockWidgets__Core__Object__setParent_Object(void *thisObj, void *parent_);
// KDDockWidgets::Core::Object::tr(const char * arg__1)
DOCKS_EXPORT void *c_static_KDDockWidgets__Core__Object__tr_char(const char *arg__1);
DOCKS_EXPORT void c_KDDockWidgets__Core__Object__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__Object__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
DOCKS_EXPORT void c_KDDockWidgets__Core__Object_Finalizer(void *cppObj);
}
