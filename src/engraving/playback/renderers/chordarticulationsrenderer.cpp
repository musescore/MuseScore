/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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

#include "chordarticulationsrenderer.h"

#include "bendsrenderer.h"
#include "noterenderer.h"
#include "gracechordcontext.h"
#include "ornamentsrenderer.h"
#include "tremolorenderer.h"
#include "arpeggiorenderer.h"

#include "playback/utils/expressionutils.h"

using namespace mu::engraving;
using namespace muse;
using namespace muse::mpe;

const mpe::ArticulationTypeSet& ChordArticulationsRenderer::supportedTypes()
{
    static mpe::ArticulationTypeSet SUPPORTED_TYPES;

    if (SUPPORTED_TYPES.empty()) {
        SUPPORTED_TYPES = GRACE_NOTE_ARTICULATION_TYPES;

        SUPPORTED_TYPES.insert(OrnamentsRenderer::supportedTypes().cbegin(),
                               OrnamentsRenderer::supportedTypes().cend());
        SUPPORTED_TYPES.insert(TremoloRenderer::supportedTypes().cbegin(),
                               TremoloRenderer::supportedTypes().cend());
        SUPPORTED_TYPES.insert(ArpeggioRenderer::supportedTypes().cbegin(),
                               ArpeggioRenderer::supportedTypes().cend());
    }

    return SUPPORTED_TYPES;
}

void ChordArticulationsRenderer::doRender(const EngravingItem* item, const mpe::ArticulationType /*type*/, const RenderingContext& ctx,
                                          mpe::PlaybackEventList& result)
{
    IF_ASSERT_FAILED(item->type() == ElementType::CHORD) {
        return;
    }

    for (const auto& type : ctx.commonArticulations) {
        if (muse::contains(GRACE_NOTE_ARTICULATION_TYPES, type.first)) {
            renderChordWithGraceChords(toChord(item), type.first, ctx, result);
            return;
        }
    }

    renderChord(toChord(item), ctx, result);
}

void ChordArticulationsRenderer::renderChord(const Chord* chord, const RenderingContext& ctx, muse::mpe::PlaybackEventList& result)
{
    if (renderChordArticulations(chord, ctx, result)) {
        return;
    }

    const bool supportsMultibend = ctx.profile->contains(ArticulationType::Multibend);

    for (const Note* note: chord->notes()) {
        if (supportsMultibend && BendsRenderer::isMultibendPart(note)) {
            BendsRenderer::render(note, ctx, result);
        } else {
            NoteRenderer::render(note, ctx, result);
        }
    }
}

void ChordArticulationsRenderer::renderChordWithGraceChords(const Chord* chord, const muse::mpe::ArticulationType type,
                                                            const RenderingContext& ctx,
                                                            muse::mpe::PlaybackEventList& result)
{
    const GraceChordCtx graceChordCtx = GraceChordCtx::buildCtx(chord, type, ctx);

    if (isGraceNotePlacedBeforePrincipalNote(type)) {
        for (const auto& pair : graceChordCtx.graceChordCtxList) {
            renderChord(pair.first, pair.second, result);
        }
        renderChord(chord, graceChordCtx.principalChordCtx, result);
    } else {
        renderChord(chord, graceChordCtx.principalChordCtx, result);
        for (const auto& pair : graceChordCtx.graceChordCtxList) {
            renderChord(pair.first, pair.second, result);
        }
    }
}

bool ChordArticulationsRenderer::renderChordArticulations(const Chord* chord, const RenderingContext& ctx,
                                                          mpe::PlaybackEventList& result)
{
    for (const auto& pair : ctx.commonArticulations) {
        const mpe::ArticulationType type = pair.first;

        if (OrnamentsRenderer::isAbleToRender(type)) {
            OrnamentsRenderer::render(chord, type, ctx, result);
            return true;
        }

        if (TremoloRenderer::isAbleToRender(type)) {
            TremoloRenderer::render(chord, type, ctx, result);
            return true;
        }

        if (ArpeggioRenderer::isAbleToRender(type)) {
            ArpeggioRenderer::render(chord, type, ctx, result);
            return true;
        }
    }

    return false;
}
