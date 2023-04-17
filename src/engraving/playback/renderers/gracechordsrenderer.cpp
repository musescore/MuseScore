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

#include "libmscore/chord.h"

#include "chordarticulationsrenderer.h"

using namespace mu::engraving;
using namespace mu::mpe;

const ArticulationTypeSet& GraceChordsRenderer::supportedTypes()
{
    static const mpe::ArticulationTypeSet types = {
        mpe::ArticulationType::Acciaccatura, mpe::ArticulationType::PostAppoggiatura,
        mpe::ArticulationType::PreAppoggiatura
    };

    return types;
}

void GraceChordsRenderer::doRender(const EngravingItem* item, const mpe::ArticulationType type,
                                   const RenderingContext& context,
                                   mpe::PlaybackEventList& result)
{
    const Chord* chord = toChord(item);

    IF_ASSERT_FAILED(chord) {
        return;
    }

    if (isPlacedBeforePrincipalNote(type)) {
        renderPrependedGraceNotes(chord, context, type, result);
    } else {
        renderAppendedGraceNotes(chord, context, type, result);
    }
}

bool GraceChordsRenderer::isPlacedBeforePrincipalNote(const mpe::ArticulationType type)
{
    if (type == ArticulationType::Acciaccatura || type == ArticulationType::PreAppoggiatura) {
        return true;
    }

    return false;
}

void GraceChordsRenderer::renderPrependedGraceNotes(const Chord* chord, const RenderingContext& context,
                                                    const mpe::ArticulationType type,
                                                    mpe::PlaybackEventList& result)
{
    const std::vector<Chord*>& graceChords = chord->graceNotesBefore();

    duration_t actualGraceNotesDuration = graceNotesMaxAvailableDuration(type, context, graceChords.size());
    timestamp_t graceNotesTimestampFrom = graceNotesStartTimestamp(type, actualGraceNotesDuration, context.nominalTimestamp);

    timestamp_t principalNotesTimestampFrom = principalNotesStartTimestamp(type, actualGraceNotesDuration, context.nominalTimestamp);
    duration_t totalPrincipalNotesDuration = principalNotesDuration(actualGraceNotesDuration, context.nominalDuration);

    buildGraceNoteEvents(graceChords, context, type, graceNotesTimestampFrom, actualGraceNotesDuration, result);

    buildPrincipalNoteEvents(chord, context, type, principalNotesTimestampFrom, totalPrincipalNotesDuration, result);
}

void GraceChordsRenderer::renderAppendedGraceNotes(const Chord* chord, const RenderingContext& context,
                                                   const mpe::ArticulationType type,
                                                   mpe::PlaybackEventList& result)
{
    const std::vector<Chord*>& graceChords = chord->graceNotesAfter();

    duration_t actualGraceNotesDuration = graceNotesMaxAvailableDuration(type, context, graceChords.size());
    timestamp_t graceNotesTimestampFrom = graceNotesStartTimestamp(type, actualGraceNotesDuration,
                                                                   context.nominalTimestamp + context.nominalDuration);

    timestamp_t principalNotesTimestampFrom = principalNotesStartTimestamp(type, actualGraceNotesDuration, context.nominalTimestamp);
    duration_t totalPrincipalNotesDuration = principalNotesDuration(actualGraceNotesDuration, context.nominalDuration);

    buildPrincipalNoteEvents(chord, context, type, principalNotesTimestampFrom, totalPrincipalNotesDuration, result);

    buildGraceNoteEvents(graceChords, context, type, graceNotesTimestampFrom, actualGraceNotesDuration, result);
}

duration_t GraceChordsRenderer::graceNotesTotalDuration(const std::vector<Chord*>& graceChords, const RenderingContext& context)
{
    duration_t result = 0;

    for (const Chord* graceChord : graceChords) {
        result += durationFromTicks(context.beatsPerSecond.val, graceChord->durationTypeTicks().ticks());
    }

    return result;
}

float GraceChordsRenderer::graceNotesDurationRatio(const mpe::duration_t totalDuration, const duration_t maxAvailableDuration)
{
    float result = 1.f;

    if (totalDuration > maxAvailableDuration) {
        result = maxAvailableDuration / static_cast<float>(totalDuration);
    }

    return result;
}

void GraceChordsRenderer::buildGraceNoteEvents(const std::vector<Chord*>& graceChords, const RenderingContext& context,
                                               const ArticulationType type,
                                               const timestamp_t timestampFrom,
                                               const duration_t availableDuration,
                                               mpe::PlaybackEventList& result)
{
    for (size_t i = 0; i < graceChords.size(); ++i) {
        for (const Note* graceNote : graceChords[i]->notes()) {
            if (!isNotePlayable(graceNote, context.commonArticulations)) {
                continue;
            }

            NominalNoteCtx noteCtx(graceNote, context);
            noteCtx.duration = RealRound(availableDuration / static_cast<float>(graceChords.size()), 0);
            noteCtx.timestamp = timestampFrom + i * noteCtx.duration;

            updateArticulationBoundaries(type, noteCtx.timestamp, noteCtx.duration, noteCtx.chordCtx.commonArticulations);

            result.emplace_back(buildNoteEvent(std::move(noteCtx)));
        }
    }
}

void GraceChordsRenderer::buildPrincipalNoteEvents(const Chord* chord, const RenderingContext& ctx,
                                                   const ArticulationType type,
                                                   const mpe::timestamp_t timestamp,
                                                   const mpe::duration_t duration,
                                                   mpe::PlaybackEventList& result)
{
    RenderingContext principalCtx = ctx;

    principalCtx.nominalDuration = duration;
    principalCtx.nominalTimestamp = timestamp;
    principalCtx.nominalPositionStartTick = timestampToTick(chord->score(), timestamp) - ctx.positionTickOffset;
    principalCtx.nominalPositionEndTick = timestampToTick(chord->score(), timestamp + duration) - ctx.positionTickOffset;
    principalCtx.nominalDurationTicks = principalCtx.nominalPositionEndTick - principalCtx.nominalPositionStartTick;

    updateArticulationBoundaries(type, timestamp, duration, principalCtx.commonArticulations);

    ChordArticulationsRenderer::render(chord, ArticulationType::Last, principalCtx, result);
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
