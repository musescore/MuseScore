/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "mpe/events.h"

#include "../dom/chord.h"
#include "../dom/note.h"
#include "../dom/sig.h"
#include "../dom/tie.h"

#include "playback/utils/arrangementutils.h"
#include "playback/utils/pitchutils.h"
#include "playback/playbackcontext.h"

namespace mu::engraving {
struct RenderingContext {
    muse::mpe::timestamp_t nominalTimestamp = 0;
    muse::mpe::duration_t nominalDuration = 0;
    muse::mpe::dynamic_level_t nominalDynamicLevel = 0;
    int nominalPositionStartTick = 0;
    int nominalPositionEndTick = 0;
    int nominalDurationTicks = 0;
    int positionTickOffset = 0;

    BeatsPerSecond beatsPerSecond = 0;
    TimeSigFrac timeSignatureFraction;

    muse::mpe::ArticulationType persistentArticulation = muse::mpe::ArticulationType::Undefined;
    muse::mpe::ArticulationMap commonArticulations;

    const Score* score = nullptr;
    const muse::mpe::ArticulationsProfilePtr profile;
    const PlaybackContextPtr playbackCtx;

    RenderingContext() = default;

    explicit RenderingContext(const muse::mpe::timestamp_t timestamp,
                              const muse::mpe::duration_t duration,
                              const muse::mpe::dynamic_level_t dynamicLevel,
                              const int posTick,
                              const int posTickOffset,
                              const int durationTicks,
                              const BeatsPerSecond& bps,
                              const TimeSigFrac& timeSig,
                              const muse::mpe::ArticulationType persistentArticulationType,
                              const muse::mpe::ArticulationMap& articulations,
                              const Score* scorePtr,
                              const muse::mpe::ArticulationsProfilePtr profilePtr,
                              const PlaybackContextPtr playbackCtxPtr)
        : nominalTimestamp(timestamp),
        nominalDuration(duration),
        nominalDynamicLevel(dynamicLevel),
        nominalPositionStartTick(posTick),
        nominalPositionEndTick(posTick + durationTicks),
        nominalDurationTicks(durationTicks),
        positionTickOffset(posTickOffset),
        beatsPerSecond(bps),
        timeSignatureFraction(timeSig),
        persistentArticulation(persistentArticulationType),
        commonArticulations(articulations),
        score(scorePtr),
        profile(profilePtr),
        playbackCtx(playbackCtxPtr)
    {}

    bool isValid() const
    {
        return profile
               && playbackCtx
               && beatsPerSecond > 0
               && nominalDuration > 0
               && nominalDurationTicks > 0;
    }
};

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

    RenderingContext ctx(chordTnD.timestamp,
                         chordTnD.duration,
                         playbackCtx->appliableDynamicLevel(chord->track(), chordPosTickWithOffset),
                         chordPosTick,
                         tickPositionOffset,
                         chordDurationTicks,
                         bps,
                         timeSignatureFraction,
                         playbackCtx->persistentArticulationType(chordPosTickWithOffset),
                         articulations,
                         score,
                         profile,
                         playbackCtx);

    return ctx;
}

inline muse::mpe::duration_t noteNominalDuration(const Note* note, const RenderingContext& ctx)
{
    if (!note->score()) {
        return durationFromTempoAndTicks(ctx.beatsPerSecond.val, note->chord()->actualTicks().ticks());
    }

    return durationFromStartAndTicks(note->score(), note->tick().ticks(), note->chord()->actualTicks().ticks(), 0);
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
    }
};

inline bool isNotePlayable(const Note* note, const muse::mpe::ArticulationMap& articualtionMap)
{
    if (!note->play()) {
        return false;
    }

    const Tie* tie = note->tieBack();

    if (tie && tie->playSpanner()) {
        if (!tie->startNote() || !tie->endNote()) {
            return false;
        }

        //!Note Checking whether the tied note has any multi-note articulation attached
        //!     If so, we can't ignore such note
        for (const auto& pair : articualtionMap) {
            if (muse::mpe::isMultiNoteArticulation(pair.first) && !muse::mpe::isRangedArticulation(pair.first)) {
                return true;
            }
        }

        const Chord* firstChord = tie->startNote()->chord();
        const Chord* lastChord = tie->endNote()->chord();
        if (!firstChord || !lastChord) {
            return false;
        }

        if (firstChord->tremoloType() != TremoloType::INVALID_TREMOLO
            || lastChord->tremoloType() != TremoloType::INVALID_TREMOLO) {
            return true;
        }

        auto intervals = firstChord->score()->spannerMap().findOverlapping(firstChord->tick().ticks(),
                                                                           firstChord->endTick().ticks(),
                                                                           /*excludeCollisions*/ true);
        for (auto interval : intervals) {
            const Spanner* sp = interval.value;
            if (sp->isTrill() && sp->playSpanner() && sp->endElement() == firstChord) {
                return true;
            }
        }

        return false;
    }

    return true;
}

inline muse::mpe::NoteEvent buildNoteEvent(const NominalNoteCtx& ctx)
{
    return muse::mpe::NoteEvent(ctx.timestamp,
                                ctx.duration,
                                static_cast<muse::mpe::voice_layer_idx_t>(ctx.voiceIdx),
                                static_cast<muse::mpe::staff_layer_idx_t>(ctx.staffIdx),
                                ctx.pitchLevel,
                                ctx.dynamicLevel,
                                ctx.articulations,
                                ctx.tempo.val,
                                ctx.userVelocityFraction);
}

inline muse::mpe::NoteEvent buildNoteEvent(NominalNoteCtx&& ctx)
{
    return muse::mpe::NoteEvent(ctx.timestamp,
                                ctx.duration,
                                static_cast<muse::mpe::voice_layer_idx_t>(ctx.voiceIdx),
                                static_cast<muse::mpe::staff_layer_idx_t>(ctx.staffIdx),
                                ctx.pitchLevel,
                                ctx.dynamicLevel,
                                ctx.articulations,
                                ctx.tempo.val,
                                ctx.userVelocityFraction);
}

inline muse::mpe::NoteEvent buildNoteEvent(NominalNoteCtx&& ctx, const muse::mpe::PitchCurve& pitchCurve)
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

inline muse::mpe::NoteEvent buildNoteEvent(const Note* note, const RenderingContext& ctx)
{
    return muse::mpe::NoteEvent(ctx.nominalTimestamp,
                                noteNominalDuration(note, ctx),
                                static_cast<muse::mpe::voice_layer_idx_t>(note->voice()),
                                static_cast<muse::mpe::staff_layer_idx_t>(note->staffIdx()),
                                notePitchLevel(note->playingTpc(), note->playingOctave(), note->playingTuning()),
                                ctx.nominalDynamicLevel,
                                ctx.commonArticulations,
                                ctx.beatsPerSecond.val,
                                note->userVelocityFraction());
}

inline muse::mpe::NoteEvent buildNoteEvent(NominalNoteCtx&& ctx, const muse::mpe::duration_t eventDuration,
                                           const muse::mpe::timestamp_t timestampOffset,
                                           const muse::mpe::pitch_level_t pitchLevelOffset)
{
    return muse::mpe::NoteEvent(ctx.timestamp + timestampOffset,
                                eventDuration,
                                static_cast<muse::mpe::voice_layer_idx_t>(ctx.voiceIdx),
                                static_cast<muse::mpe::staff_layer_idx_t>(ctx.staffIdx),
                                ctx.pitchLevel + pitchLevelOffset,
                                ctx.dynamicLevel,
                                ctx.articulations,
                                ctx.tempo.val,
                                ctx.userVelocityFraction);
}
}
