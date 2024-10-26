/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

/// ObjectGuard provides a weak reference to an object
/// i.e., it becomes null automatically once the object is destroyed
/// For Qt, this is simply QPointer, for Flutter it's an equivalent.

#if defined(KDDW_FRONTEND_QT)

#include <QPointer>

namespace KDDockWidgets::Core {
template<typename T>
using ObjectGuard = QT_PREPEND_NAMESPACE(QPointer<T>);
}

#else

#include "Logging_p.h"
#include "Controller_p.h"
#include "core/layouting/Item_p.h"

namespace KDDockWidgets::Core {

template<typename T>
class ObjectGuard
{
public:
    ObjectGuard() = default;

    ObjectGuard(T *o)
    {
        setObject(o);
    }

    ObjectGuard(const ObjectGuard &other)
    {
        setObject(other.obj);
    }

    ObjectGuard &
    operator=(T *o)
    {
        setObject(o);
        return *this;
    }

    ~ObjectGuard()
    {
        clear();
    }

    operator bool() const
    {
        return obj != nullptr;
    }

    bool isNull() const
    {
        return obj == nullptr;
    }

    T *operator->() const
    {
        return obj;
    }

    operator T *() const
    {
        return obj;
    }

    T *data() const
    {
        return obj;
    }

    void clear()
    {
        if (obj) {
            conn = KDBindings::ScopedConnection();
            obj = nullptr;
        }
    }

private:
    void setObject(T *o)
    {
        if (obj == o)
            return;

        clear();
        obj = o;

        if (auto object = dynamic_cast<Core::Object *>(o)) {
            conn = object->aboutToBeDeleted.connect([this] {
                obj = nullptr;
            });
        }
    }

    T *obj = nullptr;
    KDBindings::ScopedConnection conn;
};

}

#endif
