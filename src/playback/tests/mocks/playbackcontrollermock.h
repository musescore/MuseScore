/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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
#ifndef MU_PLAYBACK_PLAYBACKCONTROLLERMOCK_H
#define MU_PLAYBACK_PLAYBACKCONTROLLERMOCK_H

#include <gmock/gmock.h>

#include "playback/iplaybackcontroller.h"

namespace mu::playback {
class PlaybackControllerMock : public IPlaybackController
{
public:
    MOCK_METHOD(bool, isPlayAllowed, (), (const, override));
    MOCK_METHOD(muse::async::Notification, isPlayAllowedChanged, (), (const, override));

    MOCK_METHOD(bool, isPlaying, (), (const, override));
    MOCK_METHOD(muse::async::Notification, isPlayingChanged, (), (const, override));

    MOCK_METHOD(void, reset, (), (override));

    MOCK_METHOD((muse::async::Channel<muse::audio::secs_t, muse::midi::tick_t>), currentPlaybackPositionChanged, (), (const, override));

    MOCK_METHOD(muse::audio::TrackSequenceId, currentTrackSequenceId, (), (const, override));
    MOCK_METHOD(muse::async::Notification, currentTrackSequenceIdChanged, (), (const, override));

    MOCK_METHOD(const InstrumentTrackIdMap&, instrumentTrackIdMap, (), (const, override));

    MOCK_METHOD(const AuxTrackIdMap&, auxTrackIdMap, (), (const, override));

    MOCK_METHOD(muse::async::Channel<muse::audio::TrackId>, trackAdded, (), (const, override));
    MOCK_METHOD(muse::async::Channel<muse::audio::TrackId>, trackRemoved, (), (const, override));

    MOCK_METHOD(std::string, auxChannelName, (muse::audio::aux_channel_idx_t), (const, override));
    MOCK_METHOD((muse::async::Channel<muse::audio::aux_channel_idx_t, std::string>), auxChannelNameChanged, (), (const, override));

    MOCK_METHOD(muse::async::Promise<muse::audio::SoundPresetList>, availableSoundPresets, (const engraving::InstrumentTrackId&), (const,
                                                                                                                                   override));

    MOCK_METHOD(notation::INotationSoloMuteState::SoloMuteState, trackSoloMuteState, (const engraving::InstrumentTrackId&),
                (const, override));
    MOCK_METHOD(void, setTrackSoloMuteState, (const engraving::InstrumentTrackId&, const notation::INotationSoloMuteState::SoloMuteState&),
                (override));

    MOCK_METHOD(void, playElements, ((const std::vector<const notation::EngravingItem*>&)), (override));
    MOCK_METHOD(void, playMetronome, (int), (override));
    MOCK_METHOD(void, seekElement, (const notation::EngravingItem*), (override));
    MOCK_METHOD(void, seekBeat, (int, int), (override));

    MOCK_METHOD(bool, actionChecked, (const muse::actions::ActionCode&), (const, override));
    MOCK_METHOD(muse::async::Channel<muse::actions::ActionCode>, actionCheckedChanged, (), (const, override));

    MOCK_METHOD(QTime, totalPlayTime, (), (const, override));
    MOCK_METHOD(muse::async::Notification, totalPlayTimeChanged, (), (const, override));

    MOCK_METHOD(notation::Tempo, currentTempo, (), (const, override));
    MOCK_METHOD(muse::async::Notification, currentTempoChanged, (), (const, override));

    MOCK_METHOD(notation::MeasureBeat, currentBeat, (), (const, override));
    MOCK_METHOD(muse::audio::secs_t, beatToSecs, (int, int), (const, override));

    MOCK_METHOD(double, tempoMultiplier, (), (const, override));
    MOCK_METHOD(void, setTempoMultiplier, (double), (override));

    MOCK_METHOD(muse::Progress, loadingProgress, (), (const, override));

    MOCK_METHOD(void, applyProfile, (const SoundProfileName&), (override));

    MOCK_METHOD(void, setNotation, (notation::INotationPtr), (override));
    MOCK_METHOD(void, setIsExportingAudio, (bool), (override));
};
}

#endif // MU_PLAYBACK_PLAYBACKCONTROLLERMOCK_H
