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

    void render(const EngravingItem* item, const mpe::dynamic_level_t nominalDynamicLevel,
                const mpe::ArticulationType persistentArticulationApplied, const mpe::ArticulationsProfilePtr profile,
                mpe::PlaybackEventsMap& result) const;

    void render(const EngravingItem* item, const int tickPositionOffset, const mpe::dynamic_level_t nominalDynamicLevel,
                const mpe::ArticulationType persistentArticulationApplied, const mpe::ArticulationsProfilePtr profile,
                mpe::PlaybackEventsMap& result) const;

    void render(const EngravingItem* item, const mpe::timestamp_t actualTimestamp, const mpe::duration_t actualDuration,
                const mpe::dynamic_level_t actualDynamicLevel, const mpe::ArticulationType persistentArticulationApplied,
                const mpe::ArticulationsProfilePtr profile, mpe::PlaybackEventsMap& result) const;

    void renderChordSymbol(const Harmony* chordSymbol, const int ticksPositionOffset, const mpe::ArticulationsProfilePtr profile,
                           mpe::PlaybackEventsMap& result) const;
    void renderChordSymbol(const Harmony* chordSymbol, const mpe::timestamp_t actualTimestamp, const mpe::duration_t actualDuration,
                           const mpe::ArticulationsProfilePtr profile, mpe::PlaybackEventsMap& result) const;

    void renderMetronome(const Score* score, const int measureStartTick, const int measureEndTick, const int ticksPositionOffset,
                         mpe::PlaybackEventsMap& result) const;

    void renderMetronome(const Score* score, const int tick, const mpe::timestamp_t actualTimestamp, mpe::PlaybackEventsMap& result) const;

private:
    void renderNoteEvents(const Chord* chord, const int tickPositionOffset, const mpe::dynamic_level_t nominalDynamicLevel,
                          const mpe::ArticulationType persistentArticulationApplied, const mpe::ArticulationsProfilePtr profile,
                          mpe::PlaybackEventsMap& result) const;

    void renderFixedNoteEvent(const Note* note, const mpe::timestamp_t actualTimestamp, const mpe::duration_t actualDuration,
                              const mpe::dynamic_level_t actualDynamicLevel, const mpe::ArticulationType persistentArticulationApplied,
                              const mpe::ArticulationsProfilePtr profile, mpe::PlaybackEventList& result) const;

    void renderRestEvents(const Rest* rest, const int tickPositionOffset, mpe::PlaybackEventsMap& result) const;

    void renderArticulations(const Chord* chord, const RenderingContext& ctx, mpe::PlaybackEventList& result) const;
};
}

#endif // MU_ENGRAVING_PLAYBACKEVENTSRENDERER_H
