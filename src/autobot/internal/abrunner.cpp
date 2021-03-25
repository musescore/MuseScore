//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "abrunner.h"

#include <QTimer>

#include "log.h"

using namespace mu::autobot;

void AbRunner::run(const ITestCasePtr& tc, const IAbContextPtr& ctx)
{
    IF_ASSERT_FAILED(tc) {
        return;
    }

    m_testCase = tc;
    m_stepIndex = -1;

    auto steps = m_testCase->steps();
    for (ITestStepPtr& step : steps) {
        step->finished().onReceive(this, [this](const IAbContextPtr& ctx) {
            m_stepFinished.send(ctx);
            nextStep(ctx);
        });
    }

    nextStep(ctx);
}

int AbRunner::delayToMSec(ITestStep::Delay d) const
{
    switch (d) {
    case ITestStep::Delay::Fast: return 10;
    case ITestStep::Delay::Normal: return 500;
    case ITestStep::Delay::Long: return 1000;
    }
    return 10;
}

void AbRunner::nextStep(const IAbContextPtr& ctx)
{
    m_stepIndex += 1;
    if (size_t(m_stepIndex) >= m_testCase->steps().size()) {
        doFinish(ctx);
        return;
    }

    const ITestStepPtr& step = m_testCase->steps().at(m_stepIndex);
    QTimer::singleShot(delayToMSec(step->delay()), [this, step, ctx]() {
        ctx->addStep(step->name());
        m_stepStarted.send(ctx);
        step->make(ctx);
    });
}

void AbRunner::doFinish(const IAbContextPtr& ctx)
{
    auto steps = m_testCase->steps();
    for (ITestStepPtr& step : steps) {
        step->finished().resetOnReceive(this);
    }
    m_allFinished.send(ctx);
}

mu::async::Channel<IAbContextPtr> AbRunner::stepStarted() const
{
    return m_stepStarted;
}

mu::async::Channel<IAbContextPtr> AbRunner::stepFinished() const
{
    return m_stepFinished;
}

mu::async::Channel<IAbContextPtr> AbRunner::allFinished() const
{
    return m_allFinished;
}
