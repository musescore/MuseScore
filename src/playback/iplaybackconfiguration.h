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
#ifndef MU_PLAYBACK_IPLAYBACKCONFIGURATION_H
#define MU_PLAYBACK_IPLAYBACKCONFIGURATION_H

#include "modularity/imoduleinterface.h"
#include "async/channel.h"
#include "async/notification.h"
#include "playbacktypes.h"

namespace mu::playback {
class IPlaybackConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IPlaybackConfiguration)

public:
    virtual ~IPlaybackConfiguration() = default;

    virtual bool playNotesWhenEditing() const = 0;
    virtual void setPlayNotesWhenEditing(bool value) = 0;
    virtual muse::async::Notification playNotesWhenEditingChanged() const = 0;

    virtual bool playChordWhenEditing() const = 0;
    virtual void setPlayChordWhenEditing(bool value) = 0;
    virtual muse::async::Channel<bool> playChordWhenEditingChanged() const = 0;

    virtual bool playHarmonyWhenEditing() const = 0;
    virtual void setPlayHarmonyWhenEditing(bool value) = 0;
    virtual muse::async::Channel<bool> playHarmonyWhenEditingChanged() const = 0;

    virtual bool playNotesOnMidiInput() const = 0;
    virtual void setPlayNotesOnMidiInput(bool value) = 0;
    virtual muse::async::Channel<bool> playNotesOnMidiInputChanged() const = 0;

    virtual PlaybackCursorType cursorType() const = 0;

    virtual bool isMixerSectionVisible(MixerSectionType sectionType) const = 0;
    virtual void setMixerSectionVisible(MixerSectionType sectionType, bool visible) = 0;
    virtual muse::async::Channel<MixerSectionType, bool> isMixerSectionVisibleChanged() const = 0;

    virtual bool isAuxSendVisible(muse::audio::aux_channel_idx_t index) const = 0;
    virtual void setAuxSendVisible(muse::audio::aux_channel_idx_t index, bool visible) = 0;
    virtual muse::async::Channel<muse::audio::aux_channel_idx_t, bool> isAuxSendVisibleChanged() const = 0;

    virtual bool isAuxChannelVisible(muse::audio::aux_channel_idx_t index) const = 0;
    virtual void setAuxChannelVisible(muse::audio::aux_channel_idx_t index, bool visible) const = 0;
    virtual muse::async::Channel<muse::audio::aux_channel_idx_t, bool> isAuxChannelVisibleChanged() const = 0;

    virtual muse::audio::gain_t defaultAuxSendValue(muse::audio::aux_channel_idx_t index, muse::audio::AudioSourceType sourceType,
                                                    const muse::String& instrumentSoundId) const = 0;

    virtual bool muteHiddenInstruments() const = 0;
    virtual void setMuteHiddenInstruments(bool mute) = 0;
    virtual muse::async::Channel<bool> muteHiddenInstrumentsChanged() const = 0;

    virtual const SoundProfileName& basicSoundProfileName() const = 0;
    virtual const SoundProfileName& museSoundsProfileName() const = 0;
    virtual const SoundProfileName& compatMuseSoundsProfileName() const = 0;

    virtual SoundProfileName defaultProfileForNewProjects() const = 0;
    virtual void setDefaultProfileForNewProjects(const SoundProfileName& name) = 0;

    virtual bool soundPresetsMultiSelectionEnabled() const = 0;
    virtual void setSoundPresetsMultiSelectionEnabled(bool enabled) = 0;

    virtual bool needToShowResetSoundFlagsWhenChangeSoundWarning() const = 0;
    virtual void setNeedToShowResetSoundFlagsWhenChangeSoundWarning(bool show) = 0;

    virtual bool needToShowResetSoundFlagsWhenChangePlaybackProfileWarning() const = 0;
    virtual void setNeedToShowResetSoundFlagsWhenChangePlaybackProfileWarning(bool show) = 0;

    virtual bool needToShowOnlineSoundsConnectionWarning() const = 0;
    virtual void setNeedToShowOnlineSoundsConnectionWarning(bool show) = 0;

    virtual OnlineSoundsShowProgressBarMode onlineSoundsShowProgressBarMode() const = 0;
    virtual void setOnlineSoundsShowProgressBarMode(OnlineSoundsShowProgressBarMode mode) = 0;
    virtual muse::async::Notification onlineSoundsShowProgressBarModeChanged() const = 0;

    virtual bool shouldMeasureInputLag() const = 0;
};
}

#endif // MU_PLAYBACK_IPLAYBACKCONFIGURATION_H
