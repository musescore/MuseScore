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
#include "playbackconfigurationstub.h"

using namespace mu::playback;
using namespace muse::audio;

bool PlaybackConfigurationStub::playNotesWhenEditing() const
{
    return false;
}

void PlaybackConfigurationStub::setPlayNotesWhenEditing(bool)
{
}

muse::async::Notification PlaybackConfigurationStub::playNotesWhenEditingChanged() const
{
    return muse::async::Notification();
}

bool PlaybackConfigurationStub::playChordWhenEditing() const
{
    return false;
}

void PlaybackConfigurationStub::setPlayChordWhenEditing(bool)
{
}

muse::async::Channel<bool> PlaybackConfigurationStub::playChordWhenEditingChanged() const
{
    static muse::async::Channel<bool> ch;
    return ch;
}

bool PlaybackConfigurationStub::playHarmonyWhenEditing() const
{
    return false;
}

void PlaybackConfigurationStub::setPlayHarmonyWhenEditing(bool)
{
}

muse::async::Channel<bool> PlaybackConfigurationStub::playHarmonyWhenEditingChanged() const
{
    static muse::async::Channel<bool> ch;
    return ch;
}

bool PlaybackConfigurationStub::playNotesOnMidiInput() const
{
    return false;
}

void PlaybackConfigurationStub::setPlayNotesOnMidiInput(bool)
{
}

muse::async::Channel<bool> PlaybackConfigurationStub::playNotesOnMidiInputChanged() const
{
    static muse::async::Channel<bool> ch;
    return ch;
}

PlaybackCursorType PlaybackConfigurationStub::cursorType() const
{
    return PlaybackCursorType::SMOOTH;
}

bool PlaybackConfigurationStub::isMixerSectionVisible(MixerSectionType) const
{
    return false;
}

void PlaybackConfigurationStub::setMixerSectionVisible(MixerSectionType, bool)
{
}

muse::async::Channel<MixerSectionType, bool> PlaybackConfigurationStub::isMixerSectionVisibleChanged() const
{
    return {};
}

bool PlaybackConfigurationStub::isAuxSendVisible(aux_channel_idx_t) const
{
    return false;
}

void PlaybackConfigurationStub::setAuxSendVisible(aux_channel_idx_t, bool)
{
}

muse::async::Channel<aux_channel_idx_t, bool> PlaybackConfigurationStub::isAuxSendVisibleChanged() const
{
    return {};
}

bool PlaybackConfigurationStub::isAuxChannelVisible(aux_channel_idx_t) const
{
    return false;
}

void PlaybackConfigurationStub::setAuxChannelVisible(aux_channel_idx_t, bool) const
{
}

muse::async::Channel<aux_channel_idx_t, bool> PlaybackConfigurationStub::isAuxChannelVisibleChanged() const
{
    return {};
}

gain_t PlaybackConfigurationStub::defaultAuxSendValue(aux_channel_idx_t, AudioSourceType, const muse::String&) const
{
    return 0.f;
}

bool PlaybackConfigurationStub::muteHiddenInstruments() const
{
    return false;
}

void PlaybackConfigurationStub::setMuteHiddenInstruments(bool)
{
}

muse::async::Channel<bool> PlaybackConfigurationStub::muteHiddenInstrumentsChanged() const
{
    static muse::async::Channel<bool> ch;
    return ch;
}

const SoundProfileName& PlaybackConfigurationStub::basicSoundProfileName() const
{
    static const SoundProfileName basic;
    return basic;
}

const SoundProfileName& PlaybackConfigurationStub::museSoundsProfileName() const
{
    static const SoundProfileName museSounds;
    return museSounds;
}

const SoundProfileName& PlaybackConfigurationStub::compatMuseSoundsProfileName() const
{
    static const SoundProfileName compatMuseSounds;
    return compatMuseSounds;
}

SoundProfileName PlaybackConfigurationStub::defaultProfileForNewProjects() const
{
    return {};
}

void PlaybackConfigurationStub::setDefaultProfileForNewProjects(const SoundProfileName&)
{
}

bool PlaybackConfigurationStub::soundPresetsMultiSelectionEnabled() const
{
    return false;
}

void PlaybackConfigurationStub::setSoundPresetsMultiSelectionEnabled(bool)
{
}

bool PlaybackConfigurationStub::needToShowResetSoundFlagsWhenChangeSoundWarning() const
{
    return false;
}

void PlaybackConfigurationStub::setNeedToShowResetSoundFlagsWhenChangeSoundWarning(bool)
{
}

bool PlaybackConfigurationStub::needToShowResetSoundFlagsWhenChangePlaybackProfileWarning() const
{
    return false;
}

void PlaybackConfigurationStub::setNeedToShowResetSoundFlagsWhenChangePlaybackProfileWarning(bool)
{
}

bool PlaybackConfigurationStub::needToShowOnlineSoundsConnectionWarning() const
{
    return false;
}

void PlaybackConfigurationStub::setNeedToShowOnlineSoundsConnectionWarning(bool)
{
}

OnlineSoundsShowProgressBarMode PlaybackConfigurationStub::onlineSoundsShowProgressBarMode() const
{
    return OnlineSoundsShowProgressBarMode::Never;
}

void PlaybackConfigurationStub::setOnlineSoundsShowProgressBarMode(OnlineSoundsShowProgressBarMode)
{
}

muse::async::Notification PlaybackConfigurationStub::onlineSoundsShowProgressBarModeChanged() const
{
    return {};
}

bool PlaybackConfigurationStub::shouldMeasureInputLag() const
{
    return false;
}
