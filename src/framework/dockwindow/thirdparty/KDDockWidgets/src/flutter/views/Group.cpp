/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

/**
 * @file
 * @brief The GUI counterpart of Frame.
 *
 * @author Sérgio Martins \<sergio.martins@kdab.com\>
 */

#include "Group.h"

#include "kddockwidgets/core/Group.h"
#include "kddockwidgets/core/Stack.h"
#include "kddockwidgets/core/TitleBar.h"
#include "kddockwidgets/core/DockWidget.h"
#include "core/DockWidget_p.h"

#include "flutter/ViewFactory.h"
#include "flutter/Platform.h"
#include "flutter/views/DockWidget.h"

#include "Stack.h"

#include "Config.h"
#include "core/ViewFactory.h"
#include "core/WidgetResizeHandler_p.h"

using namespace KDDockWidgets;
using namespace KDDockWidgets::flutter;

Group::Group(Core::Group *controller, Core::View *parent)
    : View(controller, Core::ViewType::Frame, parent)
    , GroupViewInterface(controller)
{
}

Group::~Group()
{
}

int Group::currentIndex() const
{
    return 0;
}

Size Group::minSize() const
{
    const Size contentsSize = m_group->dockWidgetsMinSize();
    return contentsSize + Size(0, nonContentsHeight());
}

Size Group::maxSizeHint() const
{
    return View::maxSizeHint();
}

int Group::nonContentsHeight() const
{
    return 0;
}

Rect Group::dragRect() const
{
    // Not implemented for flutter
    return {};
}
