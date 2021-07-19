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
#ifndef MU_AUDIO_MIXER_H
#define MU_AUDIO_MIXER_H

#include <memory>
#include <map>

#include "modularity/ioc.h"
#include "async/asyncable.h"

#include "abstractaudiosource.h"
#include "mixerchannel.h"
#include "clock.h"

namespace mu::audio {
class Mixer : public AbstractAudioSource, public std::enable_shared_from_this<Mixer>, public async::Asyncable
{
public:
    Mixer();
    ~Mixer();

    IAudioSourcePtr mixedSource();

    RetVal<IMixerChannelPtr> addChannel(IAudioSourcePtr source, const AudioOutputParams& params,
                                        async::Channel<AudioOutputParams> paramsChanged);
    Ret removeChannel(const MixerChannelId id);

    void setAudioChannelsCount(const audioch_t count);

    void addClock(IClockPtr clock);
    void removeClock(IClockPtr clock);

    AudioOutputParams masterOutputParams() const;
    void setMasterOutputParams(const AudioOutputParams& params);
    async::Channel<AudioOutputParams> masterOutputParamsChanged() const;

    async::Channel<audioch_t, float> masterSignalAmplitudeRmsChanged() const;
    async::Channel<audioch_t, volume_dbfs_t> masterVolumePressureDbfsChanged() const;

    // IAudioSource
    void setSampleRate(unsigned int sampleRate) override;
    unsigned int audioChannelsCount() const override;
    void process(float* outBuffer, unsigned int samplesPerChannel) override;

private:
    void mixOutput(float* outBuffer, float* inBuffer, unsigned int samplesCount);

    std::vector<float> m_writeCacheBuff;

    AudioOutputParams m_masterParams;
    async::Channel<AudioOutputParams> m_masterOutputParamsChanged;
    std::vector<IFxProcessorPtr> m_globalFxProcessors = {};

    std::map<MixerChannelId, MixerChannelPtr> m_mixerChannels = {};

    std::set<IClockPtr> m_clocks;
    audioch_t m_audioChannelsCount = 0;

    async::Channel<audioch_t, float> m_masterSignalAmplitudeRmsChanged;
    async::Channel<audioch_t, volume_dbfs_t> m_masterVolumePressureDbfsChanged;
};

using MixerPtr = std::shared_ptr<Mixer>;
}

#endif // MU_AUDIO_MIXER_H
