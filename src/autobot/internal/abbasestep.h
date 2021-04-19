/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MU_AUTOBOT_ABBASESTEP_H
#define MU_AUTOBOT_ABBASESTEP_H

#include <memory>

#include "ret.h"
#include "../iteststep.h"

namespace mu::autobot {
class AbBaseStep : public ITestStep
{
public:
    AbBaseStep(Delay delay = Delay::Fast);
    virtual ~AbBaseStep() = default;

    Delay delay() const override;
    void make(const IAbContextPtr& ctx) override;
    async::Channel<IAbContextPtr> finished() const override;

protected:
    virtual void doRun(IAbContextPtr ctx) = 0;
    void doFinish(IAbContextPtr ctx, const Ret& ret);

private:
    Delay m_delay = Delay::Fast;
    async::Channel<IAbContextPtr> m_finished;
};

using AbBaseStepPtr = std::shared_ptr<AbBaseStep>;
}

#endif // MU_AUTOBOT_ABBASESTEP_H
