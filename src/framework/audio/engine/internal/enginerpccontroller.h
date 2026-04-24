/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "global/async/asyncable.h"

#include "global/modularity/ioc.h"
#include "audio/common/rpc/irpcchannel.h"
#include "iaudioengine.h"
#include "../isoundfontrepository.h"
#include "../iaudioengineconfiguration.h"

#include "iaudiocontext.h"

namespace muse::audio::engine {
class EngineRpcController : public async::Asyncable
{
    GlobalInject<IAudioEngineConfiguration> configuration;
    GlobalInject<synth::ISoundFontRepository> soundFontRepository;
    GlobalInject<rpc::IRpcChannel> channel;
    GlobalInject<IAudioEngine> audioEngine;

public:
    EngineRpcController() = default;

    void init();
    void deinit();

private:

    std::shared_ptr<IAudioContext> audioContext(rpc::CtxId ctxId) const;

    void onLongRequest(rpc::MsgCode code, const rpc::Handler& h);
    void onQuickRequest(rpc::MsgCode code, const rpc::Handler& h);
    void onRequest(OperationType type, rpc::MsgCode code, const rpc::Handler& h);

    std::vector<rpc::MsgCode> m_usedRequests;
    std::atomic<bool> m_terminated = false;

    struct PendingTrack {
        rpc::Msg msg;
        TrackName trackName;
        mpe::PlaybackData playbackData;
        AudioParams params;
    };

    std::map<std::string /*sfname*/, std::vector<PendingTrack> > m_pendingTracks;
    bool m_soundFontsChangedSubscribed = false;

    async::Channel<int64_t, int64_t, SaveSoundTrackStage> m_saveSoundTrackProgressStream;
    rpc::StreamId m_saveSoundTrackProgressStreamId = 0;
};
}
