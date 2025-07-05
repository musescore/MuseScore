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
#ifndef MUSE_AUTOBOT_API_AUTOBOTAPI_H
#define MUSE_AUTOBOT_API_AUTOBOTAPI_H

#include <QJSValue>

#include "api/apiobject.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "autobot/iautobot.h"
#include "autobot/iautobotconfiguration.h"
#include "global/iinteractive.h"
#include "io/ifilesystem.h"
#include "ui/imainwindow.h"

namespace muse::autobot::api {
class AutobotApi : public muse::api::ApiObject, public async::Asyncable
{
    Q_OBJECT

    Inject<autobot::IAutobot> autobot = { this };
    Inject<autobot::IAutobotConfiguration> autobotConfiguration = { this };
    Inject<actions::IActionsDispatcher> dispatcher = { this };
    Inject<IInteractive> interactive = { this };
    Inject<io::IFileSystem> fileSystem = { this };
    Inject<muse::ui::IMainWindow> mainWindow = { this };

public:
    explicit AutobotApi(muse::api::IApiEngine* e);

    Q_INVOKABLE void setInterval(int msec);
    Q_INVOKABLE void runTestCase(const QJSValue& testCase);
    Q_INVOKABLE bool pause(bool immediately = false);
    Q_INVOKABLE bool confirm(const QString& msg);
    Q_INVOKABLE void abort();
    Q_INVOKABLE void error(const QString& msg); //! TODO At the moment same as fatal, but should be not fatal error
    Q_INVOKABLE void fatal(const QString& msg);

    Q_INVOKABLE bool openProject(const QString& name);
    Q_INVOKABLE void saveProject(const QString& name = QString());

    // Helpers
    Q_INVOKABLE void sleep(int msec = -1);
    Q_INVOKABLE void waitPopup();
    Q_INVOKABLE void seeChanges(int msec = -1);
    Q_INVOKABLE void async(const QJSValue& func, const QJSValueList& args = QJSValueList());
    Q_INVOKABLE int randomInt(int min, int max) const;
    Q_INVOKABLE int fileSize(const QString& path) const;

    // Interactive
    Q_INVOKABLE QString selectedFilePath() const;

    // Window
    Q_INVOKABLE void showMainWindowOnFront();
};
}

#endif // MUSE_AUTOBOT_API_AUTOBOTAPI_H
