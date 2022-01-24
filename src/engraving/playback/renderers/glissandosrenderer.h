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

#ifndef MU_ENGRAVING_GLISSANDOSRENDERER_H
#define MU_ENGRAVING_GLISSANDOSRENDERER_H

#include "renderbase.h"

namespace Ms {
class Note;
}

namespace mu::engraving {
class GlissandosRenderer : public RenderBase<GlissandosRenderer>
{
public:
    static const mpe::ArticulationTypeSet& supportedTypes();

    static void doRender(const Ms::EngravingItem* item, const mpe::ArticulationType type, const RenderingContext& context,
                         mpe::PlaybackEventList& result);

private:
    static void renderDiscreteGlissando(const Ms::Note* note, const RenderingContext& context, mpe::PlaybackEventList& result);
    static void renderContinuousGlissando(const Ms::Note* note, const RenderingContext& context, mpe::PlaybackEventList& result);
};
}

#endif // MU_ENGRAVING_GLISSANDOSRENDERER_H
