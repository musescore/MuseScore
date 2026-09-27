/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <gmock/gmock.h>

#include "audio/main/iplayback.h"

namespace muse::audio {
class PlaybackMock : public IPlayback
{
public:
    // Init
    MOCK_METHOD(async::Promise<Ret>, init, (), (override));
    MOCK_METHOD(bool, isInited, (), (const, override));
    MOCK_METHOD(async::Channel<bool>, initedChanged, (), (const, override));
    MOCK_METHOD(void, deinit, (), (override));

    // Resources
    MOCK_METHOD(async::Promise<AudioResourceMetaList>, availableInputResources, (), (const, override));
    MOCK_METHOD(async::Promise<SoundPresetList>, availableSoundPresets, (const AudioResourceMeta& resourceMeta), (const, override));
    MOCK_METHOD(async::Promise<AudioResourceMetaList>, availableOutputResources, (), (const, override));

    // Setup tracks
    MOCK_METHOD(async::Promise<TrackIdList>, trackIdList, (), (const, override));
    MOCK_METHOD(async::Promise<RetVal<TrackName> >, trackName, (const TrackId trackId), (const, override));

    MOCK_METHOD((async::Promise<TrackId, TrackParams>), addTrack,
                (const TrackName& name, io::IODevice * data, const TrackParams& params), (override));
    MOCK_METHOD((async::Promise<TrackId, TrackParams>), addTrack,
                (const TrackName& name, const mpe::PlaybackData& data, const TrackParams& params), (override));
    MOCK_METHOD((async::Promise<TrackId, TrackParams>), addAuxTrack,
                (const TrackName& trackName, const TrackParams& params), (override));

    MOCK_METHOD(void, removeTrack, (const TrackId trackId), (override));
    MOCK_METHOD(void, removeAllTracks, (), (override));

    MOCK_METHOD(async::Channel<TrackId>, trackAdded, (), (const, override));
    MOCK_METHOD(async::Channel<TrackId>, trackRemoved, (), (const, override));

    // Params
    MOCK_METHOD(async::Promise<TrackParams>, params, (const TrackId trackId), (const, override));

    MOCK_METHOD(void, setSourceParams, (const TrackId trackId, const AudioSourceParams& params), (override));
    MOCK_METHOD(void, setControlParams, (const TrackId trackId, const ControlParams& params), (override));
    MOCK_METHOD(void, setFxChainParams, (const TrackId trackId, const AudioFxChain& params), (override));
    MOCK_METHOD(void, setAuxSendsParams, (const TrackId trackId, const AuxSendsParams& params), (override));

    MOCK_METHOD((async::Channel<TrackId, AudioSourceParams>), sourceParamsChanged, (), (const, override));
    MOCK_METHOD((async::Channel<TrackId, ControlParams>), controlParamsChanged, (), (const, override));
    MOCK_METHOD((async::Channel<TrackId, AudioFxChain>), fxChainParamsChanged, (), (const, override));
    MOCK_METHOD((async::Channel<TrackId, AuxSendsParams>), auxSendsParamsChanged, (), (const, override));

    // Master
    MOCK_METHOD(async::Promise<TrackParams>, masterParams, (), (const, override));
    MOCK_METHOD(void, setMasterControlParams, (const ControlParams& params), (override));
    MOCK_METHOD(void, setMasterFxChainParams, (const AudioFxChain& params), (override));
    MOCK_METHOD(void, setMasterAuxSendsParams, (const AuxSendsParams& params), (override));
    MOCK_METHOD(async::Channel<ControlParams>, masterControlParamsChanged, (), (const, override));
    MOCK_METHOD(async::Channel<AudioFxChain>, masterFxChainParamsChanged, (), (const, override));
    MOCK_METHOD(async::Channel<AuxSendsParams>, masterAuxSendsParamsChanged, (), (const, override));

    // Input processing
    MOCK_METHOD(void, processInput, (const TrackId trackId), (const, override));
    MOCK_METHOD(async::Promise<InputProcessingProgress>, inputProcessingProgress, (const TrackId trackId), (const, override));

    MOCK_METHOD(void, clearCache, (const TrackId trackId), (const, override));
    MOCK_METHOD(void, clearSources, (), (override));
    MOCK_METHOD(void, clearMasterOutputParams, (), (override));
    MOCK_METHOD(void, clearAllFx, (), (override));

    // Play
    MOCK_METHOD(std::shared_ptr<IPlayer>, player, (), (const, override));

    // Signal changes
    MOCK_METHOD(async::Promise<AudioSignalChanges>, signalChanges, (const TrackId trackId), (const, override));
    MOCK_METHOD(async::Promise<AudioSignalChanges>, masterSignalChanges, (), (const, override));

    // Automation
    MOCK_METHOD(async::Promise<AutomatedControlParamsChanges>, automatedControlParamsChanges, (const TrackId trackId), (const, override));

    // Export
    MOCK_METHOD(async::Promise<bool>, saveSoundTrack, (const SoundTrackFormat& format, io::IODevice& dstDevice), (override));
    MOCK_METHOD(async::Promise<bool>, saveSoundTracks, (const SoundTrackFormat& format, const SoundTrackTargetList& targets),
                (override));
    MOCK_METHOD(void, abortSavingAllSoundTracks, (), (override));
    MOCK_METHOD(SaveSoundTrackProgress, saveSoundTrackProgressChanged, (), (const, override));
};
}
