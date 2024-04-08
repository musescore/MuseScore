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
#include "testcaserunner.h"

#include <QTimer>

#include "log.h"

#include "scriptengine.h"

using namespace muse;
using namespace muse::autobot;

SpeedMode TestCaseRunner::speedMode() const
{
    return m_speedMode;
}

void TestCaseRunner::setSpeedMode(SpeedMode mode)
{
    if (m_speedMode == mode) {
        return;
    }

    m_speedMode = mode;
    m_speedModeChanged.send(mode);
}

async::Channel<SpeedMode> TestCaseRunner::speedModeChanged() const
{
    return m_speedModeChanged;
}

void TestCaseRunner::setDefaultInterval(int msec)
{
    m_intervalMsec = msec;
}

int TestCaseRunner::defaultInterval() const
{
    return m_intervalMsec;
}

void TestCaseRunner::run(const TestCase& testCase)
{
    m_abort = false;
    m_testCase.reset();
    m_testCase.testCase = testCase;
    m_testCase.steps = testCase.steps();
    m_testCase.stepsCount = m_testCase.steps.count();

    nextStep();

    if (m_testCase.currentStepIdx < m_testCase.stepsCount) {
        m_testCase.loop.exec();
    }
}

void TestCaseRunner::pause()
{
    m_paused = true;
    m_stepStatusChanged.send(StepInfo(m_testCase.lastStepName, StepStatus::Paused), muse::make_ok());
}

void TestCaseRunner::unpause(bool isNextStep)
{
    m_paused = false;
    m_stepStatusChanged.send(StepInfo(m_testCase.lastStepName, StepStatus::Started), muse::make_ok());
    if (isNextStep) {
        nextStep(false);
    }
}

void TestCaseRunner::abort()
{
    m_abort = true;
}

async::Channel<StepInfo, Ret> TestCaseRunner::stepStatusChanged() const
{
    return m_stepStatusChanged;
}

async::Channel<bool> TestCaseRunner::allFinished() const
{
    return m_allFinished;
}

void TestCaseRunner::doAbort()
{
    m_allFinished.send(true);
    m_testCase.loop.quit();
}

int TestCaseRunner::intervalMsec() const
{
    switch (m_speedMode) {
    case SpeedMode::Undefined: return m_intervalMsec;
    case SpeedMode::Default: return m_intervalMsec;
    case SpeedMode::Fast: return 250;
    case SpeedMode::Normal: return 1000;
    case SpeedMode::Slow: return 2000;
    }
    return m_intervalMsec;
}

void TestCaseRunner::nextStep(bool byInterval)
{
    if (m_abort) {
        doAbort();
        return;
    }

    if (m_paused) {
        return;
    }

    m_testCase.currentStepIdx += 1;

    if (m_testCase.currentStepIdx >= m_testCase.stepsCount) {
        return;
    }

    QTimer::singleShot(byInterval ? intervalMsec() : 0, [this]() {
        Step step = m_testCase.steps.step(m_testCase.currentStepIdx);
        QString name = step.name();
        m_testCase.lastStepName = name;

        if (step.skip()) {
            LOGD() << "step: " << name << " Skipped";
            m_stepStatusChanged.send(StepInfo(name, StepStatus::Skipped), muse::make_ok());
        } else {
            LOGD() << "step: " << name << " Started";
            m_stepStatusChanged.send(StepInfo(name, StepStatus::Started), muse::make_ok());
            m_elapsed.restart();

            Ret ret = step.exec();
            if (!ret) {
                LOGE() << "failed exec step: " << name << ", err: " << ret.toString();
                StepStatus status = static_cast<Ret::Code>(ret.code()) == Ret::Code::Cancel ? StepStatus::Aborted : StepStatus::Error;
                m_stepStatusChanged.send(StepInfo(step.name(), status), ret);
                doAbort();
                return;
            }

            LOGD() << "step: " << name << " Finished";
            m_stepStatusChanged.send(StepInfo(step.name(), StepStatus::Finished, m_elapsed.elapsed()), muse::make_ok());
        }

        bool withInterval = step.skip() ? false : true;
        nextStep(withInterval);

        m_testCase.finishedCount += 1;
        if (m_testCase.finishedCount == m_testCase.stepsCount) {
            m_allFinished.send(false);
            m_testCase.loop.quit();
        }
    });
}
