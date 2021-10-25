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
    });

    m_runner.allFinished().onReceive(this, [this](bool aborted) {
        m_report.endReport(aborted);
    });
}

mu::Ret Autobot::loadScript(const Script& script)
{
    LOGD() << script.path;

    m_context = std::make_shared<TestCaseContext>();
    m_context->setGlobalVal("script_path", script.path.toQString());

    m_engine = new ScriptEngine();
    m_engine->setScriptPath(script.path);
    Ret ret = m_engine->call("main");
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
    m_runner.runTestCase(testCase);
}

void Autobot::abort()
{
    if (m_engine) {
        m_engine->throwError("abort");
    }
    m_runner.abortTestCase();
}

bool Autobot::pause()
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

ITestCaseContextPtr Autobot::context() const
{
    return m_context;
}
