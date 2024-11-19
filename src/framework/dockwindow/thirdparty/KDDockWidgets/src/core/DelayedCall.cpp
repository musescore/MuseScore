/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "DelayedCall_p.h"
#include "DockWidget_p.h"
#include "Controller.h"
#include "DragController_p.h"
#include "core/Utils_p.h"

using namespace KDDockWidgets::Core;

DelayedCall::~DelayedCall() = default;


DelayedDelete::DelayedDelete(Controller *c)
    : m_object(c)
{
}

DelayedDelete::~DelayedDelete() = default;


void DelayedDelete::call()
{
    if (isWayland() && DragController::instance()->isInQDrag()) {
        // Workaround QTBUG-115527. FloatingWindow must be deleted after QDrag::exec() ends.
        Platform::instance()->runDelayed(200, new DelayedDelete(m_object));
        return;
    }

    // Can't use deleteLater() here due to QTBUG-83030 (deleteLater() never delivered if
    // triggered by a sendEvent() before event loop starts)
    delete m_object;
}


DelayedEmitFocusChanged::DelayedEmitFocusChanged(DockWidget *dw, bool focused)
    : m_dockWidget(dw)
    , m_focused(focused)
{
}

DelayedEmitFocusChanged::~DelayedEmitFocusChanged() = default;

void DelayedEmitFocusChanged::call()
{
    if (m_dockWidget) {
        m_dockWidget->d->isFocusedChanged.emit(m_focused);
    }
}
