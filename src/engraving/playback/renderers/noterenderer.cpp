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

#include "playback/utils/repeatutils.h"

using namespace mu::engraving;
using namespace muse;
using namespace muse::mpe;

bool NoteRenderer::shouldRender(const Note* note, const RenderingContext& ctx, const muse::mpe::ArticulationMap& articulations)
{
    if (!note->play()) {
        return false;
    }

    const Tie* tie = note->tieBack();

    if (tie && tie->playSpanner()) {
        //!Note Checking whether the tied note has any multi-note articulation attached
        //!     If so, we can't ignore such note
        for (const auto& pair : articulations) {
            if (muse::mpe::isMultiNoteArticulation(pair.first) && !muse::mpe::isRangedArticulation(pair.first)) {
                return true;
            }
        }

        const Note* startNote = tie->startNote();
        const Chord* startChord = startNote ? startNote->chord() : nullptr;
        if (startChord) {
            if (startChord->tremoloType() != TremoloType::INVALID_TREMOLO) {
                return true;
            }
        }

        const Note* endNote = tie->endNote();
        const Chord* endChord = endNote ? endNote->chord() : nullptr;
        if (endChord) {
            if (endChord->tremoloType() != TremoloType::INVALID_TREMOLO) {
                return true;
            }
        }

        if (tie->isPartialTie()) {
            return !findOutgoingNote(note, ctx).isValid();
        }

        if (!startChord || !endChord) {
            return false;
        }

        const auto& intervals = startChord->score()->spannerMap().findOverlapping(startChord->tick().ticks(),
                                                                                  startChord->endTick().ticks(),
                                                                                  /*excludeCollisions*/ true);
        for (const auto& interval : intervals) {
            const Spanner* sp = interval.value;
            if (sp->isTrill() && sp->playSpanner() && sp->endElement() == startChord) {
                return true;
            }
        }

        return false;
    }

    return true;
}

