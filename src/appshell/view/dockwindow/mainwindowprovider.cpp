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

#include "modularity/ioc.h"
#include "log.h"

using namespace mu::dock;
using namespace mu::async;

MainWindowProvider::MainWindowProvider(QObject* parent)
    : QObject(parent), m_window(nullptr)
{
}

QWindow* MainWindowProvider::qWindow() const
{
    return m_window;
}

QWindow* MainWindowProvider::topWindow() const
{
    if (m_windows.isEmpty()) {
        return qWindow();
    }
    return m_windows.top();
}

void MainWindowProvider::pushWindow(QWindow* w)
{
    m_windows.push(w);
}

void MainWindowProvider::popWindow(QWindow* w)
{
    IF_ASSERT_FAILED(m_windows.top() == w) {
        return;
    }
    m_windows.pop();
}

void MainWindowProvider::setWindow(QWindow* window)
{
    if (m_window != nullptr) {
        LOGW() << "Window for this MainWindowProvider is already set. Refusing to set it again.";
        return;
    }

    m_window = window;
    emit windowChanged();

    init();
}

void MainWindowProvider::init()
{
    modularity::ioc()->registerExport<ui::IMainWindow>("dock", this);
}

QString MainWindowProvider::filePath() const
{
    return m_window ? m_window->filePath() : "";
}

void MainWindowProvider::setFilePath(const QString& filePath)
{
    if (!m_window) {
        return;
    }

    if (filePath == m_window->filePath()) {
        return;
    }

    m_window->setFilePath(filePath);
    emit filePathChanged();
}

bool MainWindowProvider::fileModified() const
{
    return false;
}

void MainWindowProvider::setFileModified(bool /*modified*/)
{
}

void MainWindowProvider::requestShowOnBack()
{
    m_window->lower();
}

void MainWindowProvider::requestShowOnFront()
{
    struct Holder {
        QMetaObject::Connection conn;
    };

    Holder* h = new Holder();
    h->conn = QObject::connect(m_window, &QWindow::activeChanged, [this, h]() {
        if (m_window->isActive()) {
            m_window->raise();
        }

        QObject::disconnect(h->conn);
        delete h;
    });
    m_window->show();
    m_window->requestActivate();
}

bool MainWindowProvider::isFullScreen() const
{
    Qt::WindowStates states = m_window ? m_window->windowStates() : Qt::WindowStates();
    return states.testFlag(Qt::WindowFullScreen);
}

void MainWindowProvider::toggleFullScreen()
{
    if (!m_window) {
        return;
    }

    if (isFullScreen()) {
        m_window->showNormal();
    } else {
        m_window->showFullScreen();
    }
}

const QScreen* MainWindowProvider::screen() const
{
    return m_window ? m_window->screen() : nullptr;
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
    m_showToolBarDockingHolderRequested.send(globalPos);
}

mu::async::Channel<QPoint> MainWindowProvider::showToolBarDockingHolderRequested() const
{
    return m_showToolBarDockingHolderRequested;
}

void MainWindowProvider::requestShowPanelDockingHolder(const QPoint& globalPos)
{
    m_showPanelDockingHolderRequested.send(globalPos);
}

mu::async::Channel<QPoint> MainWindowProvider::showPanelDockingHolderRequested() const
{
    return m_showPanelDockingHolderRequested;
}

void MainWindowProvider::requestHideAllDockingHolders()
{
    m_hideAllHoldersRequested.notify();
}

mu::async::Notification MainWindowProvider::hideAllDockingHoldersRequested() const
{
    return m_hideAllHoldersRequested;
}
