/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "Action.h"
#include "Action_p.h"

using namespace KDDockWidgets;
using namespace KDDockWidgets::Core;

Action::Action(DockWidget *dw, const char *debugName)
    : d(new Private(dw, debugName))
{
}

Action::~Action()
{
    delete d;
}

bool Action::enabled() const
{
    return isEnabled();
}

void Action::toggle()
{
    setChecked(!isChecked());
}
