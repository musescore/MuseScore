/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#ifndef MU_ENGRAVING_CHORDARTICULATIONSRENDERER_H
#define MU_ENGRAVING_CHORDARTICULATIONSRENDERER_H

#include "renderbase.h"

namespace mu::engraving {
class ChordArticulationsRenderer : public RenderBase<ChordArticulationsRenderer>
{
public:
    static const mpe::ArticulationTypeSet& supportedTypes();

    static void doRender(const EngravingItem* item, const mpe::ArticulationType type, const RenderingContext& ctx,
                         mpe::PlaybackEventList& result);

private:
    static bool renderChordArticulations(const Chord* chord, const RenderingContext& ctx, mpe::PlaybackEventList& result);
    static void renderNoteArticulations(const Chord* chord, const RenderingContext& ctx, mpe::PlaybackEventList& result);
    static mpe::duration_t tiedNotesTotalDuration(const Note* firstNote);
};
}

#endif // MU_ENGRAVING_CHORDARTICULATIONSRENDERER_H
