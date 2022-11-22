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
#include "musesamplersequencer.h"

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

    void revokePlayingNotes() override;

protected:
    void setupSound(const mpe::PlaybackSetupData& setupData) override;
    void setupEvents(const mpe::PlaybackData& playbackData) override;
    void updateRenderingMode(const audio::RenderMode mode) override;

    audio::msecs_t playbackPosition() const override;
    void setPlaybackPosition(const audio::msecs_t newPosition) override;
    bool isActive() const override;
    void setIsActive(bool arg) override;

    void handleAuditionEvents(const MuseSamplerSequencer::EventType& event);
    void setCurrentPosition(const audio::samples_t samples);
    void extractOutputSamples(audio::samples_t samples, float* output);

    async::Channel<unsigned int> m_audioChannelsCountChanged;

    MuseSamplerLibHandlerPtr m_samplerLib = nullptr;
    ms_MuseSampler m_sampler = nullptr;
    ms_Track m_track = nullptr;
    ms_OutputBuffer m_bus;

    audio::samples_t m_currentPosition = 0;

    std::vector<float> m_leftChannel;
    std::vector<float> m_rightChannel;

    std::array<float*, 2> m_internalBuffer;

    MuseSamplerSequencer m_sequencer;
};

using MuseSamplerWrapperPtr = std::shared_ptr<MuseSamplerWrapper>;
}

#endif // MU_MUSESAMPLER_MUSESAMPLERWRAPPER_H
