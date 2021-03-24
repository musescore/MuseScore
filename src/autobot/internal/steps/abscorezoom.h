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
#ifndef MU_AUTOBOT_ABSCOREZOOM_H
#define MU_AUTOBOT_ABSCOREZOOM_H

#include "../abbasestep.h"

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"

namespace mu::autobot {
class AbScoreZoom : public AbBaseStep
{
    INJECT(autobot, actions::IActionsDispatcher, dispatcher)
public:
    AbScoreZoom(int percent, ITestStep::Delay delay = ITestStep::Delay::Fast);

protected:
    void doRun(AbContext ctx) override;

private:

    int m_percent = 100;
};
}

#endif // MU_AUTOBOT_ABSCOREZOOM_H
