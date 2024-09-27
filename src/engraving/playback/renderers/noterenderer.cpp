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

#include "playback/metaparsers/chordarticulationsparser.h"
#include "playback/metaparsers/notearticulationsparser.h"

using namespace mu::engraving;
using namespace muse;
using namespace muse::mpe;

void NoteRenderer::render(const Note* note, const RenderingContext& ctx, mpe::PlaybackEventList& result)
{
    IF_ASSERT_FAILED(note) {
        return;
    }

    NominalNoteCtx noteCtx = buildNominalNoteCtx(note, ctx);

    if (!isNotePlayable(note, noteCtx.articulations)) {
        return;
    }

    if (const Tie* tieFor = note->tieFor()) {
        if (tieFor->playSpanner() && !tieFor->isLaissezVib()) {
            renderTiedNotes(note, noteCtx);
        }
    }

    applySwingIfNeed(note, noteCtx);

    if (noteCtx.articulations.contains(ArticulationType::DiscreteGlissando)) {
        GlissandosRenderer::renderDiscreteGlissando(note, noteCtx, result);
        return;
    }

    if (noteCtx.articulations.contains(ArticulationType::ContinuousGlissando)) {
        GlissandosRenderer::renderContinuousGlissando(note, noteCtx, result);
        return;
    }

    result.emplace_back(buildNoteEvent(std::move(noteCtx)));
}

void NoteRenderer::renderTiedNotes(const Note* firstNote, NominalNoteCtx& firstNoteCtx)
{
    std::unordered_set<const Note*> renderedNotes { firstNote };

    const RenderingContext& firstChordCtx = firstNoteCtx.chordCtx;
    const Tie* currTie = firstNote->tieFor();

    while (currTie && currTie->playSpanner()) {
        const Note* currNote = currTie->endNote();
        if (!currNote || !currNote->play()) {
            break;
        }

        if (muse::contains(renderedNotes, currNote)) {
            break; // prevents infinite loop
        }

        const Chord* chord = currNote->chord();
        if (!chord) {
            break;
        }

        RenderingContext currChordCtx = buildRenderingCtx(chord, firstChordCtx.positionTickOffset,
                                                          firstChordCtx.profile, firstChordCtx.playbackCtx);
        ChordArticulationsParser::buildChordArticulationMap(chord, currChordCtx, currChordCtx.commonArticulations);

        NominalNoteCtx currNoteCtx = buildNominalNoteCtx(currNote, currChordCtx);

        if (isNotePlayable(currNote, currNoteCtx.articulations)) {
            break;
        }

        firstNoteCtx.duration += currNoteCtx.duration;
        firstNoteCtx.articulations.insert(currNoteCtx.articulations.begin(), currNoteCtx.articulations.end());

        currTie = currNote->tieFor();
        renderedNotes.insert(currNote);
    }

    if (firstNoteCtx.articulations.size() > 1) {
        firstNoteCtx.articulations.erase(mpe::ArticulationType::Standard);
    }

    updateArticulationBoundaries(firstNoteCtx.timestamp, firstNoteCtx.duration, firstNoteCtx.articulations);
}

void NoteRenderer::updateArticulationBoundaries(const timestamp_t noteTimestamp, const duration_t noteDuration,
                                                ArticulationMap& articulations)
{
    const timestamp_t noteTimestampTo = noteTimestamp + noteDuration;
    IF_ASSERT_FAILED(noteTimestampTo > 0) {
        return;
    }

    for (const auto& pair : articulations) {
        const ArticulationAppliedData& articulation = pair.second;

        const duration_percentage_t occupiedFrom = mpe::occupiedPercentage(articulation.meta.timestamp,
                                                                           noteTimestampTo);
        const duration_percentage_t occupiedTo = mpe::occupiedPercentage(articulation.meta.timestamp + articulation.meta.overallDuration,
                                                                         noteTimestampTo);

        articulations.updateOccupiedRange(pair.first, occupiedFrom, occupiedTo);
    }
}

void NoteRenderer::applySwingIfNeed(const Note* note, NominalNoteCtx& noteCtx)
{
    const Chord* chord = note->chord();
    if (!chord || chord->tuplet()) {
        return;
    }

    const SwingParameters swing = chord->staff()->swing(chord->tick());
    if (!swing.isOn()) {
        return;
    }

    //! NOTE: Swing must be applied to the "raw" note duration, but not to the additional duration (e.g, from a tied note)
    const Swing::ChordDurationAdjustment swingDurationAdjustment = Swing::applySwing(chord, swing);
    const duration_t additionalDuration = noteCtx.duration - noteCtx.chordCtx.nominalDuration;
    noteCtx.timestamp = noteCtx.timestamp + noteCtx.chordCtx.nominalDuration * swingDurationAdjustment.remainingDurationMultiplier;
    noteCtx.duration = noteCtx.chordCtx.nominalDuration * swingDurationAdjustment.durationMultiplier + additionalDuration;
}

NominalNoteCtx NoteRenderer::buildNominalNoteCtx(const Note* note, const RenderingContext& ctx)
{
    NominalNoteCtx noteCtx(note, ctx);
    NoteArticulationsParser::buildNoteArticulationMap(note, ctx, noteCtx.articulations);

    return noteCtx;
}
