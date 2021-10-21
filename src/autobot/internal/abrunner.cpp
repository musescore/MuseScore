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
#include "abrunner.h"

#include <QTimer>

#include "log.h"

using namespace mu::autobot;

void AbRunner::setStepsInterval(int msec)
{
    m_intervalMsec = msec;
}

void AbRunner::runTestCase(const QJSValue& testCase)
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

void AbRunner::abortTestCase()
{
    m_abort = true;
}

void AbRunner::nextStep()
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
