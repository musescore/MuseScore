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

#ifndef MU_VST_VSTSEQUENCER_H
#define MU_VST_VSTSEQUENCER_H

#include "audio/internal/abstracteventsequencer.h"

#include "vsttypes.h"

typedef typename std::variant<Steinberg::Vst::Event, Steinberg::Vst::ParameterInfo, mu::audio::gain_t> VstSequencerEvent;

template<>
struct std::less<VstSequencerEvent>
{
    bool operator()(const VstSequencerEvent& first,
                    const VstSequencerEvent& second) const
    {
        if (first.index() != second.index()) {
            return first.index() < second.index();
        }

        if (std::holds_alternative<Steinberg::Vst::Event>(first)) {
            return std::less<Steinberg::Vst::Event> {}(std::get<Steinberg::Vst::Event>(first),
                                                       std::get<Steinberg::Vst::Event>(second));
        }

        if (std::holds_alternative<Steinberg::Vst::ParameterInfo>(first)) {
            return std::less<Steinberg::Vst::ParameterInfo> {}(std::get<Steinberg::Vst::ParameterInfo>(first),
                                                               std::get<Steinberg::Vst::ParameterInfo>(second));
        }

        return std::get<mu::audio::gain_t>(first) < std::get<mu::audio::gain_t>(second);
    }
};

namespace mu::vst {
class VstSequencer : public audio::AbstractEventSequencer<VstEvent, PluginParamInfo, audio::gain_t>
{
public:
    void init(ParamsMapping&& mapping);

    void updateOffStreamEvents(const mpe::PlaybackEventsMap& changes) override;
    void updateMainStreamEvents(const mpe::PlaybackEventsMap& changes) override;
    void updateDynamicChanges(const mpe::DynamicLevelMap& changes) override;

    audio::gain_t currentGain() const;

private:
    void updatePlaybackEvents(EventSequenceMap& destination, const mpe::PlaybackEventsMap& changes);

    void appendControlSwitch(EventSequenceMap& destination, const mpe::NoteEvent& noteEvent, const mpe::ArticulationTypeSet& appliableTypes,
                             const ControllIdx controlIdx);
    void appendPitchBend(EventSequenceMap& destination, const mpe::NoteEvent& noteEvent, const mpe::ArticulationTypeSet& appliableTypes);

    VstEvent buildEvent(const Steinberg::Vst::Event::EventTypes type, const int32_t noteIdx, const float velocityFraction,
                        const float tuning) const;
    PluginParamInfo buildParamInfo(const PluginParamId id, const PluginParamValue value) const;

    int32_t noteIndex(const mpe::pitch_level_t pitchLevel) const;
    float noteTuning(const mpe::NoteEvent& noteEvent, const int noteIdx) const;
    float noteVelocityFraction(const mpe::NoteEvent& noteEvent) const;
    float expressionLevel(const mpe::dynamic_level_t dynamicLevel) const;
    float pitchBendLevel(const mpe::pitch_level_t pitchLevel) const;

    ParamsMapping m_mapping;
};
}

#endif // MU_VST_VSTSEQUENCER_H
