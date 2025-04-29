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
#ifndef MUSE_AUDIO_MIXER_H
#define MUSE_AUDIO_MIXER_H

#include <memory>
#include <map>

#include "global/modularity/ioc.h"
#include "global/async/asyncable.h"
#include "global/types/retval.h"

#include "../../ifxresolver.h"
#include "../../iaudioconfiguration.h"
#include "../dsp/limiter.h"

#include "abstractaudiosource.h"
#include "mixerchannel.h"
#include "iclock.h"

namespace muse {
class TaskScheduler;
}

namespace muse::audio {
class Mixer : public AbstractAudioSource, public Injectable, public async::Asyncable, public std::enable_shared_from_this<Mixer>
{
    Inject<fx::IFxResolver> fxResolver = { this };
    Inject<IAudioConfiguration> configuration = { this };

public:
    Mixer(const modularity::ContextPtr& iocCtx);
    ~Mixer();

    IAudioSourcePtr mixedSource();

    RetVal<MixerChannelPtr> addChannel(const TrackId trackId, ITrackAudioInputPtr source);
    RetVal<MixerChannelPtr> addAuxChannel(const TrackId trackId);
    Ret removeChannel(const TrackId trackId);

    void setAudioChannelsCount(const audioch_t count);

    void addClock(IClockPtr clock);
    void removeClock(IClockPtr clock);

    AudioOutputParams masterOutputParams() const;
    void setMasterOutputParams(const AudioOutputParams& params);
    void clearMasterOutputParams();
    async::Channel<AudioOutputParams> masterOutputParamsChanged() const;

    AudioSignalChanges masterAudioSignalChanges() const;

    void setIsIdle(bool idle);
    void setTracksToProcessWhenIdle(std::unordered_set<TrackId>&& trackIds);

    // IAudioSource
    void setSampleRate(unsigned int sampleRate) override;
    unsigned int audioChannelsCount() const override;
    samples_t process(float* outBuffer, samples_t samplesPerChannel) override;
    void setIsActive(bool arg) override;

private:
    using TracksData = std::map<TrackId, std::vector<float> >;

    void processTrackChannels(size_t outBufferSize, size_t samplesPerChannel, TracksData& outTracksData);
    void mixOutputFromChannel(float* outBuffer, const float* inBuffer, unsigned int samplesCount) const;
    void prepareAuxBuffers(size_t outBufferSize);
    void writeTrackToAuxBuffers(const float* trackBuffer, const AuxSendsParams& auxSends, samples_t samplesPerChannel);
    void processAuxChannels(float* buffer, samples_t samplesPerChannel);
    void completeOutput(float* buffer, samples_t samplesPerChannel);

    bool useMultithreading() const;

    void notifyNoAudioSignal();

    msecs_t currentTime() const;

    std::unique_ptr<TaskScheduler> m_taskScheduler;

    size_t m_minTrackCountForMultithreading = 0;
    size_t m_nonMutedTrackCount = 0;

    AudioOutputParams m_masterParams;
    async::Channel<AudioOutputParams> m_masterOutputParamsChanged;
    std::vector<IFxProcessorPtr> m_masterFxProcessors = {};

    std::map<TrackId, MixerChannelPtr> m_trackChannels = {};
    std::unordered_set<TrackId> m_tracksToProcessWhenIdle;

    struct AuxChannelInfo {
        MixerChannelPtr channel;
        std::vector<float> buffer;
        bool receivedAudioSignal = false;
    };

    std::vector<AuxChannelInfo> m_auxChannelInfoList;

    dsp::LimiterPtr m_limiter = nullptr;

    std::set<IClockPtr> m_clocks;
    audioch_t m_audioChannelsCount = 0;

    mutable AudioSignalsNotifier m_audioSignalNotifier;

    bool m_isSilence = false;
    bool m_isIdle = false;
};

using MixerPtr = std::shared_ptr<Mixer>;
}

#endif // MUSE_AUDIO_MIXER_H
