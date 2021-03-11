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
#include "abscorezoom.h"
#include "../abinvoker.h"

using namespace mu::autobot;
using namespace mu::actions;

AbScoreZoom::AbScoreZoom(int percent)
    : m_percent(percent)
{
}

void AbScoreZoom::doRun(AbContext ctx)
{
    dispatcher()->dispatch("zoom-x-percent", ActionData::make_arg1<int>(m_percent));
    ctx.setVal<int>(AbContext::Key::ViewZoom, m_percent);
    ctx.ret = make_ret(Ret::Code::Ok);
    doFinish(ctx);
}
