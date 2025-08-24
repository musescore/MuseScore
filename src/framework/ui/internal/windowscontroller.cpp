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

#include "windowscontroller.h"

#include "global/containers.h"

#include "log.h"

using namespace muse::ui;

void WindowsController::regWindow(WId winId)
{
    IF_ASSERT_FAILED(!muse::contains(m_windowsIds, winId)) {
        return;
    }

    m_windowsIds.push_back(winId);

    finishRegWindow(winId);
}

void WindowsController::unregWindow(WId winId)
{
    IF_ASSERT_FAILED(muse::contains(m_windowsIds, winId)) {
        return;
    }

    muse::remove(m_windowsIds, winId);
}

QRect WindowsController::mainWindowTitleBarMoveArea() const
{
    return m_mainWindowTitleBarMoveArea;
}

void WindowsController::setMainWindowTitleBarMoveArea(const QRect& area)
{
    m_mainWindowTitleBarMoveArea = area;
}

bool WindowsController::nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result)
{
    Q_UNUSED(eventType)
    Q_UNUSED(message)
    Q_UNUSED(result)
    return true;
}

void muse::ui::WindowsController::finishRegWindow(WId winId)
{
    Q_UNUSED(winId)
}
