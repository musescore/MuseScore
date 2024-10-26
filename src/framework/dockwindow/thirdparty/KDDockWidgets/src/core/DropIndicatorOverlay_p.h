/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "DropIndicatorOverlay.h"
#include "core/Group.h"

#include <kdbindings/signal.h>

#pragma once

#pragma once

// Pimpl class so we can keep kdbindings private

namespace KDDockWidgets {

namespace Core {

class DropIndicatorOverlay::Private
{
public:
    KDBindings::Signal<KDDockWidgets::Core::Group *> hoveredGroupChanged;
    KDBindings::Signal<> hoveredGroupRectChanged;
    KDBindings::Signal<> currentDropLocationChanged;
    KDBindings::ScopedConnection groupConnection;
    KDBindings::ScopedConnection dropIndicatorsInhibitedConnection;
};

}

}
