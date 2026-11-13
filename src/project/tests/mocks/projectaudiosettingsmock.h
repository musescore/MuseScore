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

#include "project/iprojectaudiosettings.h"

namespace mu::project {
class ProjectAudioSettingsMock : public IProjectAudioSettings
{
public:
    MOCK_METHOD(bool, hasAnyAudioSettings, (), (const, override));

    MOCK_METHOD(const AudioOutputParams&, masterAudioOutputParams, (), (const, override));
    MOCK_METHOD(void, setMasterAudioOutputParams, (const AudioOutputParams& params), (override));

    MOCK_METHOD(bool, containsAuxOutputParams, (muse::audio::aux_channel_idx_t index), (const, override));
    MOCK_METHOD(const AudioOutputParams&, auxOutputParams, (muse::audio::aux_channel_idx_t index), (const, override));
    MOCK_METHOD(void, setAuxOutputParams, (muse::audio::aux_channel_idx_t index, const AudioOutputParams& params,
                                           bool notifySettingsChanged), (override));

    MOCK_METHOD(const TrackInputParamsMap&, allTrackInputParams, (), (const, override));
    MOCK_METHOD(const AudioInputParams&, trackInputParams, (const engraving::InstrumentTrackId& trackId), (const, override));
    MOCK_METHOD(void, setTrackInputParams, (const engraving::InstrumentTrackId& trackId, const AudioInputParams& params,
                                            bool notifySettingsChanged), (override));
    MOCK_METHOD(void, clearTrackInputParams, (), (override));
    MOCK_METHOD(muse::async::Channel<engraving::InstrumentTrackId>, trackInputParamsChanged, (), (const, override));

    MOCK_METHOD(bool, trackHasExistingOutputParams, (const engraving::InstrumentTrackId& trackId), (const, override));
    MOCK_METHOD(const AudioOutputParams&, trackOutputParams, (const engraving::InstrumentTrackId& trackId), (const, override));
    MOCK_METHOD(void, setTrackOutputParams, (const engraving::InstrumentTrackId& trackId, const AudioOutputParams& params,
                                             bool notifySettingsChanged), (override));

    MOCK_METHOD(const SoloMuteState&, auxSoloMuteState, (muse::audio::aux_channel_idx_t index), (const, override));
    MOCK_METHOD(void, setAuxSoloMuteState, (muse::audio::aux_channel_idx_t index, const SoloMuteState& state), (override));
    MOCK_METHOD((muse::async::Channel<muse::audio::aux_channel_idx_t, SoloMuteState>), auxSoloMuteStateChanged, (), (const, override));

    MOCK_METHOD(void, removeTrackParams, (const engraving::InstrumentTrackId& trackId), (override));

    MOCK_METHOD(const playback::SoundProfileName&, activeSoundProfile, (), (const, override));
    MOCK_METHOD(void, setActiveSoundProfile, (const playback::SoundProfileName& profileName), (override));

    MOCK_METHOD(muse::async::Notification, settingsChanged, (), (const, override));
};
}
