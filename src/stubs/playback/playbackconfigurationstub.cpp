/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
using namespace mu::audio;

bool PlaybackConfigurationStub::playNotesWhenEditing() const
{
    return false;
}

void PlaybackConfigurationStub::setPlayNotesWhenEditing(bool)
{
}

bool PlaybackConfigurationStub::playChordWhenEditing() const
{
    return false;
}

void PlaybackConfigurationStub::setPlayChordWhenEditing(bool)
{
}

bool PlaybackConfigurationStub::playHarmonyWhenEditing() const
{
    return false;
}

void PlaybackConfigurationStub::setPlayHarmonyWhenEditing(bool)
{
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

bool PlaybackConfigurationStub::isAuxSendVisible(audio::aux_channel_idx_t) const
{
    return false;
}

void PlaybackConfigurationStub::setAuxSendVisible(audio::aux_channel_idx_t, bool)
{
}

mu::async::Channel<mu::audio::aux_channel_idx_t, bool> PlaybackConfigurationStub::isAuxSendVisibleChanged() const
{
    return {};
}

bool PlaybackConfigurationStub::isAuxChannelVisible(audio::aux_channel_idx_t) const
{
    return false;
}

void PlaybackConfigurationStub::setAuxChannelVisible(audio::aux_channel_idx_t, bool) const
{
}

mu::async::Channel<mu::audio::aux_channel_idx_t, bool> PlaybackConfigurationStub::isAuxChannelVisibleChanged() const
{
    return {};
}

gain_t PlaybackConfigurationStub::defaultAuxSendValue(aux_channel_idx_t, AudioSourceType, const String&) const
{
    return 0.f;
}

const SoundProfileName& PlaybackConfigurationStub::basicSoundProfileName() const
{
    static const SoundProfileName basic;
    return basic;
}

const SoundProfileName& PlaybackConfigurationStub::museSoundProfileName() const
{
    static const SoundProfileName museSounds;
    return museSounds;
}

SoundProfileName PlaybackConfigurationStub::defaultProfileForNewProjects() const
{
    return {};
}

void PlaybackConfigurationStub::setDefaultProfileForNewProjects(const SoundProfileName&)
{
}
