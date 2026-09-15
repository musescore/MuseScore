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

#include "playback/iplaybackconfiguration.h"

namespace mu::playback {
class PlaybackConfigurationMock : public IPlaybackConfiguration
{
public:
    MOCK_METHOD(bool, playNotesWhenEditing, (), (const, override));
    MOCK_METHOD(void, setPlayNotesWhenEditing, (bool value), (override));
    MOCK_METHOD(muse::async::Notification, playNotesWhenEditingChanged, (), (const, override));

    MOCK_METHOD(bool, playChordWhenEditing, (), (const, override));
    MOCK_METHOD(void, setPlayChordWhenEditing, (bool value), (override));
    MOCK_METHOD(muse::async::Channel<bool>, playChordWhenEditingChanged, (), (const, override));

    MOCK_METHOD(bool, playHarmonyWhenEditing, (), (const, override));
    MOCK_METHOD(void, setPlayHarmonyWhenEditing, (bool value), (override));
    MOCK_METHOD(muse::async::Channel<bool>, playHarmonyWhenEditingChanged, (), (const, override));

    MOCK_METHOD(bool, playNotesOnMidiInput, (), (const, override));
    MOCK_METHOD(void, setPlayNotesOnMidiInput, (bool value), (override));
    MOCK_METHOD(muse::async::Channel<bool>, playNotesOnMidiInputChanged, (), (const, override));

    MOCK_METHOD(PlaybackCursorType, cursorType, (), (const, override));

    MOCK_METHOD(bool, isMixerSectionVisible, (MixerSectionType sectionType), (const, override));
    MOCK_METHOD(void, setMixerSectionVisible, (MixerSectionType sectionType, bool visible), (override));
    MOCK_METHOD((muse::async::Channel<MixerSectionType, bool>), isMixerSectionVisibleChanged, (), (const, override));

    MOCK_METHOD(bool, isAuxSendVisible, (muse::audio::aux_channel_idx_t index), (const, override));
    MOCK_METHOD(void, setAuxSendVisible, (muse::audio::aux_channel_idx_t index, bool visible), (override));
    MOCK_METHOD((muse::async::Channel<muse::audio::aux_channel_idx_t, bool>), isAuxSendVisibleChanged, (), (const, override));

    MOCK_METHOD(bool, isAuxChannelVisible, (muse::audio::aux_channel_idx_t index), (const, override));
    MOCK_METHOD(void, setAuxChannelVisible, (muse::audio::aux_channel_idx_t index, bool visible), (const, override));
    MOCK_METHOD((muse::async::Channel<muse::audio::aux_channel_idx_t, bool>), isAuxChannelVisibleChanged, (), (const, override));

    MOCK_METHOD(muse::audio::gain_t, defaultAuxSendValue,
                (muse::audio::aux_channel_idx_t index, muse::audio::AudioSourceType sourceType,
                 const muse::String& instrumentSoundId), (const, override));

    MOCK_METHOD(bool, muteHiddenInstruments, (), (const, override));
    MOCK_METHOD(void, setMuteHiddenInstruments, (bool mute), (override));
    MOCK_METHOD(muse::async::Channel<bool>, muteHiddenInstrumentsChanged, (), (const, override));

    MOCK_METHOD(const SoundProfileName&, basicSoundProfileName, (), (const, override));
    MOCK_METHOD(const SoundProfileName&, museSoundsProfileName, (), (const, override));
    MOCK_METHOD(const SoundProfileName&, compatMuseSoundsProfileName, (), (const, override));

    MOCK_METHOD(SoundProfileName, defaultProfileForNewProjects, (), (const, override));
    MOCK_METHOD(void, setDefaultProfileForNewProjects, (const SoundProfileName& name), (override));

    MOCK_METHOD(bool, soundPresetsMultiSelectionEnabled, (), (const, override));
    MOCK_METHOD(void, setSoundPresetsMultiSelectionEnabled, (bool enabled), (override));

    MOCK_METHOD(bool, needToShowResetSoundFlagsWhenChangeSoundWarning, (), (const, override));
    MOCK_METHOD(void, setNeedToShowResetSoundFlagsWhenChangeSoundWarning, (bool show), (override));

    MOCK_METHOD(bool, needToShowResetSoundFlagsWhenChangePlaybackProfileWarning, (), (const, override));
    MOCK_METHOD(void, setNeedToShowResetSoundFlagsWhenChangePlaybackProfileWarning, (bool show), (override));

    MOCK_METHOD(bool, shouldShowOnlineSoundsProcessingError, (), (const, override));
    MOCK_METHOD(void, setShouldShowOnlineSoundsProcessingError, (bool show), (override));
    MOCK_METHOD(muse::async::Notification, shouldShowOnlineSoundsProcessingErrorChanged, (), (const, override));

    MOCK_METHOD(muse::String, onlineSoundsHandbookUrl, (), (const, override));

    MOCK_METHOD(OnlineSoundsShowProgressBarMode, onlineSoundsShowProgressBarMode, (), (const, override));
    MOCK_METHOD(void, setOnlineSoundsShowProgressBarMode, (OnlineSoundsShowProgressBarMode mode), (override));
    MOCK_METHOD(muse::async::Notification, onlineSoundsShowProgressBarModeChanged, (), (const, override));

    MOCK_METHOD(bool, shouldMeasureInputLag, (), (const, override));
};
}
