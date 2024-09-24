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

#include "noterenderer.h"

#include "dom/note.h"
#include "dom/staff.h"
#include "dom/swing.h"

#include "glissandosrenderer.h"

#include "playback/metaparsers/notearticulationsparser.h"

using namespace mu::engraving;
using namespace muse;
using namespace muse::mpe;

void NoteRenderer::render(const Note* note, const RenderingContext& ctx, mpe::PlaybackEventList& result)
{
    NominalNoteCtx noteCtx = buildNominalNoteCtx(note, ctx);

    if (isNotePlayable(note, noteCtx.chordCtx.commonArticulations)) {
        doRenderNote(note, std::move(noteCtx), result);
    }
}

void NoteRenderer::doRenderNote(const Note* note, NominalNoteCtx&& noteCtx, mpe::PlaybackEventList& result)
{
    const Chord* chord = note->chord();
    IF_ASSERT_FAILED(chord) {
        return;
    }

    Swing::ChordDurationAdjustment swingDurationAdjustment;

    if (!chord->tuplet()) {
        SwingParameters swing = chord->staff()->swing(chord->tick());

        if (swing.isOn()) {
            swingDurationAdjustment = Swing::applySwing(chord, swing);
        }
    }

    const RenderingContext& ctx = noteCtx.chordCtx;

    auto applySwingToNoteCtx = [&swingDurationAdjustment, &ctx](NominalNoteCtx& noteCtx) {
        if (swingDurationAdjustment.isNull()) {
            return;
        }

        //! NOTE: Swing must be applied to the "raw" note duration, but not to the additional duration (e.g, from a tied note)
        duration_t additionalDuration = noteCtx.duration - ctx.nominalDuration;
        noteCtx.timestamp = noteCtx.timestamp + ctx.nominalDuration * swingDurationAdjustment.remainingDurationMultiplier;
        noteCtx.duration = ctx.nominalDuration * swingDurationAdjustment.durationMultiplier + additionalDuration;
    };

    if (note->tieFor()) {
        noteCtx.duration = tiedNotesTotalDuration(note->score(), note, noteCtx.duration, ctx.positionTickOffset);
        applySwingToNoteCtx(noteCtx);
        result.emplace_back(buildNoteEvent(std::move(noteCtx)));
        return;
    }

    applySwingToNoteCtx(noteCtx);

    if (noteCtx.chordCtx.commonArticulations.contains(ArticulationType::DiscreteGlissando)) {
        GlissandosRenderer::render(note, ArticulationType::DiscreteGlissando, noteCtx.chordCtx, result);
        return;
    }

    if (noteCtx.chordCtx.commonArticulations.contains(ArticulationType::ContinuousGlissando)) {
        GlissandosRenderer::render(note, ArticulationType::ContinuousGlissando, noteCtx.chordCtx, result);
        return;
    }

    result.emplace_back(buildNoteEvent(std::move(noteCtx)));
}

NominalNoteCtx NoteRenderer::buildNominalNoteCtx(const Note* note, const RenderingContext& ctx)
{
    NominalNoteCtx noteCtx(note, ctx);
    NoteArticulationsParser::buildNoteArticulationMap(note, ctx, noteCtx.chordCtx.commonArticulations);

    return noteCtx;
}
