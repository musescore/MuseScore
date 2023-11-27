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

#include "gracechordsrenderer.h"

#include "dom/chord.h"

#include "chordarticulationsrenderer.h"

#include "playback/metaparsers/notearticulationsparser.h"

using namespace mu::engraving;
using namespace mu::mpe;

static constexpr bool FILTER_UNPLAYABLE = true;

const ArticulationTypeSet& GraceChordsRenderer::supportedTypes()
{
    static const mpe::ArticulationTypeSet types = {
        mpe::ArticulationType::Acciaccatura, mpe::ArticulationType::PostAppoggiatura,
        mpe::ArticulationType::PreAppoggiatura
    };

    return types;
}

void GraceChordsRenderer::doRender(const EngravingItem* item, const mpe::ArticulationType type,
                                   const RenderingContext& ctx,
                                   mpe::PlaybackEventList& result)
{
    const Chord* chord = toChord(item);

    IF_ASSERT_FAILED(chord) {
        return;
    }

    auto graceNoteAcceped = [](const Note* note, const RenderingContext& ctx) {
        return isNotePlayable(note, ctx.commonArticulations);
    };

    if (isPlacedBeforePrincipalNote(type)) {
        const std::vector<Chord*>& graceChords = chord->graceNotesBefore();
        GraceNotesContext graceNoteCtx = buildGraceNotesContext(graceChords, ctx, type);

        renderGraceNoteEvents(graceChords, graceNoteAcceped, ctx, graceNoteCtx, result);
        renderPrincipalChord(chord, ctx, graceNoteCtx, result);
    } else {
        const std::vector<Chord*>& graceChords = chord->graceNotesAfter();
        GraceNotesContext graceNoteCtx = buildGraceNotesContext(graceChords, ctx, type);

        renderPrincipalChord(chord, ctx, graceNoteCtx, result);
        renderGraceNoteEvents(graceChords, graceNoteAcceped, ctx, graceNoteCtx, result);
    }
}

void GraceChordsRenderer::renderGraceNote(const Note* graceNote, const Note* principalNote, const mpe::ArticulationType graceNoteType,
                                          const RenderingContext& ctx, mpe::PlaybackEventList& result)
{
    auto graceNoteAccepted = [graceNote](const Note* note, const RenderingContext&) {
        return note == graceNote;
    };

    if (isPlacedBeforePrincipalNote(graceNoteType)) {
        const std::vector<Chord*>& graceChords = principalNote->chord()->graceNotesBefore(FILTER_UNPLAYABLE);
        GraceNotesContext graceCtx = buildGraceNotesContext(graceChords, ctx, graceNoteType);

        renderGraceNoteEvents(graceChords, graceNoteAccepted, ctx, graceCtx, result);
        renderPrincipalNote(principalNote, ctx, graceCtx, result);
    } else {
        const std::vector<Chord*>& graceChords = principalNote->chord()->graceNotesAfter();
        GraceNotesContext graceCtx = buildGraceNotesContext(graceChords, ctx, graceNoteType);

        renderPrincipalNote(principalNote, ctx, graceCtx, result);
        renderGraceNoteEvents(graceChords, graceNoteAccepted, ctx, graceCtx, result);
    }
}

bool GraceChordsRenderer::isPlacedBeforePrincipalNote(const mpe::ArticulationType type)
{
    if (type == ArticulationType::Acciaccatura || type == ArticulationType::PreAppoggiatura) {
        return true;
    }

    return false;
}

void GraceChordsRenderer::renderGraceNoteEvents(const std::vector<Chord*>& graceChords,
                                                GraceNoteAccepted graceNoteAccepted,
                                                const RenderingContext& ctx,
                                                const GraceNotesContext& graceCtx,
                                                mpe::PlaybackEventList& result)
{
    timestamp_t timestamp = graceCtx.graceNotesTimestampFrom;

    for (const Chord* graceChord : graceChords) {
        duration_t duration
            = RealRound(graceCtx.durationFactor * durationFromTicks(ctx.beatsPerSecond.val, graceChord->durationTypeTicks().ticks()), 0);

        for (const Note* graceNote : graceChord->notes()) {
            if (!graceNoteAccepted(graceNote, ctx)) {
                continue;
            }

            NominalNoteCtx noteCtx(graceNote, ctx);
            noteCtx.duration = duration;
            noteCtx.timestamp = timestamp;

            NoteArticulationsParser::buildNoteArticulationMap(graceNote, ctx, noteCtx.chordCtx.commonArticulations);
            updateArticulationBoundaries(graceCtx.type, noteCtx.timestamp, noteCtx.duration, noteCtx.chordCtx.commonArticulations);

            result.emplace_back(buildNoteEvent(std::move(noteCtx)));
        }

        timestamp += duration;
    }
}

void GraceChordsRenderer::renderPrincipalChord(const Chord* chord, const RenderingContext& ctx, const GraceNotesContext& graceCtx,
                                               mpe::PlaybackEventList& result)
{
    RenderingContext principalCtx = buildPrincipalNoteCtx(chord->score(), ctx, graceCtx);

    ChordArticulationsRenderer::render(chord, ArticulationType::Last, principalCtx, result);
}

