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
#include "autobot.h"

#include <QTimer>

#include "log.h"

using namespace mu::autobot;

void Autobot::init()
{
    m_runner.stepStatusChanged().onReceive(this, [this](const QString& name, StepStatus status) {
        if (status == StepStatus::Started) {
            m_context->addStep(name);
        }

        m_report.onStepStatusChanged(name, status, m_context);
        m_stepStatusChanged.send(name, status);
    });

    m_runner.allFinished().onReceive(this, [this](bool aborted) {
        m_report.endReport(aborted);
    });

    setStatus(Status::Stopped);
}

mu::Ret Autobot::execScript(const io::path& path)
{
    LOGD() << path;

    if (status() != Status::Stopped) {
        abort();
    }

    m_context = std::make_shared<TestCaseContext>();
    m_context->setGlobalVal("script_path", path.toQString());

    m_engine = new ScriptEngine();
    m_engine->setScriptPath(path);

    setStatus(Status::Running);
    Ret ret = m_engine->call("main");
    setStatus(Status::Stopped);

    delete m_engine;
    m_engine = nullptr;
    if (!ret) {
        LOGE() << ret.toString();
    }
    return ret;
}

void Autobot::setStepsInterval(int msec)
{
    m_runner.setStepsInterval(msec);
}

void Autobot::runTestCase(const TestCase& testCase)
{
    m_report.beginReport(testCase);
    m_runner.run(testCase);
}

void Autobot::abort()
{
    if (status() == Status::Paused) {
        unpause();
    }

    setStatus(Status::Stopped);
    if (m_engine) {
        m_engine->throwError("abort");
    }
    m_runner.abort();
}

void Autobot::sleep(int msec)
{
    //! NOTE If pause state, then we sleep until to unpause
    //! It's allowing to do pause during step execution
    if (status() == IAutobot::Status::Paused) {
        m_sleepLoop.exec();
        return;
    }

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, &m_sleepLoop, &QEventLoop::quit);
    timer.start(msec);
    m_sleepLoop.exec();
}

void Autobot::pause()
{
    setStatus(Status::Paused);
    m_runner.pause();
}

void Autobot::unpause()
{
    bool isNextStep = true;

    //! NOTE If pause did on sleep (during step execution),
    //! then unpause current step (without perform next step)
    if (m_sleepLoop.isRunning()) {
        m_sleepLoop.quit();
        isNextStep = false;
    }

    m_runner.unpause(isNextStep);
    setStatus(Status::Running);
}

ITestCaseContextPtr Autobot::context() const
{
    return m_context;
}

void Autobot::setStatus(Status st)
{
    if (m_status == st) {
        return;
    }

    if (m_sleepLoop.isRunning()) {
        m_sleepLoop.quit();
    }

    m_status = st;
    m_statusChanged.send(st);
}

IAutobot::Status Autobot::status() const
{
    return m_status;
}

mu::async::Channel<IAutobot::Status> Autobot::statusChanged() const
{
    return m_statusChanged;
}

mu::async::Channel<QString, StepStatus> Autobot::stepStatusChanged() const
{
    return m_stepStatusChanged;
}
