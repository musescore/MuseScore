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

#ifndef MU_ENGRAVING_TREMOLORENDERER_H
#define MU_ENGRAVING_TREMOLORENDERER_H

#include "renderbase.h"

namespace mu::engraving {
class TremoloRenderer : public RenderBase<TremoloRenderer>
{
public:
    static const mpe::ArticulationTypeSet& supportedTypes();

    static void doRender(const EngravingItem* item, const mpe::ArticulationType preferredType, const RenderingContext& context,
                         mpe::PlaybackEventList& result);

private:
    static int stepDurationTicks(const Chord* chord, int tremoloLines);
    static void buildAndAppendEvents(const Chord* chord, const mpe::ArticulationType type, const int stepDurationTicks, const int startTick,
                                     const RenderingContext& context, mpe::PlaybackEventList& result);
};
}

#endif // MU_ENGRAVING_TREMOLORENDERER_H
