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
