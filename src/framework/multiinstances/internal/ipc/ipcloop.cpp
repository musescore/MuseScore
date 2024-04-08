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
#include "ipcloop.h"

#include "log.h"

using namespace muse::ipc;

IpcLoop::IpcLoop()
{
    m_timer.setSingleShot(true);
    QObject::connect(&m_timer, &QTimer::timeout, [this]() {
        exit(Code::Timeout);
    });
}

Code IpcLoop::exec(int timeout)
{
    if (m_timer.isActive()) {
        LOGE() << "already running";
        return Code::Undefined;
    }

    m_timer.start(timeout);
    return static_cast<Code>(m_loop.exec());
}

void IpcLoop::exit(Code returnCode)
{
    m_loop.exit(static_cast<int>(returnCode));
}
