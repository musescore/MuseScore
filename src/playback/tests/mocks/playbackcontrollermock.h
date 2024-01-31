/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
    MOCK_METHOD(async::Notification, isPlayAllowedChanged, (), (const, override));

    MOCK_METHOD(bool, isPlaying, (), (const, override));
    MOCK_METHOD(async::Notification, isPlayingChanged, (), (const, override));

    MOCK_METHOD(void, seek, (const midi::tick_t), (override));
    MOCK_METHOD(void, seek, (const audio::msecs_t), (override));
    MOCK_METHOD(void, reset, (), (override));

    MOCK_METHOD(async::Notification, playbackPositionChanged, (), (const, override));
    MOCK_METHOD(async::Channel<uint32_t>, midiTickPlayed, (), (const, override));
    MOCK_METHOD(float, playbackPositionInSeconds, (), (const, override));

    MOCK_METHOD(audio::TrackSequenceId, currentTrackSequenceId, (), (const, override));
    MOCK_METHOD(async::Notification, currentTrackSequenceIdChanged, (), (const, override));

    MOCK_METHOD(const InstrumentTrackIdMap&, instrumentTrackIdMap, (), (const, override));

    MOCK_METHOD(const AuxTrackIdMap&, auxTrackIdMap, (), (const, override));

    MOCK_METHOD(async::Channel<audio::TrackId>, trackAdded, (), (const, override));
    MOCK_METHOD(async::Channel<audio::TrackId>, trackRemoved, (), (const, override));

    MOCK_METHOD(std::string, auxChannelName, (audio::aux_channel_idx_t), (const, override));
    MOCK_METHOD((async::Channel<audio::aux_channel_idx_t, std::string>), auxChannelNameChanged, (), (const, override));

    MOCK_METHOD(async::Promise<audio::SoundPresetList>, availableSoundPresets, (engraving::InstrumentTrackId), (const, override));

    MOCK_METHOD(notation::INotationSoloMuteState::SoloMuteState, trackSoloMuteState, (const engraving::InstrumentTrackId&),
                (const, override));
    MOCK_METHOD(void, setTrackSoloMuteState, (const engraving::InstrumentTrackId&, const notation::INotationSoloMuteState::SoloMuteState&),
                (const, override));

    MOCK_METHOD(void, playElements, ((const std::vector<const notation::EngravingItem*>&)), (override));
    MOCK_METHOD(void, playMetronome, (int), (override));
    MOCK_METHOD(void, seekElement, (const notation::EngravingItem*), (override));

    MOCK_METHOD(bool, actionChecked, (const actions::ActionCode&), (const, override));
    MOCK_METHOD(async::Channel<actions::ActionCode>, actionCheckedChanged, (), (const, override));

    MOCK_METHOD(QTime, totalPlayTime, (), (const, override));
    MOCK_METHOD(async::Notification, totalPlayTimeChanged, (), (const, override));

    MOCK_METHOD(notation::Tempo, currentTempo, (), (const, override));
    MOCK_METHOD(async::Notification, currentTempoChanged, (), (const, override));

    MOCK_METHOD(notation::MeasureBeat, currentBeat, (), (const, override));
    MOCK_METHOD(audio::msecs_t, beatToMilliseconds, (int, int), (const, override));

    MOCK_METHOD(double, tempoMultiplier, (), (const, override));
    MOCK_METHOD(void, setTempoMultiplier, (double), (override));

    MOCK_METHOD(framework::Progress, loadingProgress, (), (const, override));

    MOCK_METHOD(void, applyProfile, (const SoundProfileName&), (override));

    MOCK_METHOD(void, setNotation, (notation::INotationPtr), (override));
    MOCK_METHOD(void, setIsExportingAudio, (bool), (override));
};
}

#endif // MU_PLAYBACK_PLAYBACKCONTROLLERMOCK_H
