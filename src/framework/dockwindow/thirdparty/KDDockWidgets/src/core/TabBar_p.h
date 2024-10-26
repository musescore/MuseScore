/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

// Pimpl class so we can keep kdbindings private

#include "TabBar.h"
#include "DockWidget.h"
#include "ObjectGuard_p.h"

#include <kdbindings/signal.h>

#include <unordered_map>

namespace KDDockWidgets {

namespace Core {

class Stack;
class Group;

class TabBar::Private
{
public:
    explicit Private(Stack *stack)
        : m_stack(stack)
    {
    }

    void moveTabTo(int from, int to);

    Core::Stack *const m_stack;
    ObjectGuard<DockWidget> m_lastPressedDockWidget = nullptr;
    DockWidget *m_currentDockWidget = nullptr;
    Vector<const DockWidget *> m_dockWidgets;
    bool m_removeGuard = false;
    bool m_isMovingTab = false;
    std::unordered_map<KDDockWidgets::Core::DockWidget *, KDBindings::ScopedConnection> aboutToDeleteConnections;

    KDBindings::Signal<KDDockWidgets::Core::DockWidget *> currentDockWidgetChanged;
    KDBindings::Signal<> countChanged;
};

}

}
