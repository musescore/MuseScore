/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "../QtCompat_p.h"

#include <memory>

namespace KDDockWidgets {

namespace Core {

class Window;
class View;

class EventFilterInterface
{
public:
    EventFilterInterface() = default;
    virtual ~EventFilterInterface();

    /// @brief Override to handle expose events for a certain window
    virtual bool onExposeEvent(std::shared_ptr<Window>)
    {
        return false;
    }

    /// @brief Override to handle when a view receives a mouse press event
    virtual bool onMouseButtonPress(View *, MouseEvent *)
    {
        return false;
    }

    /// @brief Override to handle when a view receives a mouse press event
    virtual bool onMouseButtonRelease(View *, MouseEvent *)
    {
        return false;
    }

    /// @brief Override to handle when a view receives a mouse press event
    virtual bool onMouseButtonMove(View *, MouseEvent *)
    {
        return false;
    }

    /// @brief Override to handle when a view receives a mouse double click event
    virtual bool onMouseDoubleClick(View *, MouseEvent *)
    {
        return false;
    }

    /// @brief Provided for convenience, aggregates all other overloads
    /// receives all mouse event types, if you return true here then the specialized counterparts
    /// won't be called Example, if true is returned here for a mouse press, then
    /// onMouseButtonPress() won't be called
    virtual bool onMouseEvent(View *, MouseEvent *)
    {
        return false;
    }

    /// @brief Override to handle drag enter, drag leave, drag move and drop events
    virtual bool onDnDEvent(View *, Event *)
    {
        return false;
    }

    /// @brief Override to handle a move event
    virtual bool onMoveEvent(View *)
    {
        return false;
    }

    /// Returns whether mouse filtering is enabled. Default true
    bool enabled() const;
    void setEnabled(bool);

private:
    bool m_enabled = true;
    EventFilterInterface(const EventFilterInterface &) = delete;
    EventFilterInterface &operator=(const EventFilterInterface &) = delete;
};

}

}
