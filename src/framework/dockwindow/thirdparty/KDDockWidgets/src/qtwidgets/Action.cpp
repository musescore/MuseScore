/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/


#include "Action_p.h"
#include "core/Action_p.h"
#include "core/Logging_p.h"

using namespace KDDockWidgets::QtWidgets;

Action::Action(Core::DockWidget *dw, const char *debugName)
    : Core::Action(dw, debugName)
{
    setCheckable(true);
    connect(this, &QAction::toggled, this, [this](bool checked) {
        if (m_lastCheckedState != checked) {
            m_lastCheckedState = checked;
            if (!signalsBlocked()) {
                KDDW_TRACE("Action::toggled({}) ; dw={} ; {}", checked, ( void * )d->dockWidget, d->debugName);
                d->toggled.emit(checked);
            }
        }
    });
}

Action::~Action() = default;

void Action::setIcon(const KDDockWidgets::Icon &icon)
{
    QAction::setIcon(icon);
}

KDDockWidgets::Icon Action::icon() const
{
    return QAction::icon();
}

void Action::setText(const QString &text)
{
    QAction::setText(text);
}

void Action::setToolTip(const QString &text)
{
    QAction::setToolTip(text);
}

QString Action::toolTip() const
{
    return QAction::toolTip();
}

void Action::setEnabled(bool enabled)
{
    QAction::setEnabled(enabled);
}

bool Action::isEnabled() const
{
    return QAction::isEnabled();
}

bool Action::isChecked() const
{
    return QAction::isChecked();
}

void Action::setChecked(bool checked)
{
    QAction::setChecked(checked);
}

bool Action::blockSignals(bool b)
{
    return QAction::blockSignals(b);
}

#ifdef DOCKS_DEVELOPER_MODE
void Action::trigger()
{
    QAction::trigger();
}
#endif
