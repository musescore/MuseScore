/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
#ifndef MU_AUTOBOT_ABSCORECLOSESTEP_H
#define MU_AUTOBOT_ABSCORECLOSESTEP_H

#include "../abbasestep.h"
#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

namespace mu::autobot {
class AbScoreCloseStep : public AbBaseStep
{
    INJECT(autobot, context::IGlobalContext, context)
public:
    AbScoreCloseStep(Delay delay = Delay::Fast);

    std::string name() const override;

protected:
    void doRun(IAbContextPtr ctx) override;
};
}

#endif // MU_AUTOBOT_ABSCORECLOSESTEP_H
