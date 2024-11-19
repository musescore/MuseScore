/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "TitleBar.h"

#include <kdbindings/signal.h>

#pragma once

// Pimpl class so we can keep kdbindings private

namespace KDDockWidgets {

namespace Core {

class TitleBar::Private
{
public:
    KDBindings::Signal<> titleChanged;
    KDBindings::Signal<> iconChanged;
    KDBindings::Signal<> isFocusedChanged;
    KDBindings::Signal<> numDockWidgetsChanged;
    KDBindings::Signal<bool> floatButtonVisibleChanged;
    KDBindings::Signal<QString> floatButtonToolTipChanged;

    /// @brief Emitted to tell the views to update their auto-hide button
    KDBindings::Signal<bool, bool, KDDockWidgets::TitleBarButtonType> autoHideButtonChanged;

    /// @brief Emitted to tell the views to update their minimize button
    KDBindings::Signal<bool, bool> minimizeButtonChanged;

    /// @brief Emitted to tell the views to update their close button
    KDBindings::Signal<bool, bool> closeButtonChanged;

    /// @brief Emitted to tell the views to update their maximize button
    KDBindings::Signal<bool, bool, KDDockWidgets::TitleBarButtonType> maximizeButtonChanged;

    KDBindings::ScopedConnection isFocusedChangedConnection;
    KDBindings::ScopedConnection isInMainWindowChangedConnection;
    KDBindings::ScopedConnection numDockWidgetsChangedConnection;

    /// Buttons which are forcibly hidden by the user's requirements (overriding kddw's default business logic)
    TitleBarButtonTypes m_userHiddenButtonTypes = {};
    TitleBarButtonTypes m_buttonsToHideIfDisabled = {};
};

}

}
