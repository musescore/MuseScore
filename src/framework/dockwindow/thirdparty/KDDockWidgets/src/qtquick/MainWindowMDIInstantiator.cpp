/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "MainWindowMDIInstantiator.h"
#include "qtquick/views/MainWindow.h"
#include "qtquick/views/MainWindowMDI.h"
#include "kddockwidgets/core/DockWidget.h"
#include "kddockwidgets/core/MainWindow.h"
#include "kddockwidgets/core/DockRegistry.h"
#include "kddockwidgets/core/MDILayout.h"

#include "DockWidgetInstantiator.h"

#include "Platform.h"

using namespace KDDockWidgets;

MainWindowMDIInstantiator::MainWindowMDIInstantiator()
{
}

QString MainWindowMDIInstantiator::uniqueName() const
{
    return m_uniqueName;
}

void MainWindowMDIInstantiator::setUniqueName(const QString &name)
{
    if (name != m_uniqueName) {
        m_uniqueName = name;
        Q_EMIT uniqueNameChanged();
    }
}

QVector<QString> MainWindowMDIInstantiator::affinities() const
{
    return m_mainWindow ? m_mainWindow->affinities() : QVector<QString>();
}

void MainWindowMDIInstantiator::addDockWidget(QQuickItem *dockWidget, QPoint localPos,
                                              const InitialOption &addingOption)
{
    if (!dockWidget || !m_mainWindow)
        return;

    Core::DockWidget *dw = QtQuick::Platform::dockWidgetForItem(dockWidget);

    m_mainWindow->mdiLayout()->addDockWidget(dw, localPos, addingOption);
}

bool MainWindowMDIInstantiator::closeDockWidgets(bool force)
{
    return m_mainWindow && m_mainWindow->closeDockWidgets(force);
}

void MainWindowMDIInstantiator::classBegin()
{
    // Nothing interesting to do here.
}

void MainWindowMDIInstantiator::componentComplete()
{
    if (m_uniqueName.isEmpty()) {
        qWarning() << Q_FUNC_INFO
                   << "Each DockWidget need an unique name. Set the uniqueName property.";
        return;
    }

    if (DockRegistry::self()->containsMainWindow(m_uniqueName)) {
        // MainWindow already exists
        return;
    }

    if (m_uniqueName.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Name can't be empty";
        return;
    }

    if (m_mainWindow) {
        qWarning() << Q_FUNC_INFO << "Main window is already initialized";
        return;
    }

    Core::View *view = new QtQuick::MainWindowMDI(m_uniqueName, this);
    m_mainWindow = view->asMainWindowController();
}
