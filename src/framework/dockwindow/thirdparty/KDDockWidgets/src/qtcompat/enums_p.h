/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

/// enum drop-in replacements for Flutter, since it doesn't depend on Qt
/// The goal is to not change core/ API too much for QtWidgets users, so they will still use the Qt enums.
/// We'll eventually use KDDW's own enums.

namespace Qt {

enum MouseButton {
    NoButton = 0,
    LeftButton = 1,
    RightButton = 2
};

enum KeyboardModifiers {
    NoModifier = 0
};

enum DropAction {
};

enum FocusReason {
    MouseFocusReason = 0,
    OtherFocusReason = 7,
};

enum FocusPolicy {
    NoFocus = 0,
    TabFocus = 1,
    ClickFocus = 2,
    StrongFocus = TabFocus | ClickFocus | 0x8
};

enum FillRule {
    OddEvenFill = 0
};

enum WindowType {
    Window = 1,
    Dialog = 2 | Window,
    Popup = 8 | Window,
    Tool = Popup | Dialog,
    FramelessWindowHint = 0x00000800,
    WindowStaysOnTopHint = 0x00040000
};
using WindowFlags = int;

enum WidgetAttribute {
    WA_SetCursor = 38,
    WA_TransparentForMouseEvents = 51,
    WA_PendingMoveEvent = 34
};
using WidgetAttributes = int;

enum CursorShape {
    ArrowCursor = 0,
    SizeFDiagCursor = 8,
    SizeBDiagCursor = 7,
    SizeVerCursor = 5,
    SizeHorCursor = 6
};

enum Orientation {
    Horizontal = 1,
    Vertical = 2
};

}

template<typename T>
class QFlags
{
public:
    using Int = int;
    QFlags() = default;

    QFlags(int value)
        : m_value(value)
    {
    }

    QFlags(T value)
        : m_value(int(value))
    {
    }

    friend int operator&(const QFlags &lhs, T rhs)
    {
        return lhs.m_value & int(rhs);
    }

    QFlags &operator|=(T rhs)
    {
        m_value |= int(rhs);
        return *this;
    }

    QFlags operator|(T rhs) const
    {
        QFlags<T> result = *this;
        result |= rhs;
        return result;
    }

    QFlags &operator&=(int rhs)
    {
        m_value &= rhs;
        return *this;
    }

    QFlags &operator&=(T rhs)
    {
        m_value &= int(rhs);
        return *this;
    }

    QFlags &operator=(int rhs)
    {
        m_value = int(rhs);
        return *this;
    }

    operator int() const
    {
        return m_value;
    }

    bool testFlag(T v) const
    {
        return m_value & int(v);
    }

    void setFlag(T v, bool enable = true)
    {
        if (enable) {
            m_value |= int(v);
        } else {
            m_value |= ~int(v);
        }
    }

    Int m_value = 0;
};

#define Q_DECLARE_FLAGS(Flags, Enum) \
    using Flags = QFlags<Enum>;

namespace Qt {
Q_DECLARE_FLAGS(MouseButtons, MouseButton);
}
