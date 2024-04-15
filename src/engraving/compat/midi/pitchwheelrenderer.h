/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include <unordered_map>
#include <functional>

#include "../../types/types.h"

#include "event.h"

namespace mu::engraving {
class PitchWheelRenderer
{
public:
    struct PitchWheelFunction {
        int32_t mStartTick = 0;
        int32_t mEndTick = 0;
        std::function<int(uint32_t)> func;
    };

    PitchWheelRenderer(PitchWheelSpecs wheelSpec);

    void addPitchWheelFunction(const PitchWheelFunction& function, uint32_t channel, staff_idx_t staffIdx, MidiInstrumentEffect effect);

    EventsHolder renderPitchWheel() const noexcept;

    static void generateRanges(const std::list<PitchWheelFunction>& functions, std::map<int, int, std::greater<> >& ranges);

private:

    struct PitchWheelFunctions
    {
        int32_t startTick = std::numeric_limits<int32_t>::max();
        int32_t endTick = 0;
        std::list<PitchWheelFunction> functions;
    };

    void renderChannelPitchWheel(EventsHolder& pitchWheelEvents, const PitchWheelFunctions& functions, uint32_t channel) const noexcept;

    int32_t findNextStartTick(const std::list<PitchWheelFunction>& functions) const noexcept;

    int32_t calculatePitchBend(const std::list<PitchWheelFunction>& functions, int32_t tick) const noexcept;

    std::map<uint32_t /*channel*/, PitchWheelFunctions> _functions;

    PitchWheelSpecs _wheelSpec;

    std::unordered_map<int, MidiInstrumentEffect> _effectByChannel;
    std::unordered_map<int, staff_idx_t> _staffIdxByChannel;
};
}  // namespace mu::engraving

#endif //MU_ENGRAVING_PITCTWHEELRENDERER_H
