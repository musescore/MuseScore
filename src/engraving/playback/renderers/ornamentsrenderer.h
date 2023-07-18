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

#ifndef MU_ENGRAVING_ORNAMENTSRENDERER_H
#define MU_ENGRAVING_ORNAMENTSRENDERER_H

#include "renderbase.h"

namespace mu::engraving {
struct DisclosurePattern;
class OrnamentsRenderer : public RenderBase<OrnamentsRenderer>
{
public:
    static const mpe::ArticulationTypeSet& supportedTypes();

    static void doRender(const EngravingItem* item, const mpe::ArticulationType preferredType, const RenderingContext& context,
                         mpe::PlaybackEventList& result);

private:
    static void convert(const mpe::ArticulationType type, const DisclosurePattern& pattern, NominalNoteCtx&& noteCtx,
                        mpe::PlaybackEventList& result);

    static int alterationsNumberByTempo(const double beatsPerSeconds, const int principalNoteDurationTicks,
                                        const DisclosurePattern& pattern);

    static void createEvents(const mpe::ArticulationType type, NominalNoteCtx& noteCtx, const int alterationsCount,
                             const int availableDurationTicks, const int overallDurationTicks,
                             const std::vector<mpe::pitch_level_t>& pitchOffsets, mpe::PlaybackEventList& result);
};
}

#endif // MU_ENGRAVING_ORNAMENTSRENDERER_H
