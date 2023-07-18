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

#ifndef MU_APPSHELL_WINFRAMELESSWINDOWCONTROLLER_H
#define MU_APPSHELL_WINFRAMELESSWINDOWCONTROLLER_H

#include <QObject>

#include "internal/framelesswindowcontroller.h"

#include "modularity/ioc.h"
#include "ui/iuiconfiguration.h"
#include "ui/imainwindow.h"

#include "windows.h"

namespace mu::appshell {
class WinFramelessWindowController : public QObject, public FramelessWindowController
{
    INJECT(ui::IUiConfiguration, uiConfiguration)
    INJECT(ui::IMainWindow, mainWindow)

public:
    explicit WinFramelessWindowController();

    void init() override;

private:
    bool eventFilter(QObject* watched, QEvent* event) override;
    bool nativeEventFilter(const QByteArray& eventType, void* message, long* result) override;

    bool removeWindowFrame(MSG* message, long* result);
    bool calculateWindowSize(MSG* msg, long* result);
    bool processMouseMove(MSG* message, long* result) const;
    bool processMouseRightClick(MSG* message) const;

    void updateContextMenuState(MSG* message) const;
    bool showSystemMenuIfNeed(MSG* message) const;

    bool isWindowMaximized(HWND hWnd) const;
    bool isTaskbarInAutohideState() const;

    std::optional<UINT> taskbarEdge() const;

    int borderWidth() const;

    QScreen* m_screen = nullptr;

    MONITORINFO m_monitorInfo;
};
}

#endif // MU_APPSHELL_WINFRAMELESSWINDOWCONTROLLER_H
