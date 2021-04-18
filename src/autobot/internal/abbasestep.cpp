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
#include "abbasestep.h"

using namespace mu::autobot;

AbBaseStep::AbBaseStep(Delay delay)
    : m_delay(delay)
{
}

ITestStep::Delay AbBaseStep::delay() const
{
    return m_delay;
}

void AbBaseStep::make(const IAbContextPtr& ctx)
{
    doRun(ctx);
}

void AbBaseStep::doFinish(IAbContextPtr ctx, const Ret& ret)
{
    ctx->setStepRet(ret);
    m_finished.send(ctx);
}

mu::async::Channel<IAbContextPtr> AbBaseStep::finished() const
{
    return m_finished;
}
