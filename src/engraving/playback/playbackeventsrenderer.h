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

#ifndef MU_ENGRAVING_PLAYBACKEVENTSRENDERER_H
#define MU_ENGRAVING_PLAYBACKEVENTSRENDERER_H

#include "mpe/events.h"

#include "renderingcontext.h"

namespace mu::engraving {
class Chord;
class Rest;
class Score;
class Note;

class PlaybackEventsRenderer
{
public:
    PlaybackEventsRenderer() = default;

    void render(const EngravingItem* item, const int tickPositionOffset, const muse::mpe::ArticulationsProfilePtr profile,
                const PlaybackContextPtr playbackCtx, muse::mpe::PlaybackEventsMap& result) const;

    void render(const EngravingItem* item, const muse::mpe::timestamp_t actualTimestamp, const muse::mpe::duration_t actualDuration,
                const muse::mpe::dynamic_level_t actualDynamicLevel, const muse::mpe::ArticulationType persistentArticulationApplied,
                const muse::mpe::ArticulationsProfilePtr profile, muse::mpe::PlaybackEventsMap& result) const;

    void renderChordSymbol(const Harmony* chordSymbol, const int ticksPositionOffset, const muse::mpe::ArticulationsProfilePtr profile,
                           const PlaybackContextPtr playbackCtx, muse::mpe::PlaybackEventsMap& result) const;
    void renderChordSymbol(const Harmony* chordSymbol, const muse::mpe::timestamp_t actualTimestamp,
                           const muse::mpe::duration_t actualDuration, const muse::mpe::dynamic_level_t actualDynamicLevel,
                           const muse::mpe::ArticulationsProfilePtr profile, muse::mpe::PlaybackEventsMap& result) const;

    void renderMetronome(const Score* score, const int measureStartTick, const int measureEndTick, const int ticksPositionOffset,
                         const muse::mpe::ArticulationsProfilePtr profile, muse::mpe::PlaybackEventsMap& result) const;

    void renderMetronome(const Score* score, const int tick, const muse::mpe::timestamp_t actualTimestamp,
                         const muse::mpe::ArticulationsProfilePtr profile, muse::mpe::PlaybackEventsMap& result) const;

    void renderCountIn(const Score* score, const int tick, const muse::mpe::timestamp_t actualTimestamp,
                       const muse::mpe::ArticulationsProfilePtr profile, muse::mpe::PlaybackEventsMap& result,
                       muse::mpe::duration_t& totalCountInDuration) const;

private:
    void renderNoteEvents(const Chord* chord, const int tickPositionOffset, const muse::mpe::ArticulationsProfilePtr profile,
                          const PlaybackContextPtr playbackCtx, muse::mpe::PlaybackEventsMap& result) const;

    void renderFixedNoteEvent(const Note* note, const muse::mpe::timestamp_t actualTimestamp, const muse::mpe::duration_t actualDuration,
                              const muse::mpe::dynamic_level_t actualDynamicLevel,
                              const muse::mpe::ArticulationType persistentArticulationApplied,
                              const muse::mpe::ArticulationsProfilePtr profile, muse::mpe::PlaybackEventList& result) const;

    void renderRestEvents(const Rest* rest, const int tickPositionOffset, muse::mpe::PlaybackEventsMap& result) const;
};
}

#endif // MU_ENGRAVING_PLAYBACKEVENTSRENDERER_H
