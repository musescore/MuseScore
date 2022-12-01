/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#ifndef MU_AUDIO_FLUIDSEQUENCER_H
#define MU_AUDIO_FLUIDSEQUENCER_H

#include "async/channel.h"
#include "midi/midievent.h"
#include "mpe/events.h"

#include "abstracteventsequencer.h"
#include "soundmapping.h"

namespace mu::audio {
class FluidSequencer : public AbstractEventSequencer<midi::Event>
{
public:
    void init(const mpe::PlaybackSetupData& setupData);

    int currentExpressionLevel() const;

    void updateOffStreamEvents(const mpe::PlaybackEventsMap& changes) override;
    void updateMainStreamEvents(const mpe::PlaybackEventsMap& changes) override;
    void updateDynamicChanges(const mpe::DynamicLevelMap& changes) override;

    async::Channel<midi::channel_t, midi::Program> channelAdded() const;

    const ChannelMap& channels() const;

private:
    void updatePlaybackEvents(EventSequenceMap& destination, const mpe::PlaybackEventsMap& changes);

    void appendControlSwitch(EventSequenceMap& destination, const mpe::NoteEvent& noteEvent, const mpe::ArticulationTypeSet& appliableTypes,
                             const int midiControlIdx);

    void appendPitchBend(EventSequenceMap& destination, const mpe::NoteEvent& noteEvent, const mpe::ArticulationTypeSet& appliableTypes,
                         const midi::channel_t channelIdx);

    midi::channel_t channel(const mpe::NoteEvent& noteEvent) const;
    midi::note_idx_t noteIndex(const mpe::pitch_level_t pitchLevel) const;
    midi::tuning_t noteTuning(const mpe::NoteEvent& noteEvent, const int noteIdx) const;
    midi::velocity_t noteVelocity(const mpe::NoteEvent& noteEvent) const;
    int expressionLevel(const mpe::dynamic_level_t dynamicLevel) const;
    int pitchBendLevel(const mpe::pitch_level_t pitchLevel) const;

    mutable ChannelMap m_channels;
};
}

#endif // MU_AUDIO_FLUIDSEQUENCER_H
