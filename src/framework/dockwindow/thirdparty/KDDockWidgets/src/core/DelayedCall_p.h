/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "KDDockWidgets.h"
#include "ObjectGuard_p.h"

namespace KDDockWidgets::Core {

class DockWidget;
class Controller;

class DelayedCall
{
public:
    DelayedCall() = default;
    virtual ~DelayedCall();
    virtual void call() = 0;

    KDDW_DELETE_COPY_CTOR(DelayedCall)
};

class DelayedDelete : public DelayedCall
{
public:
    explicit DelayedDelete(Controller *);
    ~DelayedDelete() override;

    void call() override;

    KDDW_DELETE_COPY_CTOR(DelayedDelete)
private:
    ObjectGuard<Controller> m_object;
};

class DelayedEmitFocusChanged : public DelayedCall
{
public:
    explicit DelayedEmitFocusChanged(DockWidget *, bool focused);
    ~DelayedEmitFocusChanged() override;

    void call() override;

    KDDW_DELETE_COPY_CTOR(DelayedEmitFocusChanged)
private:
    ObjectGuard<DockWidget> m_dockWidget;
    const bool m_focused;
};

}
