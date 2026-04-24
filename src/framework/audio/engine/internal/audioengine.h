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

#include "iaudioengine.h"

#include "global/types/ret.h"

namespace muse::audio::engine {
class AudioContext;
class AudioEngine : public IAudioEngine
{
public:
    AudioEngine();
    ~AudioEngine();

    Ret init(const OutputSpec& outputSpec) override;
    void deinit() override;

    RetVal<std::shared_ptr<IAudioContext> > addAudioContext(const AudioCtxId& ctxId) override;
    std::shared_ptr<IAudioContext> context(const AudioCtxId& ctxId) const override;
    void destroyContext(const AudioCtxId& ctxId) override;

    void setOutputSpec(const OutputSpec& outputSpec) override;
    OutputSpec outputSpec() const override;
    async::Channel<OutputSpec> outputSpecChanged() const override;

    void execOperation(OperationType type, const Operation& func) override;
    OperationType operation() const override;

    samples_t process(float* buffer, samples_t samplesPerChannel) override;

private:

    samples_t fillSilent(float* buffer, samples_t samplesPerChannel);

    std::atomic<bool> m_inited = false;

    std::map<AudioCtxId, std::shared_ptr<AudioContext> > m_contexts;

    OutputSpec m_outputSpec;
    async::Channel<OutputSpec> m_outputSpecChanged;

    std::atomic<bool> m_processing = false;
    std::atomic<OperationType> m_operationType = OperationType::Undefined;
    std::mutex m_quickOperationWaitMutex;
};
}

#endif // MUSE_AUDIO_AUDIOENGINE_H
