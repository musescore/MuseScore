/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "kddockwidgets/docks_export.h"
#include "kddockwidgets/QtCompat_p.h"

namespace KDDockWidgets {

namespace Core {

class Stack;

/// @brief The interface that Stack views share
class DOCKS_EXPORT StackViewInterface
{
public:
    explicit StackViewInterface(Stack *);
    virtual ~StackViewInterface();

    /// @brief Reimplement if you want to support dragging by QTabWidget instead of TitleBar
    /// This is only implemented by QtWidgets.
    /// Instead of reimplementing this, consider reimplementing
    /// TabBarViewInterface::isPositionDraggable() instead. This exists only because the background
    /// of QTabBar is the QTabWidget, which probably isn't true for other frontends.
    virtual bool isPositionDraggable(Point p) const;

    /// @brief Sets QTabWidget::documentMode(). Only implemented for QtWidgets.
    /// Probably not interesting for other frontends to implement, therefore it's not pure-virtual.
    virtual void setDocumentMode(bool);

protected:
    Stack *const m_stack;
    StackViewInterface(const StackViewInterface &) = delete;
    StackViewInterface &operator=(const StackViewInterface &) = delete;
};

}

}
