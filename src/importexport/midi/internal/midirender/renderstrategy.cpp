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

#include "renderstrategy.h"
#include "midirendertypes.h"
#include "engraving/dom/note.h"

namespace mu::iex::midi {
void RenderStrategy::render(RenderContext& ctx, MidiRenderOutData& outData,
                            std::unordered_map<mu::engraving::EngravingItem*, Meta>& renderMeta)
{
    renderRegularNotes(ctx, renderMeta);
    renderGraceNotes(ctx, renderMeta);
    renderPalmMute(ctx, renderMeta);
    mergeEventsBefore(ctx, outData, renderMeta);
//        mergeEvents(ctx, outData, renderMeta);
//        mergeEventsAfter(ctx, outData, renderMeta);
}

void RenderStrategy::renderRegularNotes(RenderContext& ctx,
                                        std::unordered_map<mu::engraving::EngravingItem*, Meta>& renderMeta)
{
    for (const auto&[id, part]: ctx.parts()) {
        for (const auto&[tick, note]: part.regularNotes) {
            int on = tick;
            int off = on + note->playTicks();
            auto pitch = static_cast<uint8_t>(note->pitch());
            auto& meta = renderMeta.at(toEngravingItem(note));
            meta.noteOn = on;
            meta.noteOff = off;
            meta.pitch = pitch;
        }
    }
}

void RenderStrategy::renderGraceNotes(RenderContext& ctx,
                                      std::unordered_map<mu::engraving::EngravingItem*, Meta>& renderMeta)
{
    for (const auto&[id, part]: ctx.parts()) {
        for (const auto&[tick, note]: part.graceNotesBefore) {
            int on = tick - note->playTicks();
            int off = on + note->playTicks();
            auto pitch = static_cast<uint8_t>(note->pitch());
            auto& meta = renderMeta.at(toEngravingItem(note));
            meta.noteOn = on;
            meta.noteOff = off;
            meta.pitch = pitch;
        }
    }
}

void RenderStrategy::renderPalmMute(RenderContext& ctx,
                                    std::unordered_map<mu::engraving::EngravingItem*, Meta>& renderMeta)
{
    // TODO: setup program change
}

void RenderStrategy::mergeEventsBefore(mu::iex::midi::RenderContext& ctx, mu::iex::midi::MidiRenderOutData& outData,
                                       std::unordered_map<mu::engraving::EngravingItem*, Meta>& renderMeta)
{
    for (const auto&[id, part]: ctx.parts()) {
        for (const auto&[tick, note]: part.graceNotesBefore) {
            const auto& meta = renderMeta.at(toEngravingItem(note));
            for (auto i: meta.dependentItems) {
                auto& noteMeta = renderMeta.at(i);
                noteMeta.noteOff -= note->playTicks();
            }
        }
    }
}
}
