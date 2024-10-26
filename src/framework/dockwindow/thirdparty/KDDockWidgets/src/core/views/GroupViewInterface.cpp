/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "GroupViewInterface.h"
#include "core/Group.h"
#include "core/TabBar.h"
#include "core/Stack.h"

namespace KDDockWidgets::Core {

GroupViewInterface::GroupViewInterface(Group *controller)
    : m_group(controller)
{
}

GroupViewInterface::~GroupViewInterface() = default;

bool GroupViewInterface::isMDI() const
{
    return m_group->isMDI();
}

Group *GroupViewInterface::group() const
{
    return m_group;
}

Rect GroupViewInterface::dragRect() const
{
    return {};
}

void GroupViewInterface::removeDockWidget(DockWidget *dw)
{
    m_group->tabBar()->removeDockWidget(dw);
}

void GroupViewInterface::insertDockWidget(DockWidget *dw, int index)
{
    m_group->stack()->insertDockWidget(dw, index);
}

} // namespace