void GraceChordsRenderer::renderPrincipalNote(const Note* note, const RenderingContext& ctx, const GraceNotesContext& graceCtx,
                                              mpe::PlaybackEventList& result)
{
    RenderingContext principalCtx = buildPrincipalNoteCtx(note->score(), ctx, graceCtx);

    ChordArticulationsRenderer::renderNote(note->chord(), note, principalCtx, result);
}

GraceChordsRenderer::GraceNotesContext GraceChordsRenderer::buildGraceNotesContext(const std::vector<Chord*>& graceChords,
                                                                                   const RenderingContext& ctx,
                                                                                   const mpe::ArticulationType type)
{
    duration_t availableGraceNotesDuration = graceNotesMaxAvailableDuration(type, ctx, graceChords.size());
    duration_t accumulatedGraceNotesDuration = graceNotesTotalDuration(graceChords, ctx);
    duration_t actualGraceNotesDuration = std::min(availableGraceNotesDuration, accumulatedGraceNotesDuration);

    GraceNotesContext result;
    result.type = type;
    result.durationFactor = double(actualGraceNotesDuration) / accumulatedGraceNotesDuration;

    timestamp_t graceNotesTimeOffset = isPlacedBeforePrincipalNote(type) ? 0 : ctx.nominalDuration;
    result.graceNotesTimestampFrom = graceNotesStartTimestamp(type, actualGraceNotesDuration,
                                                              ctx.nominalTimestamp + graceNotesTimeOffset);

    result.principalNotesTimestampFrom = principalNotesStartTimestamp(type, actualGraceNotesDuration, ctx.nominalTimestamp);
    result.totalPrincipalNotesDuration = principalNotesDuration(actualGraceNotesDuration, ctx.nominalDuration);

    return result;
}

RenderingContext GraceChordsRenderer::buildPrincipalNoteCtx(const Score* score, const RenderingContext& ctx,
                                                            const GraceNotesContext& graceCtx)
{
    mpe::timestamp_t timestamp = graceCtx.principalNotesTimestampFrom;
    mpe::duration_t duration = graceCtx.totalPrincipalNotesDuration;

    RenderingContext principalCtx = ctx;
    principalCtx.nominalDuration = duration;
    principalCtx.nominalTimestamp = timestamp;
    principalCtx.nominalPositionStartTick = timestampToTick(score, timestamp) - ctx.positionTickOffset;
    principalCtx.nominalPositionEndTick = timestampToTick(score, timestamp + duration) - ctx.positionTickOffset;
    principalCtx.nominalDurationTicks = principalCtx.nominalPositionEndTick - principalCtx.nominalPositionStartTick;

    updateArticulationBoundaries(graceCtx.type, timestamp, duration, principalCtx.commonArticulations);

    return principalCtx;
}

duration_t GraceChordsRenderer::graceNotesTotalDuration(const std::vector<Chord*>& graceChords, const RenderingContext& context)
{
    duration_t result = 0;

    for (const Chord* graceChord : graceChords) {
        result += durationFromTicks(context.beatsPerSecond.val, graceChord->durationTypeTicks().ticks());
    }

    return result;
}

duration_t GraceChordsRenderer::graceNotesMaxAvailableDuration(const ArticulationType type, const RenderingContext& ctx,
                                                               const size_t graceNotesCount)
{
    duration_t halvedDuration = 0.5 * ctx.nominalDuration;
    duration_t twoThirdsDuration = (2 * ctx.nominalDuration) / 3;

    if ((type == ArticulationType::PostAppoggiatura
         || (type == ArticulationType::PreAppoggiatura && graceNotesCount == 1))) {
        if (ctx.timeSignatureFraction.isCompound() && ctx.nominalDurationTicks > QUAVER_TICKS) {
            return twoThirdsDuration;
        } else {
            return halvedDuration;
        }
    }

    duration_t minAcciacaturaDuration = durationFromTicks(ctx.beatsPerSecond.val, DEMISEMIQUAVER_TICKS / 2);

    return std::min(minAcciacaturaDuration * static_cast<duration_t>(graceNotesCount), halvedDuration);
}

timestamp_t GraceChordsRenderer::graceNotesStartTimestamp(const mpe::ArticulationType type, const mpe::duration_t availableDuration,
                                                          const mpe::timestamp_t& nominalTimestamp)
{
    if (type == ArticulationType::PostAppoggiatura) {
        return nominalTimestamp - availableDuration;
    }

    return nominalTimestamp;
}

timestamp_t GraceChordsRenderer::principalNotesStartTimestamp(const mpe::ArticulationType type, const mpe::duration_t graceNotesDuration,
                                                              const mpe::timestamp_t& nominalTimestamp)
{
    if (type == ArticulationType::PreAppoggiatura
        || type == ArticulationType::Acciaccatura) {
        return nominalTimestamp + graceNotesDuration;
    }

    return nominalTimestamp;
}

timestamp_t GraceChordsRenderer::principalNotesDuration(const mpe::duration_t graceNotesDuration,
                                                        const mpe::duration_t& nominalDuration)
{
    return nominalDuration - graceNotesDuration;
}
