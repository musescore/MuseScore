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
#ifndef MUSE_AUDIO_AUDIOENGINE_H
#define MUSE_AUDIO_AUDIOENGINE_H

#include <memory>

#include "../iaudioengine.h"

#include "global/async/notification.h"
#include "global/types/ret.h"

#include "global/modularity/ioc.h"
#include "audio/common/rpc/irpcchannel.h"

namespace muse::audio::worker {
class AudioBuffer;
class AudioEngine : public IAudioEngine
{
    Inject<rpc::IRpcChannel> rpcChannel;

public:
    AudioEngine() = default;
    ~AudioEngine();

    struct RenderConstraints {
        samples_t minSamplesToReserveWhenIdle = 0;
        samples_t minSamplesToReserveInRealtime = 0;

        // mixer
        size_t desiredAudioThreadNumber = 0;
        size_t minTrackCountForMultithreading = 0;
    };

    Ret init(std::shared_ptr<AudioBuffer> bufferPtr, const RenderConstraints& consts);
    void deinit();

    using OnReadBufferChanged = std::function<void (const samples_t, const sample_rate_t)>;
    void setOnReadBufferChanged(const OnReadBufferChanged func);

    sample_rate_t sampleRate() const override;

    void setSampleRate(const sample_rate_t sampleRate) override;
    void setReadBufferSize(const uint16_t readBufferSize) override;
    void setAudioChannelsCount(const audioch_t count) override;

    RenderMode mode() const override;
    void setMode(const RenderMode newMode) override;
    async::Notification modeChanged() const override;

    MixerPtr mixer() const override;

private:

    void updateBufferConstraints();

    bool m_inited = false;

    sample_rate_t m_sampleRate = 0;
    samples_t m_readBufferSize = 0;

    MixerPtr m_mixer = nullptr;
    std::shared_ptr<AudioBuffer> m_buffer = nullptr;
    RenderConstraints m_renderConsts;

    RenderMode m_currentMode = RenderMode::Undefined;
    async::Notification m_modeChanges;

    OnReadBufferChanged m_onReadBufferChanged;
};
}

#endif // MUSE_AUDIO_AUDIOENGINE_H
