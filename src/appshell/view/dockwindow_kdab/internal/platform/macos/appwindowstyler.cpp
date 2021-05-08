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
#include "appwindowstyler.h"

#include <QObject>
#include <QWindow>

using namespace mu::appshell;

AppWindowStyler::AppWindowStyler(QObject* parent)
    : QObject(parent), m_targetWindow(nullptr)
{
}

void AppWindowStyler::init()
{
    uiConfiguration()->currentThemeChanged().onNotify(this, [this]() {
        uiConfiguration()->applyPlatformStyle(m_targetWindow);
    });
}

QWindow* AppWindowStyler::targetWindow() const
{
    return m_targetWindow;
}

void AppWindowStyler::setTargetWindow(QWindow* window)
{
    if (m_targetWindow == window) {
        return;
    }

    m_targetWindow = window;
    emit targetWindowChanged();

    uiConfiguration()->applyPlatformStyle(m_targetWindow);
}
