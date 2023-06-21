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
#ifndef MU_AUDIO_MIXERCHANNEL_H
#define MU_AUDIO_MIXERCHANNEL_H

#include "modularity/ioc.h"

#include "async/asyncable.h"

#include "ifxresolver.h"
#include "ifxprocessor.h"
#include "track.h"
#include "internal/dsp/compressor.h"

namespace mu::audio {
class MixerChannel : public ITrackAudioOutput, public async::Asyncable
{
    INJECT(fx::IFxResolver, fxResolver)

public:
    explicit MixerChannel(const TrackId trackId, IAudioSourcePtr source, const unsigned int sampleRate);
    explicit MixerChannel(const TrackId trackId, const unsigned int sampleRate, unsigned int audioChannelsCount);

    TrackId trackId() const;

    const AudioOutputParams& outputParams() const override;
    void applyOutputParams(const AudioOutputParams& requiredParams) override;
    async::Channel<AudioOutputParams> outputParamsChanged() const override;

    async::Channel<audioch_t, AudioSignalVal> audioSignalChanges() const override;

    bool isActive() const override;
    void setIsActive(bool arg) override;

    void setSampleRate(unsigned int sampleRate) override;
    unsigned int audioChannelsCount() const override;
    bool setAudioChannelsCount(unsigned int channels) override;
    samples_t process(float* buffer, size_t bufferSize, samples_t samplesPerChannel) override;

private:
    void completeOutput(float* buffer, size_t bufferSize, unsigned int samplesCount) const;
    void notifyAboutAudioSignalChanges(const audioch_t audioChannelNumber, const float linearRms) const;

    TrackId m_trackId = -1;

    unsigned int m_sampleRate = 0;
    unsigned int m_audioChannelsCount = 0;
    AudioOutputParams m_params;

    IAudioSourcePtr m_audioSource = nullptr;
    std::vector<IFxProcessorPtr> m_fxProcessors = {};

    dsp::CompressorPtr m_compressor = nullptr;

    mutable async::Channel<AudioOutputParams> m_paramsChanges;
    mutable AudioSignalsNotifier m_audioSignalNotifier;
};

using MixerChannelPtr = std::shared_ptr<MixerChannel>;
}

#endif // MU_AUDIO_MIXERCHANNEL_H