void NoteRenderer::render(const Note* note, const RenderingContext& ctx, mpe::PlaybackEventList& result)
{
    IF_ASSERT_FAILED(note) {
        return;
    }

    NominalNoteCtx noteCtx = buildNominalNoteCtx(note, ctx);
    if (!shouldRender(note, ctx, noteCtx.articulations)) {
        return;
    }

    const Tie* tieFor = note->tieFor();
    if (tieFor && tieFor->playSpanner()) {
        if (tieFor->isPartialTie()) {
            renderPartialTie(note, noteCtx);
        } else if (!tieFor->isLaissezVib()) {
            renderNormalTie(note, noteCtx);
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

    mpe::NoteEvent ev = buildNoteEvent(std::move(noteCtx));

    if (ev.arrangementCtx().actualTimestamp >= 0) {
        result.emplace_back(std::move(ev));
    } else {
        ArrangementContext arrCtx = ev.arrangementCtx();
        arrCtx.actualDuration = arrCtx.actualDuration + arrCtx.actualTimestamp;
        arrCtx.actualTimestamp = 0;

        PitchContext pitchCtx = ev.pitchCtx();
        ExpressionContext expCtx = ev.expressionCtx();

        result.emplace_back(mpe::NoteEvent(std::move(arrCtx), std::move(pitchCtx), std::move(expCtx)));
    }
}

void NoteRenderer::renderPartialTie(const Note* outgoingNote, NominalNoteCtx& outgoingNoteCtx)
{
    const RenderingContext& outgoingChordCtx = outgoingNoteCtx.chordCtx;
    const PartiallyTiedNoteInfo incomingNoteInfo = findIncomingNote(outgoingNote, outgoingChordCtx);
    if (!incomingNoteInfo.isValid()) {
        return;
    }

    const int incomingNotePositionTickOffset = incomingNoteInfo.repeat->utick - incomingNoteInfo.repeat->tick;
    RenderingContext incomingChordCtx = buildRenderingCtx(incomingNoteInfo.note->chord(), incomingNotePositionTickOffset,
                                                          outgoingChordCtx.profile, outgoingChordCtx.playbackCtx);

    ChordArticulationsParser::buildChordArticulationMap(incomingNoteInfo.note->chord(), incomingChordCtx,
                                                        incomingChordCtx.commonArticulations);

    NominalNoteCtx incomingNoteCtx = buildNominalNoteCtx(incomingNoteInfo.note, incomingChordCtx);

    if (shouldRender(incomingNoteInfo.note, incomingChordCtx, incomingNoteCtx.articulations)) {
        return;
    }

    const Tie* tieFor = incomingNoteInfo.note->tieFor();
    if (tieFor && tieFor->playSpanner() && !tieFor->isLaissezVib()) {
        renderNormalTie(incomingNoteInfo.note, incomingNoteCtx);
    }

    addTiedNote(incomingNoteCtx, outgoingNoteCtx);
    updateArticulationBoundaries(outgoingNoteCtx.timestamp, outgoingNoteCtx.duration, outgoingNoteCtx.articulations);
}

void NoteRenderer::renderNormalTie(const Note* firstNote, NominalNoteCtx& firstNoteCtx)
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

        if (!notesInSameRepeat(firstChordCtx.score, firstNote, currNote, firstChordCtx.positionTickOffset)) {
            const TieJumpPointList* jumpPoints = firstNote->tieJumpPoints();
            if (jumpPoints && !jumpPoints->empty()) {
                renderPartialTie(firstNote, firstNoteCtx);
            }
            break;
        }

        const Chord* chord = currNote->chord();
        if (!chord) {
            break;
        }

        RenderingContext currChordCtx = buildRenderingCtx(chord, firstChordCtx.positionTickOffset,
                                                          firstChordCtx.profile, firstChordCtx.playbackCtx);
        ChordArticulationsParser::buildChordArticulationMap(chord, currChordCtx, currChordCtx.commonArticulations);

        const NominalNoteCtx currNoteCtx = buildNominalNoteCtx(currNote, currChordCtx);
        if (shouldRender(currNote, currChordCtx, currNoteCtx.articulations)) {
            if (currNoteCtx.articulations.contains(ArticulationType::DiscreteGlissando)) {
                firstNoteCtx.duration += GlissandosRenderer::discreteGlissandoStepDuration(currNote, currNoteCtx.duration);
            }

            break;
        }

        addTiedNote(currNoteCtx, firstNoteCtx);

        currTie = currNote->tieFor();
        renderedNotes.insert(currNote);
    }

    if (firstNoteCtx.articulations.size() > 1) {
        firstNoteCtx.articulations.erase(mpe::ArticulationType::Standard);
    }

    updateArticulationBoundaries(firstNoteCtx.timestamp, firstNoteCtx.duration, firstNoteCtx.articulations);
}

void NoteRenderer::addTiedNote(const NominalNoteCtx& tiedNoteCtx, NominalNoteCtx& firstNoteCtx)
{
    if (tiedNoteCtx.articulations.size() == 1) {
        if (tiedNoteCtx.articulations.begin()->first == mpe::ArticulationType::Standard) {
            firstNoteCtx.duration += tiedNoteCtx.duration;
            return;
        }
    }

    const float avgDurationFactor = percentageToFactor(tiedNoteCtx.articulations.averageDurationFactor());
    firstNoteCtx.duration += tiedNoteCtx.duration * avgDurationFactor;

    // Ignore these articulations so we won't re-apply them to the total duration
    static const ArticulationTypeSet ARTICULATION_TO_IGNORE_TYPES {
        ArticulationType::Staccato,
        ArticulationType::Staccatissimo,
    };

    for (const auto& pair : tiedNoteCtx.articulations) {
        if (!muse::contains(ARTICULATION_TO_IGNORE_TYPES, pair.first)) {
            firstNoteCtx.articulations.insert(pair);
        }
    }
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
