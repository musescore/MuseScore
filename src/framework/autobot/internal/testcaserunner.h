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
#ifndef MUSE_AUTOBOT_TESTCASERUNNER_H
#define MUSE_AUTOBOT_TESTCASERUNNER_H

#include <vector>
#include <memory>
#include <QJSValue>
#include <QEventLoop>
#include <QElapsedTimer>

#include "async/channel.h"
#include "async/asyncable.h"
#include "types/ret.h"

#include "../autobottypes.h"

namespace muse::autobot {
class TestCaseRunner : public async::Asyncable
{
public:
    TestCaseRunner() = default;

    SpeedMode speedMode() const;
    void setSpeedMode(SpeedMode mode);
    async::Channel<SpeedMode> speedModeChanged() const;
    int defaultInterval() const;
    void setDefaultInterval(int msec);

    int intervalMsec() const;

    void run(const TestCase& testCase);
    void pause();
    void unpause(bool isNextStep);
    void abort();

    async::Channel<StepInfo, Ret> stepStatusChanged() const;

    async::Channel<bool /*aborted*/> allFinished() const;

private:

    struct TestCaseData
    {
        TestCase testCase;
        Steps steps;
        int stepsCount = 0;
        int currentStepIdx = -1;
        int finishedCount = 0;
        QString lastStepName;
        QEventLoop loop;

        void reset()
        {
            testCase = TestCase();
            steps = Steps();
            stepsCount = 0;
            currentStepIdx = -1;
            finishedCount = 0;
            lastStepName.clear();
            if (loop.isRunning()) {
                loop.quit();
            }
        }
    };

    void nextStep(bool byInterval = true);
    void doAbort();

    int m_intervalMsec = 1000;
    TestCaseData m_testCase;
    bool m_abort = false;
    bool m_paused = false;

    QElapsedTimer m_elapsed;

    async::Channel<StepInfo, Ret> m_stepStatusChanged;
    async::Channel<bool /*aborted*/> m_allFinished;

    SpeedMode m_speedMode = SpeedMode::Undefined;
    async::Channel<SpeedMode> m_speedModeChanged;
};
}

#endif // MUSE_AUTOBOT_TESTCASERUNNER_H
