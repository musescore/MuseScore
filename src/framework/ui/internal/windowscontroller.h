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

#include <QAbstractNativeEventFilter>
#include <QWindow>

#include "iwindowscontroller.h"

namespace muse::ui {
class WindowsController : public IWindowsController, public QAbstractNativeEventFilter
{
public:
    void regWindow(WId winId) override;
    void unregWindow(WId winId) override;

    QRect mainWindowTitleBarMoveArea() const override;
    void setMainWindowTitleBarMoveArea(const QRect& area) override;

protected:
    bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override;

    virtual void finishRegWindow(WId winId);

    std::vector<WId> m_windowsIds;

private:
    QRect m_mainWindowTitleBarMoveArea;
};
}
