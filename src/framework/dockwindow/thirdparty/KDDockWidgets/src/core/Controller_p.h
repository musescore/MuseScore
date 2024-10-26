/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

// Pimpl class so we can keep kdbindings private

#include "Controller.h"

#include <kdbindings/signal.h>

namespace KDDockWidgets {

namespace Core {

class Controller::Private
{
public:
    explicit Private(ViewType type, View *view)
        : m_view(view)
        , m_type(type)
    {
    }

    /// signal counterpart for setParentView()
    KDBindings::Signal<Core::View *> parentViewChanged;

    /// signal emitted when ~Controller starts
    KDBindings::Signal<> aboutToBeDeleted;

    View *const m_view;
    const ViewType m_type;
};

}

}
