/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "MainWindowMDIViewInterface.h"
#include "DockWidgetViewInterface.h"
#include "core/MDILayout.h"

using namespace KDDockWidgets;
using namespace KDDockWidgets::Core;

MainWindowMDIViewInterface::MainWindowMDIViewInterface(MDILayout *layout)
    : m_mdiLayout(layout)
{
}

void MainWindowMDIViewInterface::addDockWidget(DockWidgetViewInterface *dockWidget, Point localPos,
                                               const InitialOption &addingOption)
{
    DockWidget *dw = dockWidget ? dockWidget->dockWidget() : nullptr;
    m_mdiLayout->addDockWidget(dw, localPos, addingOption);
}

#ifdef KDDW_FRONTEND_QT
void MainWindowMDIViewInterface::addDockWidget(DockWidgetViewInterface *dockWidget,
                                               QPointF localPos, const InitialOption &addingOption)
{
    addDockWidget(dockWidget, localPos.toPoint(), addingOption);
}
#endif
