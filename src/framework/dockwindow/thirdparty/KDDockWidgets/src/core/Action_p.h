/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "Action.h"

#include <kdbindings/signal.h>

#pragma once

// Pimpl class so we can keep kdbindings private

namespace KDDockWidgets {

namespace Core {
class DockWidget;

class Action::Private
{
public:
    explicit Private(Core::DockWidget *dw, const char *dbgName)
        : dockWidget(dw)
        , debugName(dbgName)
    {
    }

    KDBindings::Signal<bool> toggled;
    Core::DockWidget *const dockWidget;
    const char *const debugName;
};

}

}
