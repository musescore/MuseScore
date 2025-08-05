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

#ifndef MUSE_AUDIO_SOUNDTRACKWRITER_H
#define MUSE_AUDIO_SOUNDTRACKWRITER_H

#include <vector>

#include "global/async/asyncable.h"

#include "global/modularity/ioc.h"
#include "audio/worker/iaudioengine.h"
#include "audio/common/rpc/irpcchannel.h"

#include "audio/common/audiotypes.h"
#include "audio/worker/iaudiosource.h"
#include "../encoders/abstractaudioencoder.h"

namespace muse::audio::soundtrack {
class SoundTrackWriter : public muse::Injectable, public async::Asyncable
{
    muse::Inject<worker::IAudioEngine> audioEngine = { this };
    muse::Inject<rpc::IRpcChannel> rpcChannel = { this };

public:
    SoundTrackWriter(const io::path_t& destination, const SoundTrackFormat& format, const msecs_t totalDuration,
                     worker::IAudioSourcePtr source, const muse::modularity::ContextPtr& iocCtx);
    ~SoundTrackWriter() override;

    Ret write();
    void abort();

    Progress progress();

private:
    Ret generateAudioData();

    void sendStepProgress(int step, int64_t current, int64_t total);

    worker::IAudioSourcePtr m_source = nullptr;

    std::vector<float> m_inputBuffer;
    std::vector<float> m_intermBuffer;
    samples_t m_renderStep = 0;

    encode::AbstractAudioEncoderPtr m_encoderPtr = nullptr;

    Progress m_progress;
    std::atomic<bool> m_isAborted = false;
};
}

#endif // MUSE_AUDIO_SOUNDTRACKWRITER_H
