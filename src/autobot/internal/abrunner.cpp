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

#include "steps/abscoreloadstep.h"
#include "steps/abscorezoom.h"
#include "steps/abscoreclosestep.h"
#include "steps/abdrawserializationstep.h"
#include "steps/abdrawdeserializationstep.h"
#include "steps/abdrawcompstep.h"

using namespace mu::autobot;

void AbRunner::init()
{
    m_steps = {
        Step(new AbScoreLoadStep()),
        Step(new AbScoreZoom(100), 10),
        Step(new AbDrawSerializationStep(), 0),
        Step(new AbDrawDeserializationStep(), 0),
        Step(new AbDrawCompStep(), 0),
        Step(new AbScoreZoom(50), 1000),
        Step(new AbScoreZoom(100), 1000),
        Step(new AbScoreCloseStep(), 1000)
    };

    for (Step& s : m_steps) {
        s.step->finished().onReceive(this, [this](const AbContext& ctx) {
            nextStep(ctx);
        });
    }
}

void AbRunner::run(const io::path& scorePath)
{
    AbContext ctx;
    ctx.setVal<io::path>(AbContext::Key::ScoreFile, scorePath);

    m_currentIndex = -1;
    nextStep(ctx);
}

void AbRunner::nextStep(const AbContext& ctx)
{
    m_currentIndex += 1;
    if (size_t(m_currentIndex) >= m_steps.size()) {
        m_finished.send(ctx);
        return;
    }

    const Step& step = m_steps.at(m_currentIndex);
    AbBaseStepPtr stepPtr = step.step;
    QTimer::singleShot(step.delayMSec, [this, stepPtr, ctx]() {
        stepPtr->make(ctx);
    });
}

mu::async::Channel<AbContext> AbRunner::finished() const
{
    return m_finished;
}
