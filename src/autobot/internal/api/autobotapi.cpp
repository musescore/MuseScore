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
#include "autobotapi.h"

#include <QTimer>
#include <QEventLoop>

#include "log.h"

using namespace mu::api;

AutobotApi::AutobotApi(IApiEngine* e)
    : ApiObject(e)
{
}

bool AutobotApi::openProject(const QString& name)
{
    io::path dir = autobotConfiguration()->filesPath();
    io::path filePath = dir + "/" + name;
    Ret ret = projectFilesController()->openProject(filePath);
    return ret;
}

void AutobotApi::sleep(int msec)
{
    QEventLoop loop;
    QTimer timer;
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(msec);
    loop.exec();
}

void AutobotApi::setInterval(int msec)
{
    m_intervalMsec = msec;
}

void AutobotApi::setTestCase(const QString& name)
{
    LOGD() << "test case: " << name;
}

void AutobotApi::step(const QString& name)
{
    LOGD() << "step: " << name;
    sleep(m_intervalMsec);
}
