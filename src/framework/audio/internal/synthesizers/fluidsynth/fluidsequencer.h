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
#include "midi/miditypes.h"
#include "mpe/events.h"

#include "../../abstracteventsequencer.h"
#include "soundmapping.h"

typedef typename std::variant<mu::midi::Event, mu::midi::DynamicEvent> FluidSequencerEvent;

template<>
struct std::less<FluidSequencerEvent>
{
    bool operator()(const FluidSequencerEvent& first,
                    const FluidSequencerEvent& second) const
    {
        if (first.index() != second.index()) {
            return first.index() < second.index();
        }

        if (std::holds_alternative<mu::midi::Event>(first)) {
            return std::less<mu::midi::Event> {}(std::get<mu::midi::Event>(first),
                                                 std::get<mu::midi::Event>(second));
        }

        if (std::holds_alternative<mu::midi::DynamicEvent>(first)) {
            return std::less<mu::midi::DynamicEvent> {}(std::get<mu::midi::DynamicEvent>(first),
                                                        std::get<mu::midi::DynamicEvent>(second));
        }

        return false;
    }
};

namespace mu::audio {
class FluidSequencer : public AbstractEventSequencer<mu::midi::Event, mu::midi::DynamicEvent>
{
public:
    void init(const mpe::PlaybackSetupData& setupData, const std::optional<midi::Program>& programOverride, bool useDynamicEvents);

    void updateOffStreamEvents(const mpe::PlaybackEventsMap& events) override;
    void updateMainStreamEvents(const mpe::PlaybackEventsMap& evetns, const mpe::DynamicLevelLayers& dynamics) override;

    async::Channel<midi::channel_t, midi::Program> channelAdded() const;

    const ChannelMap& channels() const;

private:
    void updatePlaybackEvents(EventSequenceMap& destination, const mpe::PlaybackEventsMap& events, bool useNoteVelocity = true);
    void updateDynamicEvents(EventSequenceMap& destination, const mpe::PlaybackEventsMap& events, const mpe::DynamicLevelLayers& dynamics);

    void appendControlSwitch(EventSequenceMap& destination, const mpe::NoteEvent& noteEvent, const mpe::ArticulationTypeSet& appliableTypes,
                             const int midiControlIdx);

    void appendPitchBend(EventSequenceMap& destination, const mpe::NoteEvent& noteEvent, const mpe::ArticulationTypeSet& appliableTypes,
                         const midi::channel_t channelIdx);

    void appendDynamicEvents(EventSequenceMap& destination, const mpe::NoteEvent& noteEvent, const mpe::DynamicLevelLayers& dynamicLayers);

    midi::channel_t channel(const mpe::NoteEvent& noteEvent) const;
    midi::note_idx_t noteIndex(const mpe::pitch_level_t pitchLevel) const;
    midi::tuning_t noteTuning(const mpe::NoteEvent& noteEvent, const int noteIdx) const;
    midi::velocity_t noteVelocity(const mpe::NoteEvent& noteEvent) const;
    int dynamicLevelToMidiVelocity(const mpe::dynamic_level_t dynamicLevel) const;
    int pitchBendLevel(const mpe::pitch_level_t pitchLevel) const;

    mutable ChannelMap m_channels;
    bool m_useDynamicEvents = false;
};
}

#endif // MU_AUDIO_FLUIDSEQUENCER_H
