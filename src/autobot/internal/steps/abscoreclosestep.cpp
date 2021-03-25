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
#include "abscoreclosestep.h"

using namespace mu::autobot;

AbScoreCloseStep::AbScoreCloseStep(Delay delay)
    : AbBaseStep(delay)
{
}

std::string AbScoreCloseStep::name() const
{
    return "CloseScore";
}

void AbScoreCloseStep::doRun(IAbContextPtr ctx)
{
    auto notation = context()->currentNotation();

    //! NOTE The action "close notation" must be implemented somewhere
    context()->setCurrentNotation(nullptr);
    if (notation) {
        notation->setOpened(false);
    }

    doFinish(ctx, make_ret(Ret::Code::Ok));
}
