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
#include "../iaudioengine.h"
#include "../iengineplayback.h"
#include "../isoundfontrepository.h"
#include "../iaudioengineconfiguration.h"

namespace muse::audio::engine {
class EngineRpcController : public async::Asyncable
{
    Inject<rpc::IRpcChannel> channel;
    Inject<IAudioEngine> audioEngine;
    Inject<IEnginePlayback> playback;
    Inject<synth::ISoundFontRepository> soundFontRepository;
    Inject<IAudioEngineConfiguration> configuration;

public:
    EngineRpcController() = default;

    void init();
    void deinit();

private:

    void onLongMethod(rpc::Method method, const rpc::Handler& h);
    void onQuickMethod(rpc::Method method, const rpc::Handler& h);
    void onMethod(OperationType type, rpc::Method method, const rpc::Handler& h);

    std::vector<rpc::Method> m_usedMethods;

    struct PendingTrack {
        rpc::Msg msg;
        TrackSequenceId seqId;
        TrackName trackName;
        mpe::PlaybackData playbackData;
        AudioParams params;
    };

    std::map<std::string /*sfname*/, std::vector<PendingTrack> > m_pendingTracks;

    async::Channel<TrackSequenceId, int64_t, int64_t> m_saveSoundTrackProgressStream;
    rpc::StreamId m_saveSoundTrackProgressStreamId = 0;
};
}
