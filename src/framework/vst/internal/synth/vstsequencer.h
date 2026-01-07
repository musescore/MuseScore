/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "audio/engine/internal/abstracteventsequencer.h"

#include "vsttypes.h"

namespace muse::vst {
class VstSequencer : public audio::engine::AbstractEventSequencer<VstEvent, ParamChangeEvent, muse::audio::gain_t>
{
public:
    void init(ParamsMapping&& mapping, bool useDynamicEvents);

    muse::audio::gain_t currentGain() const;

private:
    void updateOffStreamEvents(const mpe::PlaybackEventsMap& events, const mpe::DynamicLevelLayers& dynamics) override;
    void updateMainStreamEvents(const mpe::PlaybackEventsMap& events, const mpe::DynamicLevelLayers& dynamics) override;

    using SostenutoTimeAndDurations = std::vector<mpe::TimestampAndDuration>;

    void addPlaybackEvents(EventSequenceMap& destination, const mpe::PlaybackEventsMap& events);
    void addDynamicEvents(EventSequenceMap& destination, const mpe::DynamicLevelLayers& layers);
    void addNoteEvent(EventSequenceMap& destination, const mpe::NoteEvent& noteEvent, SostenutoTimeAndDurations& sostenutoTimeAndDurations);
    void addPedalEvent(EventSequenceMap& destination, const mpe::ArticulationMeta& meta);
    void addControlChangeEvent(EventSequenceMap& destination, const mpe::timestamp_t timestamp, const mpe::ControllerChangeEvent& event);
    void addParamChange(EventSequenceMap& destination, const mpe::timestamp_t timestamp, const ControlIdx controlIdx,
                        const PluginParamValue value);
    void addPitchCurve(EventSequenceMap& destination, const mpe::NoteEvent& noteEvent, const mpe::ArticulationMeta& artMeta);
    void addSostenutoEvents(EventSequenceMap& destination, const SostenutoTimeAndDurations& sostenutoTimeAndDurations);

    VstEvent buildEvent(const Steinberg::Vst::Event::EventTypes type, const int32_t noteIdx, const float velocityFraction,
                        const float tuning) const;

    int32_t noteIndex(const mpe::pitch_level_t pitchLevel) const;
    float noteTuning(const mpe::NoteEvent& noteEvent, const int noteIdx) const;
    float noteVelocityFraction(const mpe::NoteEvent& noteEvent) const;
    float expressionLevel(const mpe::dynamic_level_t dynamicLevel) const;
    float pitchBendLevel(const mpe::pitch_level_t pitchLevel) const;

    bool m_inited = false;
    bool m_useDynamicEvents = false;
    ParamsMapping m_mapping;
};
}
