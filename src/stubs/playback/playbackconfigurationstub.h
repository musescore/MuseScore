/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_PLAYBACK_PLAYBACKCONFIGURATIONSTUB_H
#define MU_PLAYBACK_PLAYBACKCONFIGURATIONSTUB_H

#include "playback/iplaybackconfiguration.h"

namespace mu::playback {
class PlaybackConfigurationStub : public IPlaybackConfiguration
{
public:
    bool playNotesWhenEditing() const override;
    void setPlayNotesWhenEditing(bool value) override;
    muse::async::Notification playNotesWhenEditingChanged() const override;

    bool playChordWhenEditing() const override;
    void setPlayChordWhenEditing(bool value) override;
    muse::async::Channel<bool> playChordWhenEditingChanged() const override;

    bool playHarmonyWhenEditing() const override;
    void setPlayHarmonyWhenEditing(bool value) override;
    muse::async::Channel<bool> playHarmonyWhenEditingChanged() const override;

    bool playNotesOnMidiInput() const override;
    void setPlayNotesOnMidiInput(bool value) override;
    muse::async::Channel<bool> playNotesOnMidiInputChanged() const override;

    PlaybackCursorType cursorType() const override;

    bool isMixerSectionVisible(MixerSectionType sectionType) const override;
    void setMixerSectionVisible(MixerSectionType sectionType, bool visible) override;
    muse::async::Channel<MixerSectionType, bool> isMixerSectionVisibleChanged() const override;

    bool isAuxSendVisible(muse::audio::aux_channel_idx_t index) const override;
    void setAuxSendVisible(muse::audio::aux_channel_idx_t index, bool visible) override;
    muse::async::Channel<muse::audio::aux_channel_idx_t, bool> isAuxSendVisibleChanged() const override;

    bool isAuxChannelVisible(muse::audio::aux_channel_idx_t index) const override;
    void setAuxChannelVisible(muse::audio::aux_channel_idx_t index, bool visible) const override;
    muse::async::Channel<muse::audio::aux_channel_idx_t, bool> isAuxChannelVisibleChanged() const override;

    muse::audio::gain_t defaultAuxSendValue(muse::audio::aux_channel_idx_t index, muse::audio::AudioSourceType sourceType,
                                            const muse::String& instrumentSoundId) const override;

    bool muteHiddenInstruments() const override;
    void setMuteHiddenInstruments(bool mute) override;
    muse::async::Channel<bool> muteHiddenInstrumentsChanged() const override;

    const SoundProfileName& basicSoundProfileName() const override;
    const SoundProfileName& museSoundsProfileName() const override;
    const SoundProfileName& compatMuseSoundsProfileName() const override;

    SoundProfileName defaultProfileForNewProjects() const override;
    void setDefaultProfileForNewProjects(const SoundProfileName& name) override;

    bool soundPresetsMultiSelectionEnabled() const override;
    void setSoundPresetsMultiSelectionEnabled(bool enabled) override;

    bool needToShowResetSoundFlagsWhenChangeSoundWarning() const override;
    void setNeedToShowResetSoundFlagsWhenChangeSoundWarning(bool show) override;

    bool needToShowResetSoundFlagsWhenChangePlaybackProfileWarning() const override;
    void setNeedToShowResetSoundFlagsWhenChangePlaybackProfileWarning(bool show) override;

    bool needToShowOnlineSoundsConnectionWarning() const override;
    void setNeedToShowOnlineSoundsConnectionWarning(bool show) override;

    OnlineSoundsShowProgressBarMode onlineSoundsShowProgressBarMode() const override;
    void setOnlineSoundsShowProgressBarMode(OnlineSoundsShowProgressBarMode mode) override;
    muse::async::Notification onlineSoundsShowProgressBarModeChanged() const override;

    bool shouldMeasureInputLag() const override;
};
}

#endif // MU_PLAYBACK_PLAYBACKCONFIGURATIONSTUB_H
