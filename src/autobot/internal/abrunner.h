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
#ifndef MU_AUTOBOT_ABRUNNER_H
#define MU_AUTOBOT_ABRUNNER_H

#include <vector>
#include <memory>

#include "async/channel.h"
#include "async/asyncable.h"
#include "ret.h"
#include "io/path.h"

#include "abcontext.h"
#include "abbasestep.h"
#include "../itestcase.h"

namespace mu::autobot {
class AbRunner : public async::Asyncable
{
public:
    AbRunner() = default;

    void run(const ITestCasePtr& tc, const IAbContextPtr& ctx);

    async::Channel<IAbContextPtr> stepStarted() const;
    async::Channel<IAbContextPtr> stepFinished() const;

    async::Channel<IAbContextPtr> allFinished() const;

private:

    void nextStep(const IAbContextPtr& ctx);
    void doFinish(const IAbContextPtr& ctx);

    int delayToMSec(ITestStep::Delay d) const;

    ITestCasePtr m_testCase;
    int m_stepIndex = -1;

    async::Channel<IAbContextPtr> m_stepStarted;
    async::Channel<IAbContextPtr> m_stepFinished;
    async::Channel<IAbContextPtr> m_allFinished;
};
}

#endif // MU_AUTOBOT_ABRUNNER_H
