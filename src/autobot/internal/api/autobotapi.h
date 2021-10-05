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
#ifndef MU_API_AUTOBOTAPI_H
#define MU_API_AUTOBOTAPI_H

#include "apiobject.h"

#include "modularity/ioc.h"
#include "project/iprojectfilescontroller.h"
#include "autobot/iautobotconfiguration.h"

namespace mu::api {
class AutobotApi : public ApiObject
{
    Q_OBJECT

    INJECT(api, autobot::IAutobotConfiguration, autobotConfiguration)
    INJECT(api, project::IProjectFilesController, projectFilesController)

public:
    explicit AutobotApi(IApiEngine* e);

    Q_INVOKABLE bool openProject(const QString& name);
    Q_INVOKABLE void sleep(int msec);

    Q_INVOKABLE void setInterval(int msec);
    Q_INVOKABLE void setTestCase(const QString& name);
    Q_INVOKABLE void step(const QString& name);

private:
    int m_intervalMsec = 1000;
};
}

#endif // MU_API_AUTOBOTAPI_H
