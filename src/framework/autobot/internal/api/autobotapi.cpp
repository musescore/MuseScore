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
#include <QFileInfo>
#include <QDir>
#include <QDateTime>

#include "async/async.h"
#include "io/fileinfo.h"

#include "log.h"

using namespace muse::autobot::api;
using namespace muse::autobot;

AutobotApi::AutobotApi(muse::api::IApiEngine* e)
    : ApiObject(e)
{
}

void AutobotApi::setInterval(int msec)
{
    autobot()->setDefaultIntervalMsec(msec);
}

void AutobotApi::runTestCase(const QJSValue& testCase)
{
    TestCase ts(testCase);
    autobot()->runTestCase(ts);
}

bool AutobotApi::pause(bool immediately)
{
    if (immediately) {
        IInteractive::Result res = interactive()->questionSync("Pause", "Continue?",
                                                               { IInteractive::Button::Continue, IInteractive::Button::Abort });

        if (res.standardButton() == IInteractive::Button::Abort) {
            abort();
            return false;
        }

        return true;
    }

    autobot()->pause();
    return true;
}

bool AutobotApi::confirm(const QString& msg)
{
    int pauseBtn = int(IInteractive::Button::CustomButton) + 1;
    IInteractive::Result res = interactive()->questionSync("Confirm", msg.toStdString(), {
        interactive()->buttonData(IInteractive::Button::Continue),
        IInteractive::ButtonData(pauseBtn, "Pause"),
        interactive()->buttonData(IInteractive::Button::Abort)
    });

    if (res.standardButton() == IInteractive::Button::Abort) {
        abort();
        return false;
    }

    if (res.button() == pauseBtn) {
        pause();
        return false;
    }

    return true;
}

void AutobotApi::abort()
{
    autobot()->abort();
}

void AutobotApi::error(const QString& msg)
{
    autobot()->fatal(msg);
}

void AutobotApi::fatal(const QString& msg)
{
    autobot()->fatal(msg);
}

bool AutobotApi::openProject(const QString& name)
{
    TRACEFUNC;
    io::paths_t dirs = autobotConfiguration()->testingFilesDirPaths();

    io::path_t filePath;
    for (const io::path_t& dir : dirs) {
        filePath = dir + "/" + name;
        if (io::FileInfo::exists(filePath)) {
            break;
        }
    }

    Ret ret = projectFilesController()->openProject(filePath);
    return ret;
}

void AutobotApi::saveProject(const QString& name)
{
    TRACEFUNC;
    io::path_t dir = autobotConfiguration()->savingFilesPath();
    if (!QFileInfo::exists(dir.toQString())) {
        QDir().mkpath(dir.toQString());
    }

    io::path_t filePath = dir + "/" + QDateTime::currentDateTime().toString("yyMMddhhmmss") + "_" + name;
    projectFilesController()->saveProject(filePath);
}

void AutobotApi::sleep(int msec)
{
    if (msec < 0) {
        msec = autobot()->intervalMsec();
    }

    autobot()->sleep(msec);
}

void AutobotApi::waitPopup()
{
    //! NOTE We could do it smartly, check a current popup actually opened,
    //! but or just sleep some time
    sleep(2000);
}

void AutobotApi::seeChanges(int msec)
{
    if (msec < 0) {
        msec = autobot()->intervalMsec() / 2;
    }
    sleep(msec);
}

void AutobotApi::async(const QJSValue& func, const QJSValueList& args)
{
    async::Async::call(this, [func, args]() {
        QJSValue mut_func = func;
        mut_func.call(args);
    });
}

int AutobotApi::randomInt(int min, int max) const
{
    srand(time(nullptr)); // Seed the time
    int val = rand() % (max - min + 1) + min;
    return val;
}

int AutobotApi::fileSize(const QString& pathStr) const
{
    RetVal<uint64_t> size = fileSystem()->fileSize(io::path_t(pathStr));
    if (!size.ret) {
        LOGD() << "filed get file size, err: " << size.ret.toString();
    }
    return size.val;
}

QString AutobotApi::selectedFilePath() const
{
    return autobot()->autobotInteractive()->selectedFilePath().toQString();
}

void AutobotApi::showMainWindowOnFront()
{
    mainWindow()->requestShowOnFront();
}
