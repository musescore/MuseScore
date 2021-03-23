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

    void run(const ITestCasePtr& tc, const io::path& scorePath);
    async::Channel<AbContext> finished() const;

private:

    void nextStep(const AbContext& ctx);
    void doFinish(const AbContext& ctx);

    int delayToMSec(ITestStep::Delay d) const;

    ITestCasePtr m_testCase;
    int m_stepIndex = -1;
    async::Channel<AbContext> m_finished;
};
}

#endif // MU_AUTOBOT_ABRUNNER_H
