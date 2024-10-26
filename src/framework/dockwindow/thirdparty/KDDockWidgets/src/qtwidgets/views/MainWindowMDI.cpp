/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "MainWindowMDI.h"
#include "kddockwidgets/core/MDILayout.h"
#include "kddockwidgets/core/MainWindow.h"

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtWidgets;
using namespace KDDockWidgets::Core;

MainWindowMDI::MainWindowMDI(const QString &uniqueName, QWidget *parent,
                             Qt::WindowFlags flags)
    : QtWidgets::MainWindow(uniqueName, MainWindowOption_MDI, parent, flags)
    , Core::MainWindowMDIViewInterface(mainWindow()->mdiLayout())
{
}

MainWindowMDI::~MainWindowMDI()
{
}
