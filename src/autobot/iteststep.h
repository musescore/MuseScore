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
#ifndef MU_AUTOBOT_ITESTSTEP_H
#define MU_AUTOBOT_ITESTSTEP_H

#include <memory>
#include "async/channel.h"
#include "iabcontext.h"

namespace mu::autobot {
class ITestStep
{
public:
    virtual ~ITestStep() = default;

    enum class Delay {
        Fast,
        Normal,
        Long
    };

    virtual std::string name() const = 0;
    virtual Delay delay() const = 0;
    virtual void make(const IAbContextPtr& ctx) = 0;
    virtual async::Channel<IAbContextPtr> finished() const = 0;
};

using ITestStepPtr = std::shared_ptr<ITestStep>;
}

#endif // MU_AUTOBOT_ITESTSTEP_H
