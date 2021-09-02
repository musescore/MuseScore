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
#include "playbackcontrollerstub.h"

using namespace mu::playback;

bool PlaybackControllerStub::isPlayAllowed() const
{
    return false;
}

mu::async::Notification PlaybackControllerStub::isPlayAllowedChanged() const
{
    return mu::async::Notification();
}

bool PlaybackControllerStub::isPlaying() const
{
    return false;
}

mu::async::Notification PlaybackControllerStub::isPlayingChanged() const
{
    return mu::async::Notification();
}

float PlaybackControllerStub::playbackPosition() const
{
    return 0.f;
}

mu::async::Channel<uint32_t> PlaybackControllerStub::midiTickPlayed() const
{
    return mu::async::Channel<uint32_t>();
}

void PlaybackControllerStub::playElementOnClick(const mu::notation::EngravingItem*)
{
}
