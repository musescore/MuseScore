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
#ifndef MUSE_AUDIO_AUDIOENGINE_H
#define MUSE_AUDIO_AUDIOENGINE_H

#include <memory>
#include <atomic>
#include <mutex>

#include "../iaudioengine.h"

#include "global/types/ret.h"

namespace muse::audio::engine {
class AudioBuffer;
class AudioEngine : public IAudioEngine
{
public:
    AudioEngine();
    ~AudioEngine();

    struct RenderConstraints {
        samples_t minSamplesToReserveWhenIdle = 0;
        samples_t minSamplesToReserveInRealtime = 0;

        // mixer
        size_t desiredAudioThreadNumber = 0;
        size_t minTrackCountForMultithreading = 0;
    };

    Ret init(const OutputSpec& outputSpec, const RenderConstraints& consts);
    void deinit();

    void setOutputSpec(const OutputSpec& outputSpec) override;
    OutputSpec outputSpec() const override;
    async::Channel<OutputSpec> outputSpecChanged() const override;

    RenderMode mode() const override;
    void setMode(const RenderMode newMode) override;
    async::Channel<RenderMode> modeChanged() const override;

    void execOperation(OperationType type, const Operation& func) override;

    MixerPtr mixer() const override;

    void processAudioData() override;
    samples_t process(float* buffer, samples_t samplesPerChannel) override;
    void popAudioData(float* dest, size_t sampleCount) override;

private:

    void updateBufferConstraints();
    samples_t fillSilent(float* buffer, samples_t samplesPerChannel);

    std::atomic<bool> m_inited = false;

    OutputSpec m_outputSpec;
    async::Channel<OutputSpec> m_outputSpecChanged;

    std::atomic<RenderMode> m_mode = RenderMode::Undefined;
    async::Channel<RenderMode> m_modeChanged;

    std::atomic<bool> m_processing = false;
    std::atomic<OperationType> m_operationType = OperationType::Undefined;
    std::mutex m_quickOperationWaitMutex;

    MixerPtr m_mixer = nullptr;
    std::shared_ptr<AudioBuffer> m_buffer = nullptr;
    RenderConstraints m_renderConsts;
};
}

#endif // MUSE_AUDIO_AUDIOENGINE_H
