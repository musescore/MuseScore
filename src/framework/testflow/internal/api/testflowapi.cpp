/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "testflowapi.h"

#include <QTimer>
#include <QEventLoop>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>

#include "async/async.h"
#include "io/fileinfo.h"

#include "log.h"

using namespace muse::actions;
using namespace muse::testflow::api;
using namespace muse::testflow;

TestflowApi::TestflowApi(muse::api::IApiEngine* e)
    : ApiObject(e)
{
    srand(time(nullptr)); // Seed the time for random number generation
}

void TestflowApi::setInterval(int msec)
{
    testflow()->setDefaultIntervalMsec(msec);
}

void TestflowApi::runTestCase(const QJSValue& testCase)
{
    TestCase ts(testCase);
    testflow()->runTestCase(ts);
}

bool TestflowApi::pause(bool immediately)
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

    testflow()->pause();
    return true;
}

bool TestflowApi::confirm(const QString& msg)
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

void TestflowApi::abort()
{
    testflow()->abort();
}

void TestflowApi::error(const QString& msg)
{
    testflow()->fatal(msg);
}

void TestflowApi::fatal(const QString& msg)
{
    testflow()->fatal(msg);
}

bool TestflowApi::openProject(const QString& name)
{
    TRACEFUNC;
    io::paths_t dirs = testflowConfiguration()->testingFilesDirPaths();

    io::path_t filePath;
    bool found = false;
    for (const io::path_t& dir : dirs) {
        filePath = dir + "/" + name;
        if (io::FileInfo::exists(filePath)) {
            found = true;
            break;
        }
    }

    if (!found) {
        LOGE() << "not found file: " << name;
        return false;
    }

    dispatcher()->dispatch("file-open", ActionData::make_arg1<QUrl>(QUrl::fromLocalFile(filePath.toQString())));

    return true;
}

void TestflowApi::saveProject(const QString& name)
{
    TRACEFUNC;
    io::path_t dir = testflowConfiguration()->savingFilesPath();
    if (!QFileInfo::exists(dir.toQString())) {
        QDir().mkpath(dir.toQString());
    }

    io::path_t filePath = dir + "/" + QDateTime::currentDateTime().toString("yyMMddhhmmss") + "_" + name;

    dispatcher()->dispatch("file-save-at", ActionData::make_arg1<io::path_t>(filePath));
}

void TestflowApi::sleep(int msec)
{
    if (msec < 0) {
        msec = testflow()->intervalMsec();
    }

    testflow()->sleep(msec);
}

void TestflowApi::waitPopup()
{
    //! NOTE We could do it smartly, check a current popup actually opened,
    //! but or just sleep some time
    sleep(2000);
}

void TestflowApi::seeChanges(int msec)
{
    if (msec < 0) {
        msec = testflow()->intervalMsec() / 2;
    }
    sleep(msec);
}

void TestflowApi::async(const QJSValue& func, const QJSValueList& args)
{
    async::Async::call(this, [func, args]() {
        QJSValue mut_func = func;
        mut_func.call(args);
    });
}

int TestflowApi::randomInt(int min, int max) const
{
    int val = rand() % (max - min + 1) + min;
    return val;
}

int TestflowApi::fileSize(const QString& pathStr) const
{
    RetVal<uint64_t> size = fileSystem()->fileSize(io::path_t(pathStr));
    if (!size.ret) {
        LOGD() << "filed get file size, err: " << size.ret.toString();
    }
    return size.val;
}

QString TestflowApi::selectedFilePath() const
{
    return testflow()->testflowInteractive()->selectedFilePath().toQString();
}

void TestflowApi::showMainWindowOnFront()
{
    mainWindow()->requestShowOnFront();
}
