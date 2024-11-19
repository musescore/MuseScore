/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "DockRegistry.h"
#include "ObjectGuard_p.h"

#include <kdbindings/signal.h>


#pragma once

// Pimpl class so we can keep kdbindings private

namespace KDDockWidgets {

class DockRegistry::Private
{
public:
    Core::ObjectGuard<Core::DockWidget> m_focusedDockWidget;

    /// @brief emitted when a main window or a floating window change screen
    KDBindings::Signal<std::shared_ptr<Core::Window>> windowChangedScreen;

    /// @brief emitted when the MDI group that's being resized changed
    KDBindings::Signal<> groupInMDIResizeChanged;

    /// @brief emitted whenever Config::dropIndicatorsInhibited changes
    /// @param inhibited
    KDBindings::Signal<bool> dropIndicatorsInhibitedChanged;

    KDBindings::ConnectionHandle m_connection;

    int m_numLayoutSavers = 0;

    CloseReason m_currentCloseReason = CloseReason::Unspecified;
};

}
