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

#include "windowsmodel.h"

#include <QWindow>

#ifdef Q_OS_WIN
#include "internal/platform/win/winwindowscontroller.h"
#else
#include "internal/windowscontroller.h"
#endif

using namespace mu::appshell;

WindowsModel::WindowsModel(QObject* parent)
    : QObject(parent)
{
#ifdef Q_OS_WIN
    m_controller = new WinWindowsController();
#else
    m_controller = new WindowsController();
#endif
}

WindowsModel::~WindowsModel()
{
    delete m_controller;
}

void WindowsModel::init()
{
    m_controller->init();
}

QRect WindowsModel::mainWindowTitleBarMoveArea() const
{
    return m_controller->mainWindowTitleBarMoveArea();
}

void WindowsModel::setMainWindowTitleBarMoveArea(const QRect& area)
{
    if (mainWindowTitleBarMoveArea() == area) {
        return;
    }

    m_controller->setMainWindowTitleBarMoveArea(area);
    emit mainWindowTitleBarMoveAreaChanged(area);
}
