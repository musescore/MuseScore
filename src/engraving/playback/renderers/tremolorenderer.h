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

#ifndef MU_ENGRAVING_TREMOLORENDERER_H
#define MU_ENGRAVING_TREMOLORENDERER_H

#include "renderbase.h"

namespace mu::engraving {
class TremoloRenderer : public RenderBase<TremoloRenderer>
{
public:
    static const muse::mpe::ArticulationTypeSet& supportedTypes();

    static void doRender(const EngravingItem* item, const muse::mpe::ArticulationType preferredType, const RenderingContext& ctx,
                         muse::mpe::PlaybackEventList& result);

private:
    using TremoloTimeCache = std::unordered_map<const Note*, TimestampAndDuration>;

    static int stepDurationTicks(const Chord* chord, int tremoloLines);
    static void buildAndAppendEvents(const Chord* chord, const muse::mpe::ArticulationType type, const int stepDurationTicks,
                                     const int startTick, const RenderingContext& ctx, TremoloTimeCache& tremoloCache,
                                     muse::mpe::PlaybackEventList& result);
    static const TimestampAndDuration& tremoloTimeAndDuration(const Note* note, const RenderingContext& ctx, TremoloTimeCache& cache);
};
}

#endif // MU_ENGRAVING_TREMOLORENDERER_H
