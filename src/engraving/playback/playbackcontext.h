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

#ifndef MU_ENGRAVING_PLAYBACKCONTEXT_H
#define MU_ENGRAVING_PLAYBACKCONTEXT_H

#include "mpe/events.h"

#include "libmscore/note.h"

#include "playback/utils/arrangementutils.h"
#include "playback/utils/pitchutils.h"

namespace mu::engraving {
struct PlaybackContext {
    mpe::timestamp_t nominalTimestamp = 0;
    mpe::duration_t nominalDuration = 0;
    mpe::dynamic_level_t nominalDynamicLevel = 0;
    int nominalPositionTick = 0;
    int nominalDurationTicks = 0;
    BeatsPerSecond beatsPerSecond = 0;
    mpe::ArticulationMap commonArticulations;
    mpe::ArticulationsProfilePtr profile;

    PlaybackContext() = default;

    explicit PlaybackContext(const mpe::timestamp_t timestamp,
                             const mpe::duration_t duration,
                             const mpe::dynamic_level_t dynamicLevel,
                             const int posTick,
                             const int durationTicks,
                             const BeatsPerSecond& bps,
                             const mpe::ArticulationMap& articulations,
                             const mpe::ArticulationsProfilePtr profilePtr)
        : nominalTimestamp(timestamp),
        nominalDuration(duration),
        nominalDynamicLevel(dynamicLevel),
        nominalPositionTick(posTick),
        nominalDurationTicks(durationTicks),
        beatsPerSecond(bps),
        commonArticulations(articulations),
        profile(profilePtr)
    {}

    bool isValid() const
    {
        return profile
               && beatsPerSecond > 0
               && nominalDuration > 0
               && nominalDurationTicks > 0;
    }
};

struct NominalNoteCtx {
    int voiceIdx = 0;
    mpe::timestamp_t timestamp = 0;
    mpe::duration_t duration = 0;

    mpe::pitch_level_t pitchLevel = 0;

    PlaybackContext chordCtx;

    explicit NominalNoteCtx(const Ms::Note* note, const PlaybackContext& ctx)
        : voiceIdx(note->voice()),
        timestamp(ctx.nominalTimestamp),
        duration(ctx.nominalDuration),
        pitchLevel(notePitchLevel(note->tpc(), note->octave())),
        chordCtx(ctx)
    {}
};

inline mpe::NoteEvent buildNoteEvent(NominalNoteCtx&& ctx)
{
    return mpe::NoteEvent(ctx.timestamp,
                          ctx.duration,
                          ctx.voiceIdx,
                          ctx.pitchLevel,
                          ctx.chordCtx.nominalDynamicLevel,
                          ctx.chordCtx.commonArticulations);
}

inline mpe::NoteEvent buildNoteEvent(const Ms::Note* note, const PlaybackContext& ctx)
{
    return mpe::NoteEvent(ctx.nominalTimestamp,
                          ctx.nominalDuration,
                          note->voice(),
                          notePitchLevel(note->tpc(), note->octave()),
                          ctx.nominalDynamicLevel,
                          ctx.commonArticulations);
}

inline mpe::NoteEvent buildNoteEvent(NominalNoteCtx&& ctx, const mpe::duration_t eventDuration,
                                     const mpe::timestamp_t timestampOffset,
                                     const mpe::pitch_level_t pitchLevelOffset)
{
    return mpe::NoteEvent(ctx.timestamp + timestampOffset,
                          eventDuration,
                          ctx.voiceIdx,
                          ctx.pitchLevel + pitchLevelOffset,
                          ctx.chordCtx.nominalDynamicLevel,
                          ctx.chordCtx.commonArticulations);
}
}

#endif // MU_ENGRAVING_PLAYBACKCONTEXT_H
