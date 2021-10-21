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
#include <QFileInfo>
#include <QDir>
#include <QDateTime>

#include "async/async.h"

#include "log.h"

using namespace mu::api;

AutobotApi::AutobotApi(IApiEngine* e)
    : ApiObject(e)
{
}

bool AutobotApi::openProject(const QString& name)
{
    io::path dir = autobotConfiguration()->testingFilesDirPath();
    io::path filePath = dir + "/" + name;
    Ret ret = projectFilesController()->openProject(filePath);
    return ret;
}

void AutobotApi::saveProject(const QString& name)
{
    io::path dir = autobotConfiguration()->savingFilesPath();
    if (!QFileInfo::exists(dir.toQString())) {
        QDir().mkpath(dir.toQString());
    }

    io::path filePath = dir + "/" + QDateTime::currentDateTime().toString(Qt::ISODate) + "_" + name;
    projectFilesController()->saveProject(filePath);
}

void AutobotApi::setInterval(int msec)
{
    autobot()->setInterval(msec);
    //m_intervalMsec = msec;
}

void AutobotApi::runTestCase(const QJSValue& testCase)
{
    m_abort = false;
    m_testCase.testCase = testCase;
    m_testCase.steps = testCase.property("steps");
    m_testCase.stepsCount = m_testCase.steps.property("length").toInt();
    m_testCase.currentStepIdx = -1;

    nextStep();

    if (m_testCase.currentStepIdx < m_testCase.stepsCount) {
        m_testCase.loop.exec();
    }
}

void AutobotApi::nextStep()
{
    if (m_abort) {
        m_testCase.loop.quit();
        return;
    }

    m_testCase.currentStepIdx += 1;

    if (m_testCase.currentStepIdx >= m_testCase.stepsCount) {
        return;
    }

    QTimer::singleShot(m_intervalMsec, [this]() {
        QJSValue step = m_testCase.steps.property(m_testCase.currentStepIdx);
        QJSValue func = step.property("func");

        func.call();

        nextStep();

        m_testCase.finishedCount += 1;
        if (m_testCase.finishedCount == m_testCase.stepsCount) {
            m_testCase.loop.quit();
        }
    });
}

void AutobotApi::abort()
{
    m_abort = true;
}

bool AutobotApi::pause()
{
    using namespace mu::framework;
    IInteractive::Result res = interactive()->question("Pause", "Continue?",
                                                       { IInteractive::Button::Continue, IInteractive::Button::Abort });

    if (res.standardButton() == IInteractive::Button::Abort) {
        abort();
        return false;
    }

    return true;
}

void AutobotApi::sleep(int msec) const
{
    if (msec < 0) {
        msec = m_intervalMsec;
    }

    QEventLoop loop;
    QTimer timer;
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(msec);
    loop.exec();
}

void AutobotApi::waitPopup() const
{
    //! NOTE We could do it smartly, check a current popup actually opened, but or just sleep some time
    sleep(500);
}

void AutobotApi::seeChanges(int msec)
{
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
