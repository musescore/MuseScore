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

void AutobotApi::sleep(int msec)
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

void AutobotApi::setInterval(int msec)
{
    m_intervalMsec = msec;
}

void AutobotApi::runTestCase(QJSValue testCase)
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
        QJSValue wait = step.property("wait");
        bool isWait = wait.isUndefined() ? true : wait.toBool();

        if (!isWait) {
            nextStep();
        }

        func.call();

        if (isWait) {
            nextStep();
        }

        m_testCase.finishedCount += 1;
        if (m_testCase.finishedCount == m_testCase.stepsCount) {
            m_testCase.loop.quit();
        }
    });
}
