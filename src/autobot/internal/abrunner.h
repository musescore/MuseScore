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

namespace mu::autobot {
class AbRunner : public async::Asyncable
{
public:
    AbRunner() = default;

    void init();
    void run(const io::path& scorePath);

    async::Channel<AbContext> finished() const;

private:

    struct Step
    {
        int delayMSec = 10;
        AbBaseStepPtr step;
        Step(AbBaseStep* s)
            : step(std::shared_ptr<AbBaseStep>(s)) {}
        Step(AbBaseStep* s, int d)
            : delayMSec(d), step(std::shared_ptr<AbBaseStep>(s)) {}
    };

    void nextStep(const AbContext& ctx);

    std::vector<Step> m_steps;
    int m_currentIndex = -1;
    async::Channel<AbContext> m_finished;
};
}

#endif // MU_AUTOBOT_ABRUNNER_H
