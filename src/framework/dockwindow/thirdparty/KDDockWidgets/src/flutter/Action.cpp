/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "Action.h"
#include "core/Action_p.h"
#include "core/Logging_p.h"
#include "core/DockWidget.h"

using namespace KDDockWidgets::Flutter;

Action::Action(Core::DockWidget *dw, const char *debugName)
    : KDDockWidgets::Core::Action(dw, debugName)
{
}

Action::~Action() = default;

void Action::setIcon(const KDDockWidgets::Icon &)
{
    KDDW_ERROR("Not implemented for Flutter");
}

KDDockWidgets::Icon Action::icon() const
{
    KDDW_ERROR("Not implemented for Flutter");
    return {};
}

bool Action::blockSignals(bool b)
{
    const bool old = m_signalsBlocked;
    m_signalsBlocked = b;
    return old;
}

void Action::setChecked(bool checked)
{
    if (m_checked == checked)
        return;

    m_checked = checked;

    if (!m_signalsBlocked) {
        KDDW_TRACE("Emitting Action::toggled({})", checked);
        d->toggled.emit(checked);
    }
}
