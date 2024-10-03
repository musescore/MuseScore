/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "gracechordsrenderer.h"

#include "gracechordcontext.h"
#include "chordarticulationsrenderer.h"
#include "bendsrenderer.h"
#include "noterenderer.h"

#include "playback/utils/expressionutils.h"

using namespace mu::engraving;
using namespace muse;
using namespace muse::mpe;

const ArticulationTypeSet& GraceChordsRenderer::supportedTypes()
{
    return GRACE_NOTE_ARTICULATION_TYPES;
}

void GraceChordsRenderer::doRender(const EngravingItem* item, const mpe::ArticulationType type,
                                   const RenderingContext& ctx,
                                   mpe::PlaybackEventList& result)
{
    const Chord* chord = toChord(item);
    IF_ASSERT_FAILED(chord) {
        return;
    }

    const GraceChordCtx graceChordCtx = GraceChordCtx::buildCtx(chord, type, ctx);

    if (isGraceNotePlacedBeforePrincipalNote(type)) {
        renderGraceNoteEvents(graceChordCtx, result);
        ChordArticulationsRenderer::render(chord, ArticulationType::Last, graceChordCtx.principalChordCtx, result);
    } else {
        ChordArticulationsRenderer::render(chord, ArticulationType::Last, graceChordCtx.principalChordCtx, result);
        renderGraceNoteEvents(graceChordCtx, result);
    }
}

void GraceChordsRenderer::renderGraceNoteEvents(const GraceChordCtx& graceChordCtx, mpe::PlaybackEventList& result)
{
    for (const auto& pair : graceChordCtx.graceChordCtxList) {
        for (const Note* graceNote : pair.first->notes()) {
            if (BendsRenderer::isMultibendStart(graceNote)) {
                BendsRenderer::render(graceNote, pair.second, result);
            } else {
                NoteRenderer::render(graceNote, pair.second, result);
            }
        }
    }
}
