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

using namespace mu::dock;

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
