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
class ArpeggioRenderer : public RenderBase<ArpeggioRenderer>
{
public:
    const mpe::ArticulationTypeSet& supportedTypes() const;

    void doRender(const Ms::EngravingItem* item, const mpe::ArticulationType preferredType, PlaybackContext &&context,
                  mpe::PlaybackEventList& result) const;

private:
    bool isDirectionUp(const mpe::ArticulationType type) const;
};
}

#endif // MU_ENGRAVING_ARPEGGIORENDERER_H
