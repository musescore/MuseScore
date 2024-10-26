/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Waqar Ahmed <waqar.ahmed@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "ViewGuard.h"
#include "View.h"
#include "core/View_p.h"

using namespace KDDockWidgets;
using namespace KDDockWidgets::Core;

ViewGuard::ViewGuard(View *view)
{
    setView(view);
}

ViewGuard::ViewGuard(const ViewGuard &other)
{
    setView(other.view());
}

ViewGuard::~ViewGuard()
{
    clear();
}

ViewGuard::operator bool() const
{
    return !isNull();
}

bool ViewGuard::isNull() const
{
    return v == nullptr;
}

View *ViewGuard::operator->()
{
    return v;
}

const View *ViewGuard::operator->() const
{
    return v;
}

void ViewGuard::clear()
{
    v = nullptr;
    m_onDestroy.disconnect();
}

View *ViewGuard::view() const
{
    return v;
}

ViewGuard &ViewGuard::operator=(View *view)
{
    setView(view);
    return *this;
}

ViewGuard &ViewGuard::operator=(const ViewGuard &other)
{
    if (this == &other)
        return *this;

    setView(other.view());
    return *this;
}

void ViewGuard::setView(View *view)
{
    if (view == v)
        return;

    if (view && view->inDtor()) {
        // We don't care about views that are already being in DTOR. They count as already deleted
        // for what's ViewGuard concerned. This is rare anyway, would need to require some
        // reentrancy.
        view = nullptr;
    }

    clear();
    v = view;

    if (v) {
        m_onDestroy = v->d->beingDestroyed.connect([this] { v = nullptr; });
    }
}
