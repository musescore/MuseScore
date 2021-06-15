/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "mainwindowprovider.h"

#include <QWindow>

#include "thirdparty/KDDockWidgets/src/private/DockRegistry_p.h"

#include "log.h"

using namespace mu::dock;
using namespace mu::async;

inline QWindow* mainWindow()
{
    auto windows = KDDockWidgets::DockRegistry::self()->mainwindows();

    if (windows.isEmpty()) {
        return nullptr;
    }

    auto mainWindow = windows.first();
    return mainWindow ? mainWindow->windowHandle() : nullptr;
}

QMainWindow* MainWindowProvider::qMainWindow() const
{
    return nullptr;
}

QWindow* MainWindowProvider::qWindow() const
{
    return mainWindow();
}

void MainWindowProvider::requestShowOnBack()
{
    QWindow* w = mainWindow();
    w->lower();
}

void MainWindowProvider::requestShowOnFront()
{
    struct Holder {
        QMetaObject::Connection conn;
    };

    QWindow* w = mainWindow();
    Holder* h = new Holder();
    h->conn = QObject::connect(w, &QWindow::activeChanged, [w, h]() {
        if (w->isActive()) {
            w->raise();
        }

        QObject::disconnect(h->conn);
        delete h;
    });
    w->show();
    w->requestActivate();
}

bool MainWindowProvider::isFullScreen() const
{
    const QWindow* window = mainWindow();
    Qt::WindowStates states = window ? window->windowStates() : Qt::WindowStates();
    return states.testFlag(Qt::WindowFullScreen);
}

void MainWindowProvider::toggleFullScreen()
{
    QWindow* window = mainWindow();
    if (!window) {
        return;
    }

    if (isFullScreen()) {
        window->showNormal();
    } else {
        window->showFullScreen();
    }
}

const QScreen* MainWindowProvider::screen() const
{
    const QWindow* window = mainWindow();
    return window ? window->screen() : nullptr;
}

void MainWindowProvider::requestChangeToolBarOrientation(const QString& toolBarName, mu::framework::Orientation orientation)
{
    m_dockOrientationChanged.send(toolBarName, orientation);
}

Channel<QString, mu::framework::Orientation> MainWindowProvider::changeToolBarOrientationRequested() const
{
    return m_dockOrientationChanged;
}

void MainWindowProvider::requestShowToolBarDockingHolder(const QPoint& globalPos)
{
    m_showDockingHolderRequested.send(globalPos);
}

mu::async::Channel<QPoint> MainWindowProvider::showToolBarDockingHolderRequested() const
{
    return m_showDockingHolderRequested;
}

void MainWindowProvider::requestHideAllDockingHolders()
{
    m_hideAllHoldersRequested.notify();
}

mu::async::Notification MainWindowProvider::hideAllDockingHoldersRequested() const
{
    return m_hideAllHoldersRequested;
}
