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

using namespace mu;
using namespace mu::autobot;

void TestCaseRunner::setStepsInterval(int msec)
{
    m_intervalMsec = msec;
}

void TestCaseRunner::runTestCase(const TestCase& testCase)
{
    m_abort = false;
    m_testCase.testCase = testCase;
    m_testCase.steps = testCase.steps();
    m_testCase.stepsCount = m_testCase.steps.count();
    m_testCase.currentStepIdx = -1;

    nextStep();

    if (m_testCase.currentStepIdx < m_testCase.stepsCount) {
        m_testCase.loop.exec();
    }
}

void TestCaseRunner::abortTestCase()
{
    m_abort = true;
}

async::Channel<QString /*name*/, StepStatus> TestCaseRunner::stepStatusChanged() const
{
    return m_stepStatusChanged;
}

async::Channel<bool> TestCaseRunner::allFinished() const
{
    return m_allFinished;
}

void TestCaseRunner::nextStep()
{
    if (m_abort) {
        m_allFinished.send(true);
        m_testCase.loop.quit();
        return;
    }

    m_testCase.currentStepIdx += 1;

    if (m_testCase.currentStepIdx >= m_testCase.stepsCount) {
        return;
    }

    QTimer::singleShot(m_intervalMsec, [this]() {
        Step step = m_testCase.steps.step(m_testCase.currentStepIdx);
        QString name = step.name();
        LOGD() << "step: " << name;

        if (step.skip()) {
            m_stepStatusChanged.send(name, StepStatus::Skipped);
        } else {
            m_stepStatusChanged.send(name, StepStatus::Started);

            Ret ret = step.exec();
            if (!ret) {
                LOGE() << "failed exec step: " << name << ", err: " << ret.toString();
            }

            m_stepStatusChanged.send(step.name(), StepStatus::Finished);
        }

        nextStep();

        m_testCase.finishedCount += 1;
        if (m_testCase.finishedCount == m_testCase.stepsCount) {
            m_allFinished.send(false);
            m_testCase.loop.quit();
        }
    });
}
