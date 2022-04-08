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

#ifndef MU_MUSESAMPLER_MUSESAMPLERWRAPPER_H
#define MU_MUSESAMPLER_MUSESAMPLERWRAPPER_H

#include <memory>

#include "audio/abstractsynthesizer.h"
#include "async/channel.h"

#include "libhandler.h"

namespace mu::musesampler {
class MuseSamplerWrapper : public audio::synth::AbstractSynthesizer
{
public:
    MuseSamplerWrapper(MuseSamplerLibHandlerPtr samplerLib, const audio::AudioSourceParams& params);
    ~MuseSamplerWrapper() override;

    void setSampleRate(unsigned int sampleRate) override;
    unsigned int audioChannelsCount() const override;
    async::Channel<unsigned int> audioChannelsCountChanged() const override;
    audio::samples_t process(float* buffer, audio::samples_t samplesPerChannel) override;

    std::string name() const override;
    audio::AudioSourceType type() const override;
    void flushSound() override;
    bool isValid() const override;

protected:
    void setupSound(const mpe::PlaybackSetupData& setupData) override;

    void loadMainStreamEvents(const mpe::PlaybackEventsMap& events) override;
    void loadOffStreamEvents(const mpe::PlaybackEventsMap& events) override;
    void loadDynamicLevelChanges(const mpe::DynamicLevelMap& dynamicLevels) override;

    void extractOutputSamples(audio::samples_t samples, float* output);
    void addNoteEvent(const mpe::NoteEvent& noteEvent);
    int pitchIndex(const mpe::pitch_level_t pitchLevel) const;

    ms_NoteArticulation noteArticulationTypes(const mpe::NoteEvent& noteEvent) const;

    async::Channel<unsigned int> m_audioChannelsCountChanged;

    MuseSamplerLibHandlerPtr m_samplerLib = nullptr;
    ms_MuseSampler m_sampler = nullptr;
    ms_Track m_track = nullptr;
    ms_OutputBuffer m_bus;
};

using MuseSamplerWrapperPtr = std::shared_ptr<MuseSamplerWrapper>;
}

#endif // MU_MUSESAMPLER_MUSESAMPLERWRAPPER_H
