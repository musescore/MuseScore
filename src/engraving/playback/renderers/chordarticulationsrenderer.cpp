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

#include "chordarticulationsrenderer.h"

#include "libmscore/tempo.h"
#include "libmscore/staff.h"
#include "libmscore/swing.h"

#include "playback/metaparsers/notearticulationsparser.h"
#include "ornamentsrenderer.h"
#include "tremolorenderer.h"
#include "arpeggiorenderer.h"
#include "glissandosrenderer.h"

using namespace mu;
using namespace mu::engraving;
using namespace mu::mpe;

const mpe::ArticulationTypeSet& ChordArticulationsRenderer::supportedTypes()
{
    static mpe::ArticulationTypeSet SUPPORTED_TYPES;

    if (SUPPORTED_TYPES.empty()) {
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

    const Chord* chord = toChord(item);

    if (renderChordArticulations(chord, ctx, result)) {
        return;
    }

    renderNoteArticulations(chord, ctx, result);
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

void ChordArticulationsRenderer::renderNoteArticulations(const Chord* chord, const RenderingContext& ctx,
                                                         mpe::PlaybackEventList& result)
{
    Swing::ChordDurationAdjustment swingDurationAdjustment;

    if (!chord->tuplet()) {
        SwingParameters swing = chord->staff()->swing(chord->tick());

        if (swing.isOn()) {
            swingDurationAdjustment = Swing::applySwing(chord, swing);
        }
    }

    for (const Note* note : chord->notes()) {
        NominalNoteCtx noteCtx(note, ctx);

        NoteArticulationsParser::buildNoteArticulationMap(note, ctx, noteCtx.chordCtx.commonArticulations);

        if (!isNotePlayable(note, noteCtx.chordCtx.commonArticulations)) {
            continue;
        }

        if (!swingDurationAdjustment.isNull()) {
            //! NOTE: Swing must be applied to the "raw" note duration, but not to the additional duration (e.g, from a tied note)
            duration_t additionalDuration = noteCtx.duration - ctx.nominalDuration;
            noteCtx.timestamp = noteCtx.timestamp + ctx.nominalDuration * swingDurationAdjustment.remainingDurationMultiplier;
            noteCtx.duration = ctx.nominalDuration * swingDurationAdjustment.durationMultiplier + additionalDuration;
        }

        if (note->tieFor()) {
            noteCtx.duration = tiedNotesTotalDuration(note);
            result.emplace_back(buildNoteEvent(std::move(noteCtx)));
            continue;
        }

        if (noteCtx.chordCtx.commonArticulations.contains(ArticulationType::DiscreteGlissando)) {
            GlissandosRenderer::render(note, ArticulationType::DiscreteGlissando, noteCtx.chordCtx, result);
            continue;
        }

        if (noteCtx.chordCtx.commonArticulations.contains(ArticulationType::ContinuousGlissando)) {
            GlissandosRenderer::render(note, ArticulationType::ContinuousGlissando, noteCtx.chordCtx, result);
            continue;
        }

        result.emplace_back(buildNoteEvent(std::move(noteCtx)));
    }
}

duration_t ChordArticulationsRenderer::tiedNotesTotalDuration(const Note* firstNote)
{
    mpe::duration_t result = 0;

    const Score* score = firstNote->score();
    const std::vector<Note*> tiedNotes = firstNote->tiedNotes();

    for (const Note* tiedNote : tiedNotes) {
        if (!tiedNote || !tiedNote->chord()) {
            continue;
        }

        BeatsPerSecond bps = score->tempomap()->tempo(tiedNote->tick().ticks());
        result += durationFromTicks(bps.val, tiedNote->chord()->actualTicks().ticks());
    }

    return result;
}
