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

#ifndef MU_IMPORTEXPORT_MIDIRENDER_HPP
#define MU_IMPORTEXPORT_MIDIRENDER_HPP

#include "importexport/midi/imidirender.h"
#include "midirendertypes.h"

namespace mu::iex::midi {
class RenderStrategy;

class MidiRender : public IMidiRender
{
public:
    ~MidiRender() override = default;

    void render(mu::engraving::Score* score, RenderContext& ctx, MidiRenderOutData& outData, RenderStrategy& strategy) override;

private:
    RenderContext m_ctx;
    mu::engraving::Score* m_score;

    std::unordered_map<mu::engraving::EngravingItem*, Meta> m_renderMeta;

    void collectNotes();

    void
    collectGraceNotes(bool isBefore, const mu::engraving::Chord* chord, int mainChordStartTick, uint64_t partID,
                      mu::engraving::Chord* dependentItem);
};
}

#endif //MU_IMPORTEXPORT_MIDIRENDER_HPP
