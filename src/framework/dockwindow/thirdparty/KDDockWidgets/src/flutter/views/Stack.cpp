/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "Stack.h"
#include "Controller.h"
#include "kddockwidgets/core/Stack.h"
#include "kddockwidgets/core/TitleBar.h"
#include "flutter/ViewFactory.h"
#include "DockRegistry.h"
#include "Config.h"
#include "Window_p.h"
#include "core/View_p.h"

using namespace KDDockWidgets;
using namespace KDDockWidgets::flutter;

Stack::Stack(Core::Stack *controller, Core::View *parent)
    : View(controller, Core::ViewType::Stack, parent)
    , StackViewInterface(controller)
{
}

void Stack::setDocumentMode(bool)
{
}

bool Stack::isPositionDraggable(Point) const
{
    return true;
}
