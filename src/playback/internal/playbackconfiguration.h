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
#ifndef MU_PLAYBACK_PLAYBACKCONFIGURATION_H
#define MU_PLAYBACK_PLAYBACKCONFIGURATION_H

#include "../iplaybackconfiguration.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "musesampler/imusesamplerinfo.h"
#include "audio/iaudioconfiguration.h"

namespace mu::playback {
class PlaybackConfiguration : public IPlaybackConfiguration, public muse::async::Asyncable
{
    INJECT(muse::musesampler::IMuseSamplerInfo, musesamplerInfo)
    INJECT(muse::audio::IAudioConfiguration, audioConfiguration)

public:
    void init();

    bool playNotesWhenEditing() const override;
    void setPlayNotesWhenEditing(bool value) override;
    muse::async::Notification playNotesWhenEditingChanged() const override;

    bool playChordWhenEditing() const override;
    void setPlayChordWhenEditing(bool value) override;
    muse::async::Channel<bool> playChordWhenEditingChanged() const override;

    bool playHarmonyWhenEditing() const override;
    void setPlayHarmonyWhenEditing(bool value) override;
    muse::async::Channel<bool> playHarmonyWhenEditingChanged() const override;

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

    bool jackTransportEnable() const override;
    void setJackTransportEnable(bool mute) override;
    muse::async::Channel<bool> jackTransportEnableChanged() const override;

    const SoundProfileName& basicSoundProfileName() const override;
    const SoundProfileName& museSoundProfileName() const override;
    SoundProfileName defaultProfileForNewProjects() const override;
    void setDefaultProfileForNewProjects(const SoundProfileName& name) override;

    bool soundPresetsMultiSelectionEnabled() const override;
    void setSoundPresetsMultiSelectionEnabled(bool enabled) override;

    bool needToShowResetSoundFlagsWhenChangeSoundWarning() const override;
    void setNeedToShowResetSoundFlagsWhenChangeSoundWarning(bool show) override;

    bool needToShowResetSoundFlagsWhenChangePlaybackProfileWarning() const override;
    void setNeedToShowResetSoundFlagsWhenChangePlaybackProfileWarning(bool show) override;

    bool shouldMeasureInputLag() const override;

private:
    const SoundProfileName& fallbackSoundProfileStr() const;

    muse::async::Notification m_playNotesWhenEditingChanged;
    muse::async::Channel<bool> m_playChordWhenEditingChanged;
    muse::async::Channel<bool> m_playHarmonyWhenEditingChanged;
    muse::async::Channel<muse::audio::aux_channel_idx_t, bool> m_isAuxSendVisibleChanged;
    muse::async::Channel<muse::audio::aux_channel_idx_t, bool> m_isAuxChannelVisibleChanged;
    muse::async::Channel<MixerSectionType, bool> m_isMixerSectionVisibleChanged;

    muse::async::Channel<bool> m_muteHiddenInstrumentsChanged;

    muse::async::Channel<bool> m_jackTransportEnableChanged;
};
}

#endif // MU_PLAYBACK_PLAYBACKCONFIGURATION_H
