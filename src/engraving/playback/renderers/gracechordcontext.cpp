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

#include "gracechordcontext.h"

#include "playback/utils/expressionutils.h"

using namespace mu::engraving;
using namespace muse;
using namespace muse::mpe;

static duration_t graceNotesTotalDuration(const std::vector<Chord*>& graceChords, const BeatsPerSecond& bps)
{
    duration_t result = 0;

    for (const Chord* graceChord : graceChords) {
        result += durationFromTempoAndTicks(bps.val, graceChord->durationTypeTicks().ticks());
    }

    return result;
}

static duration_t graceNotesMaxAvailableDuration(const ArticulationType type, const RenderingContext& ctx,
                                                 const size_t graceNotesCount)
{
    const duration_t halvedDuration = 0.5 * ctx.nominalDuration;
    const duration_t twoThirdsDuration = (2 * ctx.nominalDuration) / 3;

    if ((type == ArticulationType::PostAppoggiatura
         || (type == ArticulationType::PreAppoggiatura && graceNotesCount == 1))) {
        if (ctx.timeSignatureFraction.isCompound() && ctx.nominalDurationTicks > QUAVER_TICKS) {
            return twoThirdsDuration;
        } else {
            return halvedDuration;
        }
    }

    const duration_t minAcciacaturaDuration = durationFromTempoAndTicks(ctx.beatsPerSecond.val, DEMISEMIQUAVER_TICKS / 2);
    return std::min(minAcciacaturaDuration * static_cast<duration_t>(graceNotesCount), halvedDuration);
}

static timestamp_t graceNotesStartTimestamp(const mpe::ArticulationType type, const mpe::duration_t availableDuration,
                                            const mpe::timestamp_t nominalTimestamp)
{
    if (type == ArticulationType::PostAppoggiatura) {
        return nominalTimestamp - availableDuration;
    }

    return nominalTimestamp;
}

static timestamp_t principalNotesStartTimestamp(const mpe::ArticulationType type, const mpe::duration_t graceNotesDuration,
                                                const mpe::timestamp_t nominalTimestamp)
{
    if (type == ArticulationType::PreAppoggiatura
        || type == ArticulationType::Acciaccatura) {
        return nominalTimestamp + graceNotesDuration;
    }

    return nominalTimestamp;
}

static RenderingContext buildGraceRenderingCtx(const RenderingContext& baseCtx,
                                               const timestamp_t timestamp, const duration_t duration)
{
    RenderingContext result(baseCtx);
    result.nominalTimestamp = timestamp;
    result.nominalDuration = duration;
    result.nominalPositionStartTick = timestampToTick(result.score, timestamp) - result.positionTickOffset;
    result.nominalPositionEndTick = timestampToTick(result.score, timestamp + duration) - result.positionTickOffset;
    result.nominalDurationTicks = result.nominalPositionEndTick - result.nominalPositionStartTick;

    return result;
}

GraceChordCtx GraceChordCtx::buildCtx(const Chord* chord, const mpe::ArticulationType type, const RenderingContext& ctx)
{
    std::vector<Chord*> graceChords;

    const bool isPlacedBeforePrincipal = isGraceNotePlacedBeforePrincipalNote(type);
    if (isPlacedBeforePrincipal) {
        graceChords = chord->graceNotesBefore(true /*filterUnplayable*/);
    } else {
        graceChords = chord->graceNotesAfter(true /*filterUnplayable*/);
    }

    const duration_t availableGraceNotesDuration = graceNotesMaxAvailableDuration(type, ctx, graceChords.size());
    const duration_t accumulatedGraceNotesDuration = graceNotesTotalDuration(graceChords, ctx.beatsPerSecond);
    const duration_t actualGraceNotesDuration = std::min(availableGraceNotesDuration, accumulatedGraceNotesDuration);
    const timestamp_t principalChordTimestamp = principalNotesStartTimestamp(type, actualGraceNotesDuration, ctx.nominalTimestamp);
    const duration_t principalChordDuration = ctx.nominalDuration - actualGraceNotesDuration;

    GraceChordCtx result { buildGraceRenderingCtx(ctx, principalChordTimestamp, principalChordDuration), {} };
    result.principalChordCtx.commonArticulations.erase(type);

    const double graceNotesDurationFactor = double(actualGraceNotesDuration) / accumulatedGraceNotesDuration;
    const timestamp_t graceNotesTimeOffset = isPlacedBeforePrincipal ? 0 : ctx.nominalDuration;

    timestamp_t graceChordTimestamp = graceNotesStartTimestamp(type, actualGraceNotesDuration,
                                                               ctx.nominalTimestamp + graceNotesTimeOffset);

    for (const Chord* graceChord : graceChords) {
        const int durationTicks = graceChord->durationTypeTicks().ticks();
        const duration_t duration = muse::RealRound(
            graceNotesDurationFactor * durationFromTempoAndTicks(ctx.beatsPerSecond.val, durationTicks), 0);

        RenderingContext graceNoteCtx = buildGraceRenderingCtx(ctx, graceChordTimestamp, duration);
        result.graceChordCtxList.emplace_back(std::make_pair(graceChord, graceNoteCtx));

        graceChordTimestamp += duration;
    }

    return result;
}
