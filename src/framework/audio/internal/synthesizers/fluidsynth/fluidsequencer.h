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

#ifndef MUSE_AUDIO_FLUIDSEQUENCER_H
#define MUSE_AUDIO_FLUIDSEQUENCER_H

#include "global/async/channel.h"
#include "midi/midievent.h"
#include "mpe/events.h"

#include "../../abstracteventsequencer.h"
#include "soundmapping.h"

namespace muse::audio {
class FluidSequencer : public AbstractEventSequencer<midi::Event>
{
public:
    void init(const mpe::PlaybackSetupData& setupData, const std::optional<midi::Program>& programOverride, bool useDynamicEvents);

    int currentExpressionLevel() const;
    int naturalExpressionLevel() const;

    async::Channel<midi::channel_t, midi::Program> channelAdded() const;

    const ChannelMap& channels() const;
    int lastStaff() const;

private:
    void updateOffStreamEvents(const mpe::PlaybackEventsMap& events, const mpe::DynamicLevelLayers& dynamics,
                               const mpe::PlaybackParamList& params) override;
    void updateMainStreamEvents(const mpe::PlaybackEventsMap& events, const mpe::DynamicLevelLayers& dynamics,
                                const mpe::PlaybackParamLayers& params) override;

    using SostenutoTimeAndDurations = std::map<midi::channel_t, std::vector<mpe::TimestampAndDuration> >;

    void addPlaybackEvents(EventSequenceMap& destination, const mpe::PlaybackEventsMap& events);
    void addDynamicEvents(EventSequenceMap& destination, const mpe::DynamicLevelLayers& dynamics);
    void addNoteEvent(EventSequenceMap& destination, const mpe::NoteEvent& noteEvent, SostenutoTimeAndDurations& sostenutoTimeAndDurations);
    void addControlChangeEvent(EventSequenceMap& destination, const mpe::timestamp_t timestamp, const mpe::ControllerChangeEvent& event);
    void addControlChange(EventSequenceMap& destination, const mpe::timestamp_t timestamp, const int midiControlIdx,
                          const midi::channel_t channelIdx, const uint32_t value);
    void addPitchCurve(EventSequenceMap& destination, const mpe::NoteEvent& noteEvent, const mpe::ArticulationMeta& artMeta,
                       const midi::channel_t channelIdx);
    void addPitchBend(EventSequenceMap& destination, const mpe::timestamp_t timestamp, const midi::channel_t channelIdx,
                      const uint32_t value);
    void addSostenutoEvents(EventSequenceMap& destination, const SostenutoTimeAndDurations& sostenutoTimeAndDurations);

    midi::channel_t channel(const mpe::NoteEvent& noteEvent) const;
    midi::note_idx_t noteIndex(const mpe::pitch_level_t pitchLevel) const;
    midi::tuning_t noteTuning(const mpe::NoteEvent& noteEvent, const int noteIdx) const;
    midi::velocity_t noteVelocity(const mpe::NoteEvent& noteEvent) const;
    int expressionLevel(const mpe::dynamic_level_t dynamicLevel) const;
    int pitchBendLevel(const mpe::pitch_level_t pitchLevel) const;

    mutable ChannelMap m_channels;
    bool m_useDynamicEvents = false;
    int m_lastStaff = -1;
};
}

#endif // MUSE_AUDIO_FLUIDSEQUENCER_H
