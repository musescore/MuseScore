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

#ifndef MU_ENGRAVING_PITCTWHEELRENDERER_H
#define MU_ENGRAVING_PITCTWHEELRENDERER_H

#include <limits>
#include <list>
#include <functional>

#include "event.h"

namespace mu::engraving {
class PitchWheelRenderer
{
public:
    struct PitchWheelFunction {
        int32_t mStartTick;
        int32_t mEndTick;
        std::function<int(uint32_t)> func;
    };

    PitchWheelRenderer(PitchWheelSpecs wheelSpec);

    void addPitchWheelFunction(const PitchWheelFunction& function, uint32_t channel);

    EventMap renderPitchWheel() const noexcept;

private:

    struct PitchWheelFunctions
    {
        int32_t startTick = std::numeric_limits<int32_t>::max();
        int32_t endTick = 0;
        std::list<PitchWheelFunction> functions;
    };

    void renderChannelPitchWheel(EventMap& pitchWheelEvents, const PitchWheelFunctions& functions, uint32_t channel) const noexcept;

    int32_t findNextStartTick(const std::list<PitchWheelFunction>& functions) const noexcept;

    int32_t calculatePitchBend(const std::list<PitchWheelFunction>& functions, int32_t tick) const noexcept;

    std::map<uint32_t /*channel*/, PitchWheelFunctions> _functions;

    PitchWheelSpecs _wheelSpec;
};
}  // namespace mu::engraving

#endif //MU_ENGRAVING_PITCTWHEELRENDERER_H
