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

#include "modularity/ioc.h"

#include "log.h"

using namespace mu::autobot;

void Autobot::init()
{
    m_autobotInteractive = std::make_shared<AutobotInteractive>();

    m_runner.stepStatusChanged().onReceive(this, [this](const QString& name, StepStatus stepStatus, const Ret& ret) {
        if (stepStatus == StepStatus::Started) {
            m_context->addStep(name);
        }

        m_report.onStepStatusChanged(name, stepStatus, m_context);
        m_stepStatusChanged.send(name, stepStatus, ret);

        if (stepStatus == StepStatus::Aborted) {
            setStatus(Status::Aborted);
        } else if (stepStatus == StepStatus::Error) {
            setStatus(Status::Error);
        }
    });

    m_runner.allFinished().onReceive(this, [this](bool aborted) {
        m_report.endReport(aborted);
    });

    setStatus(Status::Undefined);
    setSpeedMode(SpeedMode::Default);
}

void Autobot::affectOnServices()
{
    //! NOTE Disable reset on mouse press for testing purpose
    navigation()->setIsResetOnMousePress(false);

    //! NOTE Only defaults shortcuts
    shortcutsRegister()->reload(true);

    //! NOTE Change Interactive implementation
    using namespace mu::framework;
    auto realInteractive = modularity::ioc()->resolve<IInteractive>("autobot");
    m_autobotInteractive->setRealInteractive(realInteractive);
    modularity::ioc()->unregisterExport<IInteractive>("autobot");
    modularity::ioc()->registerExport<IInteractive>("autobot", m_autobotInteractive);
}

void Autobot::restoreAffectOnServices()
{
    navigation()->setIsResetOnMousePress(true);
    shortcutsRegister()->reload(false);

    using namespace mu::framework;
    auto realInteractive = m_autobotInteractive->realInteractive();
    modularity::ioc()->unregisterExport<IInteractive>("autobot");
    modularity::ioc()->registerExport<IInteractive>("autobot", realInteractive);
}

void Autobot::execScript(const io::path& path)
{
    LOGD() << path;

    if (status() == Status::Running || status() == Status::Paused) {
        abort();
    }

    //! NOTE If an error occurred during the execution of TestCase and a dialog was opened at that time,
    //! the TestCase loop does not exit because of the open dialog,
    //! so we need to close all dialogs in order to complete the execution of the previous script.
    if (m_engine) {
        std::vector<Uri> stack = interactive()->stack();
        if (stack.size() > 1) {
            const Uri& uri = stack.back();
            interactive()->close(uri);
            QTimer::singleShot(1000, [this, path]() {
                execScript(path);
            });
            return;
        }
    }

    if (m_engine) {
        LOGE() << "unknown internal error with prev execScript";
        delete m_engine;
        m_engine = nullptr;
    }

    IF_ASSERT_FAILED(!m_engine) {
        delete m_engine;
        m_engine = nullptr;
    }

    affectOnServices();

    m_context = std::make_shared<TestCaseContext>();
    m_context->setGlobalVal("script_path", path.toQString());

    m_engine = new ScriptEngine();
    m_engine->setScriptPath(path);

    setStatus(Status::Running);
    Ret ret = m_engine->call("main");

    //! NOTE Also maybe abort or error
    if (status() == Status::Running) {
        setStatus(Status::Finished);
    }

    delete m_engine;
    m_engine = nullptr;
    if (!ret) {
        LOGE() << ret.toString();
    }

    restoreAffectOnServices();
}

SpeedMode Autobot::speedMode() const
{
    return m_runner.speedMode();
}

void Autobot::setSpeedMode(SpeedMode mode)
{
    m_runner.setSpeedMode(mode);
}

mu::async::Channel<SpeedMode> Autobot::speedModeChanged() const
{
    return m_runner.speedModeChanged();
}

void Autobot::setDefaultIntervalMsec(int msec)
{
    m_runner.setDefaultInterval(msec);
}

int Autobot::defaultIntervalMsec() const
{
    return m_runner.defaultInterval();
}

int Autobot::intervalMsec() const
{
    return m_runner.intervalMsec();
}

void Autobot::runTestCase(const TestCase& testCase)
{
    m_report.beginReport(testCase);
    m_runner.run(testCase);
}

void Autobot::abort()
{
    fatal("abort");
}

void Autobot::fatal(const QString& msg)
{
    if (status() == Status::Paused) {
        unpause();
    }

    if (status() != Status::Running) {
        return;
    }

    if (m_engine) {
        m_engine->throwError(msg);
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

AutobotInteractivePtr Autobot::autobotInteractive() const
{
    return m_autobotInteractive;
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

    io::path path;
    if (m_engine) {
        path = m_engine->scriptPath();
    }
    m_statusChanged.send(path, st);
}

IAutobot::Status Autobot::status() const
{
    return m_status;
}

mu::async::Channel<mu::io::path, IAutobot::Status> Autobot::statusChanged() const
{
    return m_statusChanged;
}

mu::async::Channel<QString, StepStatus, mu::Ret> Autobot::stepStatusChanged() const
{
    return m_stepStatusChanged;
}
