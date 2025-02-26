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
#ifndef MUSE_AUDIO_MIXERCHANNEL_H
#define MUSE_AUDIO_MIXERCHANNEL_H

#include "global/modularity/ioc.h"
#include "global/async/asyncable.h"
#include "global/async/notification.h"

#include "../../ifxresolver.h"
#include "../../ifxprocessor.h"
#include "../dsp/compressor.h"
#include "track.h"

namespace muse::audio {
class MixerChannel : public ITrackAudioOutput, public Injectable, public async::Asyncable
{
    Inject<fx::IFxResolver> fxResolver = { this };

public:
    explicit MixerChannel(const TrackId trackId, IAudioSourcePtr source, const unsigned int sampleRate,
                          const modularity::ContextPtr& iocCtx);
    explicit MixerChannel(const TrackId trackId, const unsigned int sampleRate, unsigned int audioChannelsCount,
                          const modularity::ContextPtr& iocCtx);

    TrackId trackId() const;
    IAudioSourcePtr source() const;

    bool muted() const;
    async::Notification mutedChanged() const;

    bool isSilent() const;

    void notifyNoAudioSignal();

    const AudioOutputParams& outputParams() const override;
    void applyOutputParams(const AudioOutputParams& requiredParams) override;
    async::Channel<AudioOutputParams> outputParamsChanged() const override;

    AudioSignalChanges audioSignalChanges() const override;

    bool isActive() const override;
    void setIsActive(bool arg) override;

    void setSampleRate(unsigned int sampleRate) override;
    unsigned int audioChannelsCount() const override;
    async::Channel<unsigned int> audioChannelsCountChanged() const override;
    samples_t process(float* buffer, samples_t samplesPerChannel) override;

private:
    void completeOutput(float* buffer, unsigned int samplesCount);

    TrackId m_trackId = -1;

    unsigned int m_sampleRate = 0;
    unsigned int m_audioChannelsCount = 0;
    AudioOutputParams m_params;

    IAudioSourcePtr m_audioSource = nullptr;
    std::vector<IFxProcessorPtr> m_fxProcessors = {};

    dsp::CompressorPtr m_compressor = nullptr;

    bool m_isSilent = true;

    async::Notification m_mutedChanged;
    mutable async::Channel<AudioOutputParams> m_paramsChanges;
    mutable AudioSignalsNotifier m_audioSignalNotifier;
};

using MixerChannelPtr = std::shared_ptr<MixerChannel>;
}

#endif // MUSE_AUDIO_MIXERCHANNEL_H
