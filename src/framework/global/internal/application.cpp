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
#include "application.h"

#include <QApplication>

using namespace mu::framework;

void Application::setRunMode(const RunMode& mode)
{
    m_runMode = mode;
}

IApplication::RunMode Application::runMode() const
{
    return m_runMode;
}

bool Application::noGui() const
{
    switch (m_runMode) {
    case RunMode::Editor: return false;
    case RunMode::Converter: return true;
    }
    return false;
}

bool Application::notify(QObject* object, QEvent* event)
{
    return qApp->notify(object, event);
}
