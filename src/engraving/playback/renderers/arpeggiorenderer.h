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

#ifndef MU_ENGRAVING_ARPEGGIORENDERER_H
#define MU_ENGRAVING_ARPEGGIORENDERER_H

#include "renderbase.h"

namespace mu::engraving {
class Chord;

class ArpeggioRenderer : public RenderBase<ArpeggioRenderer>
{
public:
    static const mpe::ArticulationTypeSet& supportedTypes();

    static void doRender(const EngravingItem* item, const mpe::ArticulationType preferredType, const RenderingContext& context,
                         mpe::PlaybackEventList& result);

private:
    static bool isDirectionUp(const mpe::ArticulationType type);
    static mpe::msecs_t timestampOffsetStep(const RenderingContext& context, int stepCount);
    static std::map<mpe::pitch_level_t, NominalNoteCtx> arpeggioNotes(const Chord* chord, const RenderingContext& ctx);
};
}

#endif // MU_ENGRAVING_ARPEGGIORENDERER_H
