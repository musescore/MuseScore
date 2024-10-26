/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

// The goal of this file is to provide fallback types for non-Qt frontends such as Flutter

#ifdef KDDW_FRONTEND_QT

#include <QCloseEvent>
#include <QMouseEvent>
#include <QHoverEvent>
#include <QDropEvent>
#include <QIcon>
#include <QPixmap>
#include <QPolygon>
#include <QDrag>
#include <QObject>
#include <QString>

#else

#ifdef KDDW_FRONTEND_FLUTTER
#include "flutter/qcoro.h"
#endif

#include "qtcompat/geometry_helpers_p.h"
#include "qtcompat/enums_p.h"
#include "qtcompat/string_p.h"
#include "qtcompat/Object_p.h"
#include "kdtoolbox/KDStlContainerAdaptor.h"
#include <cstdint>

#endif // !Qt

namespace KDDockWidgets {

#ifdef KDDW_FRONTEND_QT

using Polygon = QPolygon;
using Icon = QIcon;
using Pixmap = QPixmap;
using Event = QEvent;
using Drag = QDrag;
using CloseEvent = QCloseEvent;
using FocusEvent = QFocusEvent;
using MouseEvent = QMouseEvent;
using HoverEvent = QHoverEvent;
using DropEvent = QDropEvent;
using DragMoveEvent = QDragMoveEvent;

using Point = QT_PREPEND_NAMESPACE(QPoint);
using Size = QT_PREPEND_NAMESPACE(QSize);
using Rect = QT_PREPEND_NAMESPACE(QRect);
using Margins = QT_PREPEND_NAMESPACE(QMargins);

template<typename T>
using Vector = QVector<T>;

template<typename T>
inline T object_cast(QObject *o)
{
    return qobject_cast<T>(o);
}

template<typename T>
inline T object_cast(const QObject *o)
{
    return qobject_cast<T>(o);
}

// Qt uses QObject, while Flutter uses Object.h
// Qt probably could also use Object.h, but for now reduce the churn for Qt users
namespace Core {
using Object = QT_PREPEND_NAMESPACE(QObject);
}

#define QT_DOCKS_EXPORT DOCKS_EXPORT

#else

class Event
{
public:
    enum Type {
        MouseButtonPress,
        MouseButtonDblClick,
        MouseButtonRelease,
        MouseMove,
        NonClientAreaMouseButtonPress,
        NonClientAreaMouseButtonRelease,
        NonClientAreaMouseMove,
        NonClientAreaMouseButtonDblClick,
        HoverEnter,
        HoverLeave,
        HoverMove,
        DragEnter,
        DragLeave,
        DragMove,
        Drop,
        Move,
        FocusIn,
        WindowActivate,
        LayoutRequest,
        Close
    };

    explicit Event(Type type)
        : m_type(type)
    {
    }

    bool
    isAccepted() const
    {
        return true;
    }

    void accept()
    {
        m_accepted = true;
    }

    void ignore()
    {
        m_accepted = false;
    }

    bool spontaneous() const
    {
        return m_spontaneous;
    }

    Type type() const
    {
        return m_type;
    }

    bool m_accepted = false;
    bool m_spontaneous = false;
    const Type m_type;
};

class CloseEvent : public Event
{
public:
    CloseEvent()
        : Event(Event::Close)
    {
    }
};

class HoverEvent : public Event
{
public:
    using Event::Event;

    Point pos() const
    {
        return {};
    }
};

class MouseEvent : public Event
{
public:
    explicit MouseEvent(Type type, Point localPos, Point /*windowPos*/,
                        Point globalPos, Qt::MouseButtons buttons, Qt::MouseButtons, Qt::KeyboardModifiers)
        : Event(type)
        , m_localPos(localPos)
        , m_globalPos(globalPos)
        , m_buttons(buttons)
    {
    }

    Point pos() const
    {
        return m_localPos;
    }

    Point globalPos() const
    {
        return m_globalPos;
    }

    Qt::MouseButton button() const
    {
        if (m_buttons & Qt::LeftButton)
            return Qt::LeftButton;

        // Nothing else matters
        return Qt::NoButton;
    }

    Qt::MouseButtons buttons() const
    {
        return m_buttons;
    }

    Point m_localPos;
    Point m_globalPos;
    Qt::MouseButtons m_buttons;
};

class DropEvent : public Event
{
public:
    using Event::Event;

    Point position() const
    {
        return {};
    }

    void setDropAction(Qt::DropAction)
    {
    }
};

class DragMoveEvent : public DropEvent
{
public:
    DragMoveEvent(Type type)
        : DropEvent(type)
    {
    }

    DragMoveEvent()
        : DropEvent(Event::DragMove)
    {
    }
};

class FocusEvent : public Event
{
public:
    Qt::FocusReason reason() const
    {
        return {};
    }
};

class Icon
{
public:
    bool isNull() const
    {
        return true;
    }
};

// Only useful on wayland, to set a drag pixmap. Not implemented yet.
class Pixmap
{
public:
};

// Used by segmented indicators controller. Not implemented yet.
class Polygon : public Vector<Point>
{
public:
    Polygon() = default;
    Polygon(Vector<Point>)
    {
    }

    Rect boundingRect() const
    {
        return Rect();
    }

    bool containsPoint(Point, Qt::FillRule) const
    {
        return false;
    }
};

#endif

}

#ifndef KDDW_FRONTEND_FLUTTER
// Only the flutter uses the coroutines
#define KDDW_QCORO_TASK bool
#define KDDW_CO_AWAIT
#define KDDW_CO_RETURN return
#endif

#ifndef KDDW_FRONTEND_QT

// Dummy Qt macros, to avoid too much ifdefs in core/
#define Q_NAMESPACE
#define Q_ENUM(name)
#define Q_ENUM_NS(name)
#define Q_DECLARE_METATYPE(name)
#define Q_INVOKABLE
#define Q_SLOTS
#define QT_DOCKS_EXPORT
#define Q_DECLARE_OPERATORS_FOR_FLAGS(name)
#define QT_BEGIN_NAMESPACE
#define QT_END_NAMESPACE
#define Q_REQUIRED_RESULT
#define QStringLiteral QString
#define QT_VERSION 6
#define QT_VERSION_CHECK(a, b, c) 6
#define Q_UNREACHABLE() std::abort();
#define Q_OBJECT

using quintptr = unsigned long long int;
using qint64 = int64_t;

namespace Qt5Qt6Compat {
inline KDDockWidgets::Point eventGlobalPos(KDDockWidgets::MouseEvent *ev)
{
    return ev->globalPos();
}

}

#endif
