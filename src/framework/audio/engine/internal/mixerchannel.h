/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "global/modularity/ioc.h"
#include "global/async/asyncable.h"
#include "global/async/notification.h"

#include "iaudiofactory.h"
#include "../ifxprocessor.h"
#include "audiosignalnotifier.h"
#include "track.h"
#include "../iplayhead.h"

namespace muse::audio::engine {
class MixerChannel : public ITrackAudioOutput, public async::Asyncable
{
    GlobalInject<IAudioFactory> audioFactory;

public:
    explicit MixerChannel(const TrackId trackId, const OutputSpec& outputSpec, IAudioSourcePtr source,
                          PlayheadPositionPtr playheadPosition);
    explicit MixerChannel(const TrackId trackId, const OutputSpec& outputSpec, PlayheadPositionPtr playheadPosition);

    void setPlayheadPosition(PlayheadPositionPtr playheadPosition);

    TrackId trackId() const;
    IAudioSourcePtr source() const;

    bool muted() const;
    async::Notification mutedChanged() const;

    bool isSilent() const;
    bool shouldProcessDuringSilence() const;
    async::Channel<bool> shouldProcessDuringSilenceChanged() const;

    AudioSignalsNotifier& signalNotifier() const;
    void setNoAudioSignal();

    const AudioOutputParams& outputParams() const override;
    void applyOutputParams(const AudioOutputParams& requiredParams) override;
    async::Channel<AudioOutputParams> outputParamsChanged() const override;

    AudioSignalChanges audioSignalChanges() const override;

    RenderMode mode() const override;
    void setMode(const RenderMode mode) override;

    void setOutputSpec(const OutputSpec& spec) override;
    unsigned int audioChannelsCount() const override;
    async::Channel<unsigned int> audioChannelsCountChanged() const override;
    samples_t process(float* buffer, samples_t samplesPerChannel) override;

private:
    void completeOutput(float* buffer, unsigned int samplesCount);

    void updateShouldProcessDuringSilence();

    TrackId m_trackId = -1;

    RenderMode m_mode = RenderMode::Undefined;
    OutputSpec m_outputSpec;
    AudioOutputParams m_params;

    IAudioSourcePtr m_audioSource = nullptr;
    PlayheadPositionPtr m_playheadPosition = nullptr;
    std::vector<IFxProcessorPtr> m_fxProcessors = {};

    bool m_isSilent = true;
    bool m_shouldProcessDuringSilence = false;
    async::Channel<bool> m_shouldProcessDuringSilenceChanged;

    async::Notification m_mutedChanged;
    mutable async::Channel<AudioOutputParams> m_paramsChanges;
    mutable AudioSignalsNotifier m_audioSignalNotifier;
};

using MixerChannelPtr = std::shared_ptr<MixerChannel>;
}
