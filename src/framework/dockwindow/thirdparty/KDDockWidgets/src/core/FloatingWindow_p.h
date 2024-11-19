/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "FloatingWindow.h"
#include "Config.h"
#include "ObjectGuard_p.h"

#include <kdbindings/signal.h>

#pragma once


namespace KDDockWidgets {

namespace Core {

class FloatingWindow::Private
{
public:
    explicit Private(FloatingWindowFlags requestedFlags, FloatingWindow *q);

    KDBindings::Signal<> activatedChanged;
    KDBindings::Signal<> numGroupsChanged;
    KDBindings::Signal<> numDockWidgetsChanged;
    KDBindings::Signal<> windowStateChanged;

    KDBindings::ScopedConnection m_visibleWidgetCountConnection;
    KDBindings::ScopedConnection m_currentStateChangedConnection;
    KDBindings::ScopedConnection m_layoutDestroyedConnection;

    const FloatingWindowFlags m_flags;
    ObjectGuard<DropArea> m_dropArea;
    bool m_minimizationPending = false;
};

}

}
