/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "core/Platform.h"
#include "kdbindings/signal.h"

#include <memory>
#include <vector>

namespace KDDockWidgets::Core {

class EventFilterInterface;

class Platform::Private
{
public:
    Private();

    /// @brief This signal is emitted when the currently focused view changes
    KDBindings::Signal<std::shared_ptr<View>> focusedViewChanged;

    /// @brief Emitted whenever a window gets activated (gets keyboard focus)
    /// Not really used within the framework. Implement only if you want your application to react
    /// to window activations and use a different style depending on activation state.
    KDBindings::Signal<std::shared_ptr<View>> windowActivated;
    KDBindings::Signal<std::shared_ptr<View>> windowDeactivated;

    bool inDestruction() const
    {
        return m_inDestruction;
    }

    bool m_inDestruction = false;

    std::vector<EventFilterInterface *> m_globalEventFilters;
};

}
