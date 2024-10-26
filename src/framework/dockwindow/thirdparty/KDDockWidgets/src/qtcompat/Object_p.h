/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

/// Implements a QObject replacement for Flutter
/// Objects in core/ still use QObject, so we minimize changes for existing Qt users
/// but for flutter, we use this replacement

#pragma once

#include "kddockwidgets/docks_export.h"
#include "enums_p.h"
#include "string_p.h"

#include <vector>

#include <kdbindings/signal.h>

namespace KDDockWidgets {

namespace Core {

class DOCKS_EXPORT_FOR_UNIT_TESTS Object
{
public:
    explicit Object(Object *parent = nullptr);
    virtual ~Object();

    void setParent(Object *parent);
    Object *parent() const;

    QString objectName() const;
    void setObjectName(const QString &);

    static QString tr(const char *);

    KDBindings::Signal<> aboutToBeDeleted;

private:
    void removeChild(Object *child);
    void addChild(Object *child);

    Object *m_parent = nullptr;
    std::vector<Object *> m_children;
    QString m_name;
};

}

template<typename T>
inline T object_cast(Core::Object *o)
{
    return dynamic_cast<T>(o);
}

template<typename T>
inline T object_cast(const Core::Object *o)
{
    return dynamic_cast<T>(o);
}

};
