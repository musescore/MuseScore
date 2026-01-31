/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#pragma once

#include <limits>

#include "mpe/events.h"

#include "../dom/chord.h"
#include "../dom/note.h"
#include "../dom/ottava.h"
#include "../dom/staff.h"
#include "../dom/sig.h"
#include "../dom/spannermap.h"

#include "utils/repeatpassutils.h"

#include "utils/arrangementutils.h"
#include "utils/pitchutils.h"
#include "playbackcontext.h"

namespace mu::engraving {
struct RenderingContext {
    RenderingContext() = default;

    muse::mpe::timestamp_t nominalTimestamp = 0;
    muse::mpe::duration_t nominalDuration = 0;
    muse::mpe::dynamic_level_t nominalDynamicLevel = 0;
    int nominalPositionStartTick = 0;
    int nominalPositionEndTick = 0;
    int nominalDurationTicks = 0;
    int positionTickOffset = 0;

    BeatsPerSecond beatsPerSecond = 0;
    TimeSigFrac timeSignatureFraction;

    muse::mpe::ArticulationMap commonArticulations;

    const Score* score = nullptr;
    const muse::mpe::ArticulationsProfilePtr profile;
    const PlaybackContextPtr playbackCtx;

    bool isValid() const
    {
        return score
               && profile
               && playbackCtx
               && beatsPerSecond > 0
               && nominalDuration > 0
               && nominalDurationTicks > 0;
    }
};

inline int ottavaPitchShiftForPass(const Note* note, const RenderingContext& ctx)
{
    if (!note || !ctx.score) {
        return 0;
    }

    const int noteTick = note->tick().ticks();
    const int utick = ctx.nominalPositionStartTick + ctx.positionTickOffset;
    const int pass = repeatPassForUtick(ctx.score, utick);

    const SpannerMap::IntervalList& spanners = ctx.score->spannerMap().findOverlapping(noteTick, noteTick);
    int bestStartTick = std::numeric_limits<int>::min();
    int resultShift = 0;

    for (const auto& spannerInterval : spanners) {
        const Spanner* spanner = spannerInterval.value;
        if (!spanner || !spanner->isOttava() || !spanner->playSpanner()) {
            continue;
        }

        if (spanner->staffIdx() != note->staffIdx()) {
            continue;
        }

        const int startTick = spanner->tick().ticks();
        const int endTick = spanner->tick2().ticks();
        if (noteTick < startTick || noteTick >= endTick) {
            continue;
        }

        if (!shouldApplyOnPass(spanner->playOnPasses(), pass)) {
            continue;
        }

        if (startTick >= bestStartTick) {
            bestStartTick = startTick;
            resultShift = toOttava(spanner)->pitchShift();
        }
    }

    return resultShift;
}

inline RenderingContext buildRenderingCtx(const Chord* chord, const int tickPositionOffset,
                                          const muse::mpe::ArticulationsProfilePtr profile, const PlaybackContextPtr playbackCtx,
                                          const muse::mpe::ArticulationMap& articulations = {})
{
    int chordPosTick = chord->tick().ticks();
    int chordDurationTicks = chord->actualTicks().ticks();
    int chordPosTickWithOffset = chordPosTick + tickPositionOffset;

    const Score* score = chord->score();

    auto chordTnD = timestampAndDurationFromStartAndDurationTicks(score, chordPosTick, chordDurationTicks, tickPositionOffset);

    BeatsPerSecond bps = score->tempomap()->multipliedTempo(chordPosTick);
    TimeSigFrac timeSignatureFraction = score->sigmap()->timesig(chordPosTick).timesig();

    RenderingContext ctx{ chordTnD.timestamp,
                          chordTnD.duration,
                          playbackCtx->appliableDynamicLevel(chord->track(), chordPosTickWithOffset),
                          chordPosTick,
                          chordPosTick + chordDurationTicks,
                          chordDurationTicks,
                          tickPositionOffset,
                          bps,
                          timeSignatureFraction,
                          articulations,
                          score,
                          profile,
                          playbackCtx };

    return ctx;
}

struct NominalNoteCtx {
    voice_idx_t voiceIdx = 0;
    staff_idx_t staffIdx = 0;
    muse::mpe::timestamp_t timestamp = 0;
    muse::mpe::duration_t duration = 0;
    BeatsPerSecond tempo = 0;
    muse::mpe::dynamic_level_t dynamicLevel = 0;
    float userVelocityFraction = 0.f;

    muse::mpe::pitch_level_t pitchLevel = 0;

    RenderingContext chordCtx;
    muse::mpe::ArticulationMap articulations;

    explicit NominalNoteCtx(const Note* note, const RenderingContext& ctx)
        : voiceIdx(note->voice()),
        staffIdx(note->staffIdx()),
        timestamp(ctx.nominalTimestamp),
        duration(ctx.nominalDuration),
        tempo(ctx.beatsPerSecond),
        dynamicLevel(ctx.nominalDynamicLevel),
        userVelocityFraction(note->userVelocityFraction()),
        pitchLevel(notePitchLevel(note->playingTpc(),
                                  note->playingOctave(),
                                  note->playingTuning())),
        chordCtx(ctx),
        articulations(ctx.commonArticulations)
    {
        const Staff* staff = note->staff();
        const int baseOttavaShift = staff ? staff->pitchOffset(note->tick()) : 0;
        const int passOttavaShift = ottavaPitchShiftForPass(note, ctx);
        const int shiftDelta = passOttavaShift - baseOttavaShift;
        if (shiftDelta != 0) {
            pitchLevel += shiftDelta * muse::mpe::PITCH_LEVEL_STEP;
        }
    }
};

inline muse::mpe::NoteEvent buildNoteEvent(const NominalNoteCtx& ctx, const muse::mpe::PitchCurve& pitchCurve = {})
{
    return muse::mpe::NoteEvent(ctx.timestamp,
                                ctx.duration,
                                static_cast<muse::mpe::voice_layer_idx_t>(ctx.voiceIdx),
                                static_cast<muse::mpe::staff_layer_idx_t>(ctx.staffIdx),
                                ctx.pitchLevel,
                                ctx.dynamicLevel,
                                ctx.articulations,
                                ctx.tempo.val,
                                ctx.userVelocityFraction,
                                pitchCurve);
}
}
