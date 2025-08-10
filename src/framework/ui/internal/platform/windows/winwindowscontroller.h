/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#pragma once

#include <windows.h>
#include <minwindef.h>

#include <QObject>

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "ui/iuiconfiguration.h"
#include "ui/imainwindow.h"

#include "internal/windowscontroller.h"

namespace muse::ui {
class WinWindowsController : public QObject, public WindowsController, public async::Asyncable
{
    INJECT(muse::ui::IUiConfiguration, uiConfiguration)
    INJECT(muse::ui::IMainWindow, mainWindow)

public:
    explicit WinWindowsController();

private:
    void finishRegWindow(WId winId) override;

    bool eventFilter(QObject* watched, QEvent* event) override;

    bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override;
    bool nativeEventFilterForMainWindow(UINT messageType, HWND hWnd, LPARAM lParam, qintptr* result);
    bool nativeEventFilterForNonMainWindow(UINT messageType, HWND hWnd);

    bool removeWindowFrame(HWND hWnd, LPARAM lParam, qintptr* result);
    bool calculateWindowSize(HWND hWnd, LPARAM lParam, qintptr* result);
    bool initWindowBackgroundColor(HWND hWnd);

    bool processMouseMove(HWND hWnd, LPARAM lParam, qintptr* result) const;
    bool processMouseRightClick(HWND hWnd, LPARAM lParam) const;

    void updateContextMenuState(HWND hWnd) const;
    bool showSystemMenuIfNeed(HWND hWnd, LPARAM lParam) const;

    bool isWindowMaximized(HWND hWnd) const;

    HWND mainWindowId() const;
    bool isMainWindow(HWND hWnd) const;

    void updateMainWindowPosition();

    HWND findTaskbar() const;
    std::optional<UINT> taskbarEdge() const;
    bool isTaskbarInAutohideState() const;
    bool isTaskbarVisibleOnMonitor(HMONITOR hMonitor) const;

    int borderWidth() const;

    QScreen* m_screen = nullptr;

    MONITORINFO m_monitorInfo;
};
}
