/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "Stack.h"
#include "Config.h"

#include "kddockwidgets/core/Stack.h"
#include "kddockwidgets/core/Group.h"
#include "kddockwidgets/core/TabBar.h"

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtQuick;

Stack::Stack(Core::Stack *controller, QQuickItem *parent)
    : QtQuick::View(controller, Core::ViewType::Stack, parent)
    , Core::StackViewInterface(controller)
{
}
