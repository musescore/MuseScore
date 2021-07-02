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

#include "async/asyncable.h"

#include "iaudiosource.h"
#include "ifxprocessor.h"
#include "imixerchannel.h"

namespace mu::audio {
class MixerChannel : public IMixerChannel, public IAudioSource, public async::Asyncable
{
public:
    explicit MixerChannel(const MixerChannelId id, IAudioSourcePtr source, AudioOutputParams params,
                          async::Channel<AudioOutputParams> paramsChanged, const unsigned int sampleRate);

    MixerChannelId id() const override;

    async::Channel<audioch_t, float> signalAmplitudeRmsChanged() const override;
    async::Channel<audioch_t, volume_dbfs_t> volumePressureDbfsChanged() const override;

    bool isActive() const override;
    void setIsActive(bool arg) override;

    void setSampleRate(unsigned int sampleRate) override;
    unsigned int audioChannelsCount() const override;
    async::Channel<unsigned int> audioChannelsCountChanged() const override;
    void process(float* buffer, unsigned int sampleCount) override;

private:
    void setOutputParams(const AudioOutputParams& params);
    void completeOutput(float* buffer, unsigned int samplesCount) const;

    MixerChannelId m_id = -1;

    AudioOutputParams m_params;

    IAudioSourcePtr m_audioSource = nullptr;
    std::vector<IFxProcessorPtr> m_fxProcessors = {};

    mutable async::Channel<audioch_t, float> m_signalAmplitudeRmsChanged;
    mutable async::Channel<audioch_t, volume_dbfs_t> m_volumePressureDbfsChanged;
};

using MixerChannelPtr = std::shared_ptr<MixerChannel>;
}

#endif // MU_AUDIO_MIXERCHANNEL_H
